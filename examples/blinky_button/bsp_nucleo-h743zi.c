/*============================================================================
* Lab Project: Blinky/Button with RTOS (FreeRTOS) and blocking
* Board: STM32 NUCLEO-H743ZI
*
*                    Q u a n t u m  L e a P s
*                    ------------------------
*                    Modern Embedded Software
*
* Copyright (C) 2005 Quantum Leaps, <state-machine.com>.
*
* SPDX-License-Identifier: MIT
============================================================================*/
#include "FreeAct.h" /* Free Active Object interface */
#include "bsp.h"

#include "stm32h743xx.h"  /* CMSIS-compliant header file for the MCU used */
#include <math.h>         /* to exercise the FPU */
/* add other drivers if necessary... */

/* Local-scope defines -----------------------------------------------------*/

/* test pins on GPIO PB */
#define LED0_PIN  0U  /* PB.0  LED1-Green */
#define LED1_PIN  14U /* PB.14 LED3-Red   */
#define LED2_PIN  7U  /* PB.7  LED2-Blue  */

/* buttons on GPIO PC */
#define B1_PIN    13U

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

    /* Perform the debouncing of buttons. The algorithm for debouncing
    * adapted from the book "Embedded Systems Dictionary" by Jack Ganssle
    * and Michael Barr, page 71.
    */
    /* Perform the debouncing of buttons. The algorithm for debouncing
    * adapted from the book "Embedded Systems Dictionary" by Jack Ganssle
    * and Michael Barr, page 71.
    */
    static struct ButtonsDebouncing {
        uint32_t depressed;
        uint32_t previous;
    } buttons = { 0U, 0U };
    uint32_t current = GPIOC->IDR; /* read GPIO PortC */
    uint32_t tmp = buttons.depressed; /* save the debounced depressed */
    buttons.depressed |= (buttons.previous & current); /* set depressed */
    buttons.depressed &= (buttons.previous | current); /* clear released */
    buttons.previous   = current; /* update the history */
    tmp ^= buttons.depressed;     /* changed debounced depressed */
    if ((tmp & (1U << B1_PIN)) != 0U) { /* debounced B1 state changed? */
        if ((buttons.depressed & (1U << B1_PIN)) != 0U) { /* depressed? */
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

    /* Configure the MPU to prevent NULL-pointer dereferencing
    * see: www.state-machine.com/null-pointer-protection-with-arm-cortex-m-mpu
    */
    MPU->RBAR = 0x0U                          /* base address (NULL) */
                | MPU_RBAR_VALID_Msk          /* valid region */
                | (MPU_RBAR_REGION_Msk & 7U); /* region #7 */
    MPU->RASR = (7U << MPU_RASR_SIZE_Pos)     /* 2^(7+1) region */
                | (0x0U << MPU_RASR_AP_Pos)   /* no-access region */
                | MPU_RASR_ENABLE_Msk;        /* region enable */

    MPU->CTRL = MPU_CTRL_PRIVDEFENA_Msk       /* enable background region */
                | MPU_CTRL_ENABLE_Msk;        /* enable the MPU */
    __ISB();
    __DSB();

    SCB_EnableICache(); /* Enable I-Cache */
    SCB_EnableDCache(); /* Enable D-Cache */

    /* enable GPIOB port clock for LEds and test pins */
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOBEN;

    /* set all used GPIOB pins as push-pull output, no pull-up, pull-down */
    GPIOB->MODER &=
        ~((3U << 2U*LED0_PIN) | (3U << 2U*LED1_PIN) | (3U << 2U*LED2_PIN));
    GPIOB->MODER |=
         ((1U << 2U*LED0_PIN) | (1U << 2U*LED1_PIN) | (1U << 2U*LED2_PIN));
    GPIOB->OTYPER &=
        ~((1U <<    LED0_PIN) | (1U <<    LED1_PIN) | (1U <<    LED2_PIN));
    GPIOB->PUPDR &=
        ~((3U << 2U*LED0_PIN) | (3U << 2U*LED1_PIN) | (3U << 2U*LED2_PIN));

    /* enable GPIOC clock port for the Button B1 */
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOCEN;

    /* configure Button B1 pin on GPIOC as input, no pull-up, pull-down */
    GPIOC->MODER   &= ~(3U << 2U*B1_PIN);
    GPIOC->PUPDR   &= ~(GPIO_PUPDR_PUPD0 << 2U*B1_PIN);
    GPIOC->PUPDR   |=  (2U << 2U*B1_PIN);
}
/*..........................................................................*/
void BSP_led0_off(void) {
    GPIOB->BSRR = (1U << (LED0_PIN + 16U));
}
/*..........................................................................*/
void BSP_led0_on(void) {
    GPIOB->BSRR = (1U << LED0_PIN);
}
/*..........................................................................*/
void BSP_led1_off(void) {
    GPIOB->BSRR = (1U << (LED1_PIN + 16U));
}
/*..........................................................................*/
void BSP_led1_on(void) {
    GPIOB->BSRR = (1U << LED1_PIN);
}
/*..........................................................................*/
void BSP_start(void) {
    /* set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate */
    /* SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC); done in FreeRTOS */

    /* set priorities of ISRs used in the system */
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
    /* light-all both LEDs */
    GPIOB->BSRR = (1U << LED0_PIN);
    GPIOB->BSRR = (1U << LED1_PIN);
    GPIOB->BSRR = (1U << LED2_PIN);

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

