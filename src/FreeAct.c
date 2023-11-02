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

/*..........................................................................*/
void Active_ctor(Active * const me, DispatchHandler dispatch) {
    me->dispatch = dispatch; /* assign the dispatch handler */
}

/*..........................................................................*/
/* thread function for all Active Objects (FreeRTOS task signature) */
static void Active_eventLoop(void *pvParameters) {
    Active *me = (Active *)pvParameters;
    static Event const initEvt = { INIT_SIG };

    configASSERT(me); /* Active object must be provided */

    /* initialize the AO */
    (*me->dispatch)(me, &initEvt);

    for (;;) {   /* for-ever "superloop" */
        Event const *e; /* pointer to event object ("message") */

        /* wait for any event and receive it into object 'e' */
        xQueueReceive(me->queue, &e, portMAX_DELAY); /* BLOCKING! */
        configASSERT(e != (Event const *)0);

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
    me->queue = xQueueCreateStatic(
                   queueLen,            /* queue length - provided by user */
                   sizeof(Event *),     /* item size */
                   (uint8_t *)queueSto, /* queue storage - provided by user */
                   &me->queue_cb);      /* queue control block */
    configASSERT(me->queue);            /* queue must be created */

    me->thread = xTaskCreateStatic(
              &Active_eventLoop,        /* the thread function */
              "AO" ,                    /* the name of the task */
              stk_depth,                /* stack depth */
              me,                       /* the 'pvParameters' parameter */
              prio + tskIDLE_PRIORITY,  /* FreeRTOS priority */
              stk_sto,                  /* stack storage - provided by user */
              &me->thread_cb);          /* task control block */
    configASSERT(me->thread);           /* thread must be created */
}

/*..........................................................................*/
void Active_post(Active * const me, Event const * const e) {
    BaseType_t status = xQueueSendToBack(me->queue, (void *)&e, (TickType_t)0);
    configASSERT(status == pdTRUE);
}

/*..........................................................................*/
void Active_postFromISR(Active * const me, Event const * const e,
                        BaseType_t *pxHigherPriorityTaskWoken)
{
    BaseType_t status = xQueueSendToBackFromISR(me->queue, (void *)&e,
                                                pxHigherPriorityTaskWoken);
    configASSERT(status == pdTRUE);
}

/*--------------------------------------------------------------------------*/
/* Time Event services... */
static void TimeEvent_callback(TimerHandle_t xTimer);

/*..........................................................................*/
void TimeEvent_ctor(TimeEvent * const me, Signal sig, Active *act) {
    /* no critical section because it is presumed that all TimeEvents
     * are created *before* multitasking has started.
     */
    me->super.sig = sig;
    me->act = act;

    /* Create a timer object */
    me->timer = xTimerCreateStatic("TE", 1U, me->type, me,
                                   TimeEvent_callback, &me->timer_cb);
    configASSERT(me->timer);            /* timer must be created */
}

/*..........................................................................*/
void TimeEvent_arm(TimeEvent * const me, uint32_t millisec) {
    TickType_t ticks;
    BaseType_t status;
    BaseType_t xHigherPriorityTaskWoken;

    ticks = (millisec / portTICK_PERIOD_MS);
    if (ticks == 0U) {
        ticks = 1U;
    }

    if (xPortIsInsideInterrupt() == pdTRUE) {
        xHigherPriorityTaskWoken = pdFALSE;

        status = xTimerChangePeriodFromISR(me->timer, ticks,
                                           &xHigherPriorityTaskWoken);
        configASSERT(status == pdPASS);

        portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
    }
    else {
        status = xTimerChangePeriod(me->timer, ticks, 0);
        configASSERT(status == pdPASS);
    }
}

/*..........................................................................*/
void TimeEvent_disarm(TimeEvent * const me) {
    BaseType_t xHigherPriorityTaskWoken;
    BaseType_t status;

    if (xPortIsInsideInterrupt() == pdTRUE) {
        xHigherPriorityTaskWoken = pdFALSE;
        status = xTimerStopFromISR(me->timer, &xHigherPriorityTaskWoken);
        configASSERT(status == pdPASS);

        portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
    }
    else {
        status = xTimerStop(me->timer, 0);
        configASSERT(status == pdPASS);
    }
}

/*..........................................................................*/
/* Use this macro to get the container of TimeEvent struct
 *  since xTimer pointing to timer_cb
 */
#define GET_TIME_EVENT_HEAD(ptr) \
    (TimeEvent*)((uintptr_t)(ptr) - offsetof(TimeEvent, timer_cb))

static void TimeEvent_callback(TimerHandle_t xTimer) {
    /* Also can use pvTimerGetTimerID(xTimer) */
    TimeEvent * const t = GET_TIME_EVENT_HEAD(xTimer);

    /* Callback always called from non-interrupt context so no need
     * to check xPortIsInsideInterrupt
     */
    Active_post(t->act, &t->super);
}
