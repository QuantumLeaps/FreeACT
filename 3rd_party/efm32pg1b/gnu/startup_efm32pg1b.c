/* File: startup_efm32pg1b.c for GNU-ARM
 * Purpose: startup file for TM4C123GH6PM Cortex-M4 device.
 *          Should be used with GCC 'GNU Tools ARM Embedded'
 * Version: CMSIS 5.0.1
 * Date: 2017-09-13
 *
 * Modified by Quantum Leaps:
 * - Added relocating of the Vector Table to free up the 256B region at 0x0
 *   for NULL-pointer protection by the MPU.
 * - Modified all exception handlers to branch to assert_failed()
 *   instead of locking up the CPU inside an endless loop.
 *
 * Created from the CMSIS template for the specified device
 * Quantum Leaps, www.state-machine.com
 *
 * NOTE:
 * The function assert_failed defined at the end of this file defines
 * the error/assertion handling policy for the application and might
 * need to be customized for each project. This function is defined in
 * assembly to re-set the stack pointer, in case it is corrupted by the
 * time assert_failed is called.
 */

/* start and end of stack defined in the linker script ---------------------*/
/*extern int __stack_start__;*/
extern int __stack_end__;

/* Weak prototypes for error handlers --------------------------------------*/
/**
* \note
* The function assert_failed defined at the end of this file defines
* the error/assertion handling policy for the application and might
* need to be customized for each project. This function is defined in
* assembly to avoid accessing the stack, which might be corrupted by
* the time assert_failed is called.
*/
__attribute__ ((naked, noreturn))
void assert_failed(char const *module, int loc);

/* Function prototypes -----------------------------------------------------*/
void Default_Handler(void);  /* Default empty handler */
void Reset_Handler(void);    /* Reset Handler */
void SystemInit(void);       /* CMSIS system initialization */

/*----------------------------------------------------------------------------
* weak aliases for each Exception handler to the Default_Handler.
* Any function with the same name will override these definitions.
*/
/* Cortex-M Processor fault exceptions... */
void NMI_Handler           (void) __attribute__ ((weak));
void HardFault_Handler     (void) __attribute__ ((weak));
void MemManage_Handler     (void) __attribute__ ((weak));
void BusFault_Handler      (void) __attribute__ ((weak));
void UsageFault_Handler    (void) __attribute__ ((weak));

/* Cortex-M Processor non-fault exceptions... */
void SVC_Handler           (void) __attribute__ ((weak, alias("Default_Handler")));
void DebugMon_Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void PendSV_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void SysTick_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));

/* external interrupts...   */
void EMU_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void WDOG0_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void LDMA_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO_EVEN_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void TIMER0_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void USART0_RX_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void USART0_TX_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void ACMP0_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void ADC0_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void IDAC0_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C0_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO_ODD_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void TIMER1_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void USART1_RX_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void USART1_TX_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void LEUART0_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void PCNT0_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void CMU_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void MSC_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void CRYPTO_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void LETIMER0_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void RTCC_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void CRYOTIMER_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void FPUEH_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));

