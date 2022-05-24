/*****************************************************************************
* Lab Project: Blinky/Button with RTOS (FreeRTOS)
* Board: EK-TM4C123GXL
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
#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common
#include "leds.h"

/* on-board LEDs */
#define LED_RED_PORT           (GPIOB)
#define LED_GREEN_PORT      (GPIOI)
#define LED_YELLOW_PORT     (GPIOG)
#define LED_RED_PIN           (GPIO_PIN_7)
#define LED_GREEN_PIN          (GPIO_PIN_7)
#define LED_YELLOW_PIN         (GPIO_PIN_11)

/* on-board switch */
#define BTN_SW1                        (1U >> 0U)
#define BTN_SW1_PORT           (GPIOD)
#define BTN_SW1_PIN           (GPIO_PIN_7)

/* Hooks ===================================================================*/
/* Application hooks used in this project ==================================*/
/* NOTE: only the "FromISR" API variants are allowed in vApplicationTickHook*/
void vApplicationTickHook(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* state of the button debouncing, see below */
    static struct ButtonsDebouncing {
        uint32_t depressed;
        uint32_t previous;
    } buttons = { 0U, 0U };
    uint32_t current;
    uint32_t tmp;

    /* perform clock tick processing */
    TimeEvent_tickFromISR(&xHigherPriorityTaskWoken);

    /* Perform the debouncing of buttons. The algorithm for debouncing
    * adapted from the book "Embedded Systems Dictionary" by Jack Ganssle
    * and Michael Barr, page 71.
    */
//    current = ~GPIOF_AHB->DATA_Bits[BTN_SW1]; /* read SW1 */
        current = HAL_GPIO_ReadPin(BTN_SW1_PORT, BTN_SW1_PIN);
    tmp = buttons.depressed; /* save the debounced depressed buttons */
    buttons.depressed |= (buttons.previous & current); /* set depressed */
    buttons.depressed &= (buttons.previous | current); /* clear released */
    buttons.previous   = current; /* update the history */
    tmp ^= buttons.depressed;     /* changed debounced depressed */

    /* debounced SW1 state changed? */
        if ((tmp & BTN_SW1) != 0U) {
       /* is SW1 depressed? */
            if ((buttons.depressed & BTN_SW1) != 0U) {
                /* post the "button-pressed" event from ISR */
                static Event const buttonPressedEvt = {BUTTON_PRESSED_SIG};
                Active_postFromISR(
                                                        AO_blinkyButton,
                                                        &buttonPressedEvt,
                                                        &xHigherPriorityTaskWoken
                                                    );
        }
        /* the button is released */
                else {
                 /* post the "button-released" event from ISR */
                 static Event const buttonReleasedEvt = {BUTTON_RELEASED_SIG};
                 Active_postFromISR(
                                                        AO_blinkyButton,
                                                        &buttonReleasedEvt,
                                                        &xHigherPriorityTaskWoken
                                                     );
        }
    }

    /* notify FreeRTOS to perform context switch from ISR, if needed */
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}
/*..........................................................................*/
void vApplicationIdleHook(void) {
#ifdef NDEBUG
    /* Put the CPU and peripherals to the low-power mode.
    * you might need to customize the clock management for your application,
    * see the datasheet for your particular Cortex-M3 MCU.
    */
    __WFI(); /* Wait-For-Interrupt */
#endif
}
/*..........................................................................*/
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask;
    (void)pcTaskName;
    /* ERROR!!! */
}
/*..........................................................................*/
/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must
* provide an implementation of vApplicationGetIdleTaskMemory() to provide
* the memory that is used by the Idle task.
*/
void vApplicationGetIdleTaskMemory(
                                                                        StaticTask_t **ppxIdleTaskTCBBuffer,
                                                                        StackType_t **ppxIdleTaskStackBuffer,
                                                                        uint32_t *pulIdleTaskStackSize
)
{
    /* If the buffers to be provided to the Idle task are declared inside
    * this function then they must be declared static - otherwise they will
    * be allocated on the stack and so not exists after this function exits.
    */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

    /* Pass out a pointer to the StaticTask_t structure in which the
    * Idle task's state will be stored.
    */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    * Note that, as the array is necessarily of type StackType_t,
    * configMINIMAL_STACK_SIZE is specified in words, not bytes.
    */
    *pulIdleTaskStackSize = sizeof(uxIdleTaskStack)/sizeof(uxIdleTaskStack[0]);
}

