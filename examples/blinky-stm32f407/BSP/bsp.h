/*****************************************************************************
* Lab Project: BlinkyButton/Button with Active Objects and FreeRTOS
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
#ifndef BSP_H
#define BSP_H

#include "FreeAct.h"

void BSP_init(void);
void BSP_start(void);
void BSP_led0_on(void);
void BSP_led0_off(void);
void BSP_led1_on(void);
void BSP_led1_off(void);
void BSP_led2_on(void);
void BSP_led2_off(void);
void BSP_led0_toggle(void);
void BSP_led1_toggle(void);
void BSP_led2_toggle(void);

enum Signals {
    TIMEOUT_SIG = USER_SIG,
    BUTTON_PRESSED_SIG,
    BUTTON_RELEASED_SIG,
};

extern Active *AO_blinkyButton;

#endif /* BSP_H */
