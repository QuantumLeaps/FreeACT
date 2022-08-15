/*****************************************************************************
* Lab Project: Blinky/Button with RTOS (FreeRTOS) and blocking
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

#include "em_device.h"  /* the device specific header (SiLabs) */
#include "em_cmu.h"     /* Clock Management Unit (SiLabs) */
#include "em_gpio.h"    /* GPIO (SiLabs) */
/* add other drivers if necessary... */

/* LEDs and Push-buttons on the EMF32-SLSTK3401A board ---------------------*/
#define LED0_PORT  gpioPortF
#define LED0_PIN   4

#define LED1_PORT  gpioPortF
#define LED1_PIN   5

#define PB_PORT    gpioPortF
#define PB0_PIN    6
#define PB1_PIN    7

/* Function Prototype ======================================================*/
void vApplicationTickHook(void);
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize);
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize);

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

    /* Perform the debouncing of buttons. The algorithm for debouncing
    * adapted from the book "Embedded Systems Dictionary" by Jack Ganssle
    * and Michael Barr, page 71.
    */
    current = ~GPIO->P[PB_PORT].DIN; /* read PB0 and BP1 */
    tmp = buttons.depressed; /* save the debounced depressed buttons */
    buttons.depressed |= (buttons.previous & current); /* set depressed */
    buttons.depressed &= (buttons.previous | current); /* clear released */
    buttons.previous   = current; /* update the history */
    tmp ^= buttons.depressed;     /* changed debounced depressed */

    if ((tmp & (1U << PB0_PIN)) != 0U) {  /* debounced PB0 state changed? */
        if ((buttons.depressed & (1U << PB0_PIN)) != 0U) { /* PB0 depressed? */
            /* post the "button-pressed" event from ISR */
            static Event const buttonPressedEvt = {BUTTON_PRESSED_SIG};
            Active_postFromISR(AO_blinkyButton, &buttonPressedEvt,
                               &xHigherPriorityTaskWoken);
        }
        else { /* the button is released */
             /* post the "button-released" event from ISR */
             static Event const buttonReleasedEvt = {BUTTON_RELEASED_SIG};
             Active_postFromISR(AO_blinkyButton, &buttonReleasedEvt,
                                 &xHigherPriorityTaskWoken);
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
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
    /* If the buffers to be provided to the Idle task are declared inside
     * this function then they must be declared static - otherwise they will
     * be allocated on the stack and so not exists after this function exits.
     */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t  uxIdleTaskStack[configMINIMAL_STACK_SIZE];

    /* Pass out a pointer to the StaticTask_t structure in which the
     * Idle task's state will be stored.
     */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = &uxIdleTaskStack[0];

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes.
     */
    *pulIdleTaskStackSize = sizeof(uxIdleTaskStack) / sizeof(uxIdleTaskStack[0]);
}

/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must
 * provide an implementation of vApplicationGetTimerTaskMemory() to provide
 * the memory that is used by the Timer task.
 */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize) {
    /* If the buffers to be provided to the Timer task are declared inside
     * this function then they must be declared static - otherwise they will
     * be allocated on the stack and so not exists after this function exits.
     */
    static StaticTask_t xTimerTask_TCB;
    static StackType_t  uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

    /* Pass out a pointer to the StaticTask_t structure in which the
     * Timer task's state will be stored.
     */
    *ppxTimerTaskTCBBuffer   = &xTimerTask_TCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = &uxTimerTaskStack[0];

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configTIMER_TASK_STACK_DEPTH is specified in words, not bytes.
     */
    *pulTimerTaskStackSize   = (uint32_t)configTIMER_TASK_STACK_DEPTH;
}

/* BSP functions ===========================================================*/
void BSP_init(void) {
    /* NOTE: SystemInit() already called from the startup code
    *  but SystemCoreClock needs to be updated
    */
    SystemCoreClockUpdate();

    /* configure the FPU usage by choosing one of the options...
    *
    * Do NOT to use the automatic FPU state preservation and
    * do NOT to use the FPU lazy stacking.
    */
    /* NOTE:
    * Use the following setting when FPU is used only by active objects
    * and NOT in any ISR. This setting is very efficient, but if any ISRs
    * start using the FPU, this can lead to corruption of the FPU registers.
    */
    FPU->FPCCR &= ~((1U << FPU_FPCCR_ASPEN_Pos) | (1U << FPU_FPCCR_LSPEN_Pos));

    /* enable clock for to the peripherals used by this application... */
    CMU_ClockEnable(cmuClock_HFPER, true);
    CMU_ClockEnable(cmuClock_GPIO, true);

    /* configure the LEDs */
    GPIO_PinModeSet(LED0_PORT, LED0_PIN, gpioModePushPull, 0);
    GPIO_PinModeSet(LED1_PORT, LED1_PIN, gpioModePushPull, 0);
    GPIO_PinOutClear(LED0_PORT, LED0_PIN);
    GPIO_PinOutClear(LED1_PORT, LED1_PIN);

    /* configure the Buttons */
    GPIO_PinModeSet(PB_PORT, PB0_PIN, gpioModeInputPull, 1);
    GPIO_PinModeSet(PB_PORT, PB1_PIN, gpioModeInputPull, 1);
}
/*..........................................................................*/
void BSP_led0_off(void) {
    GPIO->P[LED0_PORT].DOUT &= ~(1U << LED0_PIN);
}
/*..........................................................................*/
void BSP_led0_on(void) {
    GPIO->P[LED0_PORT].DOUT |= (1U << LED0_PIN);
}
/*..........................................................................*/
void BSP_led1_off(void) {
    GPIO->P[LED1_PORT].DOUT &= ~(1U << LED1_PIN);
}
/*..........................................................................*/
void BSP_led1_on(void) {
    GPIO->P[LED1_PORT].DOUT |= (1U << LED1_PIN);
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
void assert_failed(char const *module, int loc); /* prototype */
void assert_failed(char const *module, int loc) {
    /* NOTE: add here your application-specific error handling */
    (void)module;
    (void)loc;
#ifndef NDEBUG /* debug build? */
    /* light-up both LEDs */
    GPIO->P[LED0_PORT].DOUT |= (1U << LED0_PIN);
    GPIO->P[LED1_PORT].DOUT |= (1U << LED1_PIN);

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