/* BSP functions ===========================================================*/
void BSP_init(void) {
    /**/
    LEDS_init();

}
/*..........................................................................*/
void BSP_led0_off(void) {
    HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_RESET);
}
/*..........................................................................*/
void BSP_led0_on(void) {
    HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_SET);
}
/*..........................................................................*/
void BSP_led1_off(void) {
    HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_RESET);
}
/*..........................................................................*/
void BSP_led1_on(void) {
    HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_SET);
}
/*..........................................................................*/
void BSP_led2_off(void) {
    HAL_GPIO_WritePin(LED_YELLOW_PORT, LED_YELLOW_PIN, GPIO_PIN_RESET);
}
/*..........................................................................*/
void BSP_led2_on(void) {
    HAL_GPIO_WritePin(LED_YELLOW_PORT, LED_YELLOW_PIN, GPIO_PIN_SET);
}
/*..........................................................................*/
void BSP_led0_toggle(void){
    HAL_GPIO_TogglePin(LED_RED_PORT, LED_RED_PIN);
}
/*..........................................................................*/
void BSP_led1_toggle(void){
    HAL_GPIO_TogglePin(LED_GREEN_PORT, LED_GREEN_PIN);
}
/*..........................................................................*/
void BSP_led2_toggle(void){
    HAL_GPIO_TogglePin(LED_YELLOW_PORT, LED_YELLOW_PIN);
}
/*..........................................................................*/
void BSP_start(void) {
    /* set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate */
    /* SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC); done in FreeRTOS */

    /* assign all priority bits for preemption-prio. and none to sub-prio. */
    NVIC_SetPriorityGrouping(0U);

    /* set priorities of ALL ISRs used in the system, see NOTE1 */
    NVIC_SetPriority(SysTick_IRQn, 1U + configMAX_SYSCALL_INTERRUPT_PRIORITY);
    /* ... */

    /* enable IRQs... */
    /* ... */
}
/*..........................................................................*/
/* error-handling function called by exception handlers in the startup code */
void Q_onAssert(char const *module, int loc) {
    /* NOTE: add here your application-specific error handling */
    (void)module;
    (void)loc;
#ifndef NDEBUG /* debug build? */
    /* light-up all LEDs */
        HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_YELLOW_PORT, LED_YELLOW_PIN, GPIO_PIN_RESET);

    /* tie the CPU in this endless loop and wait for the debugger... */
    while (1) {
    }
#else /* production build */
    /* TODO: do whatever is necessary to put the system in a fail-safe state */
    /* important!!! */
    NVIC_SystemReset(); /* reset the CPU */
#endif
}

/*****************************************************************************
* NOTE1:
* Only ISRs prioritized at or below the
* configMAX_SYSCALL_INTERRUPT_PRIORITY level (i.e.,
* with the numerical values of priorities equal or higher than
* configMAX_SYSCALL_INTERRUPT_PRIORITY) are allowed to call any
* QP/FreeRTOS services. These ISRs are "kernel-aware".
*
* Only ISRs prioritized at or below the configMAX_SYSCALL_INTERRUPT_PRIORITY
* level (i.e., with the numerical values of priorities equal or higher than
* configMAX_SYSCALL_INTERRUPT_PRIORITY) are allowed to call any QF services.
* These ISRs are "kernel-aware".
*
* Conversely, any ISRs prioritized above the
* configMAX_SYSCALL_INTERRUPT_PRIORITY priority level (i.e., with
* the numerical values of priorities less than
* configMAX_SYSCALL_INTERRUPT_PRIORITY) are never disabled and are
* not aware of the kernel. Such "kernel-unaware" ISRs cannot call any
* QP/FreeRTOS services. The only mechanism by which a "kernel-unaware" ISR
* can communicate with the QF framework is by triggering a "kernel-aware"
* ISR, which can post/publish events.
*
* For more information, see article "Running the RTOS on a ARM Cortex-M Core"
* http://www.freertos.org/RTOS-Cortex-M3-M4.html
*/

