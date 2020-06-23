/*****************************************************************************
* Lab Project: BlinkyButton/Button with RTOS (FreeRTOS) and blocking
* Board: EMF32-SLSTK3401A
*
*                    Q u a n t u m  L e a P s
*                    ------------------------
*                    Modern Embedded Software
*
* Copyright (C) 2005-2020 Quantum Leaps, LLC. All rights reserved.
*
* Contact information:
* <www.state-machine.com>
* <info@state-machine.com>
*****************************************************************************/
#include "FreeAct.h" /* Free Active Object interface */
#include "bsp.h"
#include <stdbool.h>

/* The BlinkyButton AO ===========================================================*/
typedef struct {
    Active super; /* inherit Active base class */
    /* add private data for the AO... */
    TimeEvent te;
    bool isLedOn;
} BlinkyButton;

static void BlinkyButton_dispatch(BlinkyButton * const me, Event const * const e) {
    switch (e->sig) {
        case INIT_SIG: /* intentionally fall through... */
        case TIMEOUT_SIG: {
            if (!me->isLedOn) { /* LED not on */
                BSP_led0_on();
                me->isLedOn = true;
                TimeEvent_arm(&me->te, (200 / portTICK_RATE_MS), 0U);
            }
            else {  /* LED is on */
                BSP_led0_off();
                me->isLedOn = false;
                TimeEvent_arm(&me->te, (800 / portTICK_RATE_MS), 0U);
            }
            break;
        }
        case BUTTON_PRESSED_SIG: {
            BSP_led1_on();
            break;
        }
        case BUTTON_RELEASED_SIG: {
            BSP_led1_off();
            break;
        }
        default: {
            break;
        }
    }
}
void BlinkyButton_ctor(BlinkyButton * const me) {
    Active_ctor(&me->super, (DispatchHandler)&BlinkyButton_dispatch);
    TimeEvent_ctor(&me->te, TIMEOUT_SIG, &me->super);
    me->isLedOn = false;
}
static StackType_t blinkyButton_stack[configMINIMAL_STACK_SIZE]; /* task stack */
static Event *blinkyButton_queue[10];
static BlinkyButton blinkyButton;
Active *AO_blinkyButton = &blinkyButton.super;

/* the main function =======================================================*/
int main() {

    BSP_init(); /* initialize the BSP */

    /* create and start the BlinkyButton AO */
    BlinkyButton_ctor(&blinkyButton);
    Active_start(AO_blinkyButton,
                 1U,
                 blinkyButton_queue,
                 sizeof(blinkyButton_queue)/sizeof(blinkyButton_queue[0]),
                 blinkyButton_stack,
                 sizeof(blinkyButton_stack),
                 0U);

    BSP_start(); /* configure and start interrupts */

    vTaskStartScheduler(); /* start the FreeRTOS scheduler... */
    return 0; /* NOTE: the scheduler does NOT return */
}

