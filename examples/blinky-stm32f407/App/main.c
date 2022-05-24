/*****************************************************************************
* Lab Project: BlinkyButton/Button with RTOS (FreeRTOS) and blocking
* Board: STM32F407 custom
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
*
* Ported to STM32F407 by fjpolo@gmail.com
*
*****************************************************************************/

/**
* \brief Includes
*/
#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common




#ifndef USE_ACTIVE_OBJECTS
/**
* \brief Includes
*/
/*BSP*/
#include "leds.h"
/*OS*/
#include "FreeRTOSConfig.h"             // ARM.FreeRTOS::RTOS:Config
#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h"                       // ARM.FreeRTOS::RTOS:Core
#include "timers.h"                     // ARM.FreeRTOS::RTOS:Timers
#include "queue.h"                      // ARM.FreeRTOS::RTOS:Core
#include "semphr.h"                     // ARM.FreeRTOS::RTOS:Core

/**
* \brief Constants
*/


/**
* \brief Imported variables
*/


/**
* \brief Global variables
*/
uint32_t counter1, counter2, counter3;


/**
* \brief Threads
*/

/*vRed_Thread*/
void vRed_Thread(void *pvParameters){
	while(true){
		counter1++;
		RED_toggle();
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

/**
* \brief vGreen_Thread
*/
void vGreen_Thread(void *pvParameters){
	while(true){
		counter2++;
		GREEN_toggle();
		vTaskDelay(pdMS_TO_TICKS(125));
	}
}

/**
* \brief vBlue_Thread
*/
void vYellow_Thread(void *pvParameters){
	while(true){
		counter3++;
		YELLOW_toggle();
		vTaskDelay(pdMS_TO_TICKS(250));
	}
}



/**
* \fn main
*
* \brief 
*/
int main(void){
	static uint32_t freq = 0;
	
	/*Init BSP*/
	LEDS_init();
	
	/*Init counters*/
	counter1 = 0; 
	counter2 = 0; 
	counter3 = 0;
	
	/*OS*/
	xTaskCreate(
								vRed_Thread,
								"Red Led Task",
								100,
								NULL,
								1,
								NULL							
							);
	xTaskCreate(
								vGreen_Thread,
								"Green Led Task",
								100,
								NULL,
								1,
								NULL							
							);
	xTaskCreate(
								vYellow_Thread,
								"Yellow Led Task",
								100,
								NULL,
								1,
								NULL							
							);

	/*Init scheduler*/
	vTaskStartScheduler();

	/*Superloop - Unreachable*/
	while(true){}
	
}
#endif // USE_ACTIVE_OBJECTS


#ifdef USE_ACTIVE_OBJECTS
/**
* \brief Includes
*/
#include "bsp.h"
#include "FreeAct.h"

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
            BSP_led2_off();
						break;
        }
        case BUTTON_RELEASED_SIG: {
            BSP_led1_off();
            BSP_led2_on();
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
# endif // USE_ACTIVE_OBJECTS
