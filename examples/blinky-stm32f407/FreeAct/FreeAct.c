/*****************************************************************************
* Free Active Object pattern implementation (FreeAct) based on FreeRTOS
*
*                    Q u a n t u m  L e a P s
*                    ------------------------
*                    Modern Embedded Software
*
* Copyright (C) 2020 Quantum Leaps, LLC. All rights reserved.
*
* MIT License:
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to
* deal in the Software without restriction, including without limitation the
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
* sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*
* Contact information:
* <www.state-machine.com>
* <info@state-machine.com>
*****************************************************************************/
#include "FreeAct.h" /* Free Active Object interface */
#include <stdint.h>
/*..........................................................................*/
void Active_ctor(Active * const me, DispatchHandler dispatch) {
    me->dispatch = dispatch; /* assign the dispatch handler */
}

/*..........................................................................*/
/* Event-loop thread function for all Active Objects (FreeRTOS task signature) */
static void Active_eventLoop(void *pvParameters) {
    Active *me = (Active *)pvParameters;
    static Event const initEvt = { INIT_SIG };

    configASSERT(me); /* Active object must be provided */

    /* initialize the AO */
    (*me->dispatch)(me, &initEvt);

    for (;;) {   /* for-ever "superloop" */
        Event *e; /* pointer to event object ("message") */

        /* wait for any event and receive it into object 'e' */
        xQueueReceive(me->queue, &e, portMAX_DELAY); /* BLOCKING! */

        /* dispatch event to the active object 'me' */
        (*me->dispatch)(me, e); /* NO BLOCKING! */
    }
}

/*..........................................................................*/
void Active_start(Active * const me,
                  uint8_t prio,       /* priority (1-based) */
                  Event **queueSto,
                  uint32_t queueLen,
                  void *stackSto,
                  uint32_t stackSize,
                  uint16_t opt)
{
    StackType_t *stk_sto = stackSto;
    uint32_t stk_depth = (stackSize / sizeof(StackType_t));

    (void)opt; /* unused parameter */
//    me->queue = xQueueCreateStatic(
//                   queueLen,            /* queue length - provided by user */
//                   sizeof(Event *),     /* item size */
//                   (uint8_t *)queueSto, /* queue storage - provided by user */
//                   &me->queue_cb);      /* queue control block */
    me->queue = xQueueCreate(
                   queueLen,            /* queue length - provided by user */
                   sizeof(Event *)     /* item size */
                   );      
//    me->thread = xTaskCreateStatic(
//              &Active_eventLoop,        /* the thread function */
//              "AO" ,                    /* the name of the task */
//              stk_depth,                /* stack depth */
//              me,                       /* the 'pvParameters' parameter */
//              prio + tskIDLE_PRIORITY,  /* FreeRTOS priority */
//              stk_sto,                  /* stack storage - provided by user */
//              &me->thread_cb);          /* task control block */
    me->thread = xTaskCreate(
              &Active_eventLoop,        /* the thread function */
              "AO" ,                    /* the name of the task */
              stk_depth,                /* stack depth */
              me,                       /* the 'pvParameters' parameter */
              prio + tskIDLE_PRIORITY,  /* FreeRTOS priority */
              NULL);          

	 configASSERT(me->thread); /* thread must be created */
}

/*..........................................................................*/
void Active_post(Active * const me, Event const * const e) {
    BaseType_t status = xQueueSend(me->queue, (void *)&e, (TickType_t)0);
    configASSERT(status == pdTRUE);
}

/*..........................................................................*/
void Active_postFromISR(Active * const me, Event const * const e,
                        BaseType_t *pxHigherPriorityTaskWoken)
{
    BaseType_t status = xQueueSendFromISR(me->queue, (void *)&e,
                                          pxHigherPriorityTaskWoken);
    configASSERT(status == pdTRUE);
}

/*--------------------------------------------------------------------------*/
/* Time Event services... */

static TimeEvent *l_tevt[10]; /* all TimeEvents in the application */
static uint_fast8_t l_tevtNum; /* current number of TimeEvents */

/*..........................................................................*/
void TimeEvent_ctor(TimeEvent * const me, Signal sig, Active *act) {
    /* no critical section because it is presumed that all TimeEvents
    * are created *before* multitasking has started.
    */
    me->super.sig = sig;
    me->act = act;
    me->timeout = 0U;
    me->interval = 0U;

    /* register one more TimeEvent with the application */
    configASSERT(l_tevtNum < sizeof(l_tevt)/sizeof(l_tevt[0]));
    l_tevt[l_tevtNum] = me;
    ++l_tevtNum;
}

/*..........................................................................*/
void TimeEvent_arm(TimeEvent * const me, uint32_t timeout, uint32_t interval) {
    taskENTER_CRITICAL();
    me->timeout = timeout;
    me->interval = interval;
    taskEXIT_CRITICAL();
}

/*..........................................................................*/
void TimeEvent_disarm(TimeEvent * const me) {
    taskENTER_CRITICAL();
    me->timeout = 0U;
    taskEXIT_CRITICAL();
}

/*..........................................................................*/
void TimeEvent_tickFromISR(BaseType_t *pxHigherPriorityTaskWoken) {
    uint_fast8_t i;
    for (i = 0U; i < l_tevtNum; ++i) {
        TimeEvent * const t = l_tevt[i];
        configASSERT(t); /* TimeEvent instance must be registered */
        if (t->timeout > 0U) { /* is this TimeEvent armed? */
            if (--t->timeout == 0U) { /* is it expiring now? */
                Active_postFromISR(t->act, &t->super,
                                   pxHigherPriorityTaskWoken);
                t->timeout = t->interval; /* rearm or disarm (one-shot) */
           }
        }
    }
}