/*..........................................................................*/
__attribute__ ((section(".isr_vector")))
int const g_pfnVectors[] = {
    /* Initial Vector Table before relocation */
    (int)&__stack_end__,          /* Top of Stack                    */
    (int)&Reset_Handler,          /* Reset Handler                   */
    (int)&NMI_Handler,            /* NMI Handler                     */
    (int)&HardFault_Handler,      /* Hard Fault Handler              */
    (int)&MemManage_Handler,      /* The MPU fault handler           */
    (int)&BusFault_Handler,       /* The bus fault handler           */
    (int)&UsageFault_Handler,     /* The usage fault handler         */
    (int)&Default_Handler,        /* Reserved                        */
    (int)&Default_Handler,        /* Reserved                        */
    (int)&Default_Handler,        /* Reserved                        */
    (int)&Default_Handler,        /* Reserved                        */
    (int)&SVC_Handler,            /* SVCall handler                  */
    (int)&DebugMon_Handler,       /* Debug monitor handler           */
    (int)&Default_Handler,        /* Reserved                        */
    (int)&PendSV_Handler,         /* The PendSV handler              */
    (int)&SysTick_Handler,        /* The SysTick handler             */
    /* pad the initial VT to the total size of 256B */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    /* Relocated Vector Table beyond the 256B region around address 0.
    * That region is used for NULL-pointer protection by the MPU.
    */
    (int)&__stack_end__,          /* Top of Stack                    */
    (int)&Reset_Handler,          /* Reset Handler                   */
    (int)&NMI_Handler,            /* NMI Handler                     */
    (int)&HardFault_Handler,      /* Hard Fault Handler              */
    (int)&MemManage_Handler,      /* The MPU fault handler           */
    (int)&BusFault_Handler,       /* The bus fault handler           */
    (int)&UsageFault_Handler,     /* The usage fault handler         */
    (int)&Default_Handler,        /* Reserved                        */
    (int)&Default_Handler,        /* Reserved                        */
    (int)&Default_Handler,        /* Reserved                        */
    (int)&Default_Handler,        /* Reserved                        */
    (int)&SVC_Handler,            /* SVCall handler                  */
    (int)&DebugMon_Handler,       /* Debug monitor handler           */
    (int)&Default_Handler,        /* Reserved                        */
    (int)&PendSV_Handler,         /* The PendSV handler              */
    (int)&SysTick_Handler,        /* The SysTick handler             */

    /*IRQ handlers... */
    (int)&EMU_IRQHandler,         /*  0 - EMU                        */
    (int)&Default_Handler,        /*  1 - Reserved                   */
    (int)&WDOG0_IRQHandler,       /*  2 - WDOG0                      */
    (int)&Default_Handler,        /*  3 - Reserved                   */
    (int)&Default_Handler,        /*  4 - Reserved                   */
    (int)&Default_Handler,        /*  5 - Reserved                   */
    (int)&Default_Handler,        /*  6 - Reserved                   */
    (int)&Default_Handler,        /*  7 - Reserved                   */
    (int)&LDMA_IRQHandler,        /*  8 - LDMA                       */
    (int)&GPIO_EVEN_IRQHandler,   /*  9 - GPIO_EVEN                  */
    (int)&TIMER0_IRQHandler,      /* 10 - TIMER0                     */
    (int)&USART0_RX_IRQHandler,   /* 11 - USART0_RX                  */
    (int)&USART0_TX_IRQHandler,   /* 12 - USART0_TX                  */
    (int)&ACMP0_IRQHandler,       /* 13 - ACMP0                      */
    (int)&ADC0_IRQHandler,        /* 14 - ADC0                       */
    (int)&IDAC0_IRQHandler,       /* 15 - IDAC0                      */
    (int)&I2C0_IRQHandler,        /* 16 - I2C0                       */
    (int)&GPIO_ODD_IRQHandler,    /* 17 - GPIO_ODD                   */
    (int)&TIMER1_IRQHandler,      /* 18 - TIMER1                     */
    (int)&USART1_RX_IRQHandler,   /* 19 - USART1_RX                  */
    (int)&USART1_TX_IRQHandler,   /* 20 - USART1_TX                  */
    (int)&LEUART0_IRQHandler,     /* 21 - LEUART0                    */
    (int)&PCNT0_IRQHandler,       /* 22 - PCNT0                      */
    (int)&CMU_IRQHandler,         /* 23 - CMU                        */
    (int)&MSC_IRQHandler,         /* 24 - MSC                        */
    (int)&CRYPTO_IRQHandler,      /* 25 - CRYPTO                     */
    (int)&LETIMER0_IRQHandler,    /* 26 - LETIMER0                   */
    (int)&Default_Handler,        /* 27 - Reserved                   */
    (int)&Default_Handler,        /* 28 - Reserved                   */
    (int)&RTCC_IRQHandler,        /* 29 - RTCC                       */
    (int)&Default_Handler,        /* 30 - Reserved                   */
    (int)&CRYOTIMER_IRQHandler,   /* 31 - CRYOTIMER                  */
    (int)&Default_Handler,        /* 32 - Reserved                   */
    (int)&FPUEH_IRQHandler,       /* 33 - FPUEH                      */
};


