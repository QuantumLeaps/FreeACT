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
#ifndef FREE_ACT_H
#define FREE_ACT_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/*---------------------------------------------------------------------------*/
/* Event facilities... */

typedef uint16_t Signal; /* event signal */

enum ReservedSignals {
    INIT_SIG, /* dispatched to AO before entering event-loop */
    USER_SIG  /* first signal available to the users */
};

typedef enum {
    TYPE_ONE_SHOT = pdFALSE,    /* One-shot timer */
    TYPE_PERIODIC = pdTRUE      /* Periodic timer */
} TimerType_t;

/* Event base class */
typedef struct {
    Signal sig; /* event signal */
    /* event parameters added in subclasses of Event */
} Event;

/*---------------------------------------------------------------------------*/
/* Actvie Object facilities... */

typedef struct Active Active; /* forward declaration */

typedef void (*DispatchHandler)(Active * const me, Event const * const e);

/* Active Object base class */
struct Active {
    TaskHandle_t thread;     /* private thread */
    StaticTask_t thread_cb;  /* thread control-block (FreeRTOS static alloc) */

    QueueHandle_t queue;     /* private message queue */
    StaticQueue_t queue_cb;  /* queue control-block (FreeRTOS static alloc) */

    DispatchHandler dispatch; /* pointer to the dispatch() function */

    /* active object data added in subclasses of Active */
};

void Active_ctor(Active * const me, DispatchHandler dispatch);
void Active_start(Active * const me,
                  uint8_t prio,       /* priority (1-based) */
                  Event **queueSto,
                  uint32_t queueLen,
                  void *stackSto,
                  uint32_t stackSize,
                  uint16_t opt);
void Active_post(Active * const me, Event const * const e);
void Active_postFromISR(Active * const me, Event const * const e,
                        BaseType_t *pxHigherPriorityTaskWoken);

/*---------------------------------------------------------------------------*/
/* Time Event facilities... */

/* Time Event class */
typedef struct {
    Event super;                /* inherit Event */
    Active *act;                /* the AO that requested this TimeEvent */
    TimerHandle_t timer;        /* private timer handle */
    StaticTimer_t timer_cb;     /* timer control-block (FreeRTOS static alloc) */
    TimerType_t type;           /* timer type, periodic or one-shot */
} TimeEvent;

void TimeEvent_ctor(TimeEvent * const me, Signal sig, Active *act);
void TimeEvent_arm(TimeEvent * const me, uint32_t millisec);
void TimeEvent_disarm(TimeEvent * const me);

/* static (i.e., class-wide) operation */
void TimeEvent_tickFromISR(BaseType_t *pxHigherPriorityTaskWoken);

/*---------------------------------------------------------------------------*/
/* Assertion facilities... */

#define Q_ASSERT(check_)                   \
    if (!(check_)) {                       \
        Q_onAssert(this_module, __LINE__); \
    } else (void)0

void Q_onAssert(char const *module, int loc);

#endif /* FREE_ACT_H */