/* reset handler -----------------------------------------------------------*/
__attribute__((naked)) void Reset_Handler(void);
void Reset_Handler(void) {
    extern int main(void);
    extern int __libc_init_array(void);
    extern unsigned __data_start;  /* start of .data in the linker script */
    extern unsigned __data_end__;  /* end of .data in the linker script */
    extern unsigned const __data_load; /* initialization values for .data  */
    extern unsigned __bss_start__; /* start of .bss in the linker script */
    extern unsigned __bss_end__;   /* end of .bss in the linker script */
    extern void software_init_hook(void) __attribute__((weak));

    /* relocate the Vector Table to leave room for the NULL-pointer region
    * System Control Block/Vector Table Offset Reg := relocated Vector Table
    */
    *(int const * volatile *)0xE000ED08 = &g_pfnVectors[256/sizeof(int)];

    SystemInit(); /* CMSIS system initialization */

    /* copy the data segment initializers from flash to RAM... */
    unsigned const *src = &__data_load;
    unsigned *dst;
    for (dst = &__data_start; dst < &__data_end__; ++dst, ++src) {
        *dst = *src;
    }

    /* zero fill the .bss segment in RAM... */
    for (dst = &__bss_start__; dst < &__bss_end__; ++dst) {
        *dst = 0;
    }

    /* init hook provided? */
    if (&software_init_hook != (void (*)(void))(0)) {
        /* give control to the RTOS */
        software_init_hook(); /* this will also call __libc_init_array */
    }
    else {
        /* call all static constructors in C++ (harmless in C programs) */
        __libc_init_array();
        (void)main(); /* application's entry point; should never return! */
    }

    /* the previous code should not return, but assert just in case... */
    assert_failed("Reset_Handler", 1U);
}


/* fault exception handlers ------------------------------------------------*/
__attribute__((naked)) void NMI_Handler(void);
void NMI_Handler(void) {
    /* reset the SP to the initial value in case of stack overflow */
    __asm volatile ("  MOV  sp,%0" : : "r" (&__stack_end__));
    assert_failed("NMI_Handler", 1U);
}
/*..........................................................................*/
__attribute__((naked)) void HardFault_Handler(void);
void HardFault_Handler(void) {
    /* reset the SP to the initial value in case of stack overflow */
    __asm volatile ("  MOV  sp,%0" : : "r" (&__stack_end__));
    assert_failed("HardFault_Handler", 1U);
}
/*..........................................................................*/
__attribute__((naked)) void MemManage_Handler(void);
void MemManage_Handler(void) {
    /* reset the SP to the initial value in case of stack overflow */
    __asm volatile ("  MOV  sp,%0" : : "r" (&__stack_end__));
    assert_failed("MemManage_Handler", 1U);
}
/*..........................................................................*/
__attribute__((naked)) void BusFault_Handler(void);
void BusFault_Handler(void) {
    /* reset the SP to the initial value in case of stack overflow */
    __asm volatile ("  MOV  sp,%0" : : "r" (&__stack_end__));
    assert_failed("MemManage_Handler", 1U);
}
/*..........................................................................*/
__attribute__((naked)) void UsageFault_Handler(void);
void UsageFault_Handler(void) {
    /* reset the SP to the initial value in case of stack overflow */
    __asm volatile ("  MOV  sp,%0" : : "r" (&__stack_end__));
    assert_failed("BusFault_Handler", 1U);
}
/*..........................................................................*/
__attribute__((naked)) void Default_Handler(void);
void Default_Handler(void) {
    /* reset the SP to the initial value in case of stack overflow */
    __asm volatile ("  MOV  sp,%0" : : "r" (&__stack_end__));
    assert_failed("Default_Handler", 1U);
}
