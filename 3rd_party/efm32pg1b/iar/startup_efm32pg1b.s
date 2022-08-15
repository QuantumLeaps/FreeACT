;/***************************************************************************/
; * @file     startup_efm32pg1b.s for IAR ARM assembler
; * @brief    CMSIS Cortex-M4F Core Device Startup File for EFM32PG1B MCU
; * @version  CMSIS 5.9.0
; * @date     1 Feb 2023
; *
; * Modified by Quantum Leaps:
; * - Added relocating of the Vector Table to free up the 256B region at 0x0
; *   for NULL-pointer protection by the MPU.
; * - Modified all exception handlers to branch to assert_failed()
; *   instead of locking up the CPU inside an endless loop.
; *
; * @description
; * Created from the CMSIS template for the specified device
; * Quantum Leaps, www.state-machine.com
; *

        MODULE  ?cstartup

        ; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(8)

        PUBLIC  __vector_table
        PUBLIC  __Vectors
        PUBLIC  __Vectors_End
        PUBLIC  __Vectors_Size

;******************************************************************************
; The vector table.
;
        DATA
__vector_table
    ; Initial Vector Table before relocation
        DCD     sfe(CSTACK)
        DCD     Reset_Handler               ; Reset Handler
        DCD     NMI_Handler                 ; NMI Handler
        DCD     HardFault_Handler           ; Hard Fault Handler
        DCD     MemManage_Handler           ; MPU fault handler
        DCD     BusFault_Handler            ; Bus fault handler
        DCD     UsageFault_Handler          ; Usage fault handler
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     SVC_Handler                 ; SVCall handler
        DCD     DebugMon_Handler            ; Debug Monitor handler
        DCD     Default_Handler             ; Reserved
        DCD     PendSV_Handler              ; PendSV handler
        DCD     SysTick_Handler             ; SysTick handler
        ALIGNROM 8   ; Extend the initial Vector Table to the 2^8==256B

    ; Relocated Vector Table beyond the 256B region around address 0.
    ; That region is used for NULL-pointer protection by the MPU.
__relocated_vector_table
        DCD     sfe(CSTACK)
        DCD     Reset_Handler               ; Reset Handler
        DCD     NMI_Handler                 ; NMI Handler
        DCD     HardFault_Handler           ; Hard Fault Handler
        DCD     MemManage_Handler           ; MPU fault handler
        DCD     BusFault_Handler            ; Bus fault handler
        DCD     UsageFault_Handler          ; Usage fault handler
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     Default_Handler             ; Reserved
        DCD     SVC_Handler                 ; SVCall handler
        DCD     DebugMon_Handler            ; Debug Monitor handler
        DCD     Default_Handler             ; Reserved
        DCD     PendSV_Handler              ; PendSV handler
        DCD     SysTick_Handler             ; SysTick handler

        ; IRQ handlers...
        DCD     EMU_IRQHandler              ; 0 - EMU
        DCD     0                           ; 1 - Reserved
        DCD     WDOG0_IRQHandler            ; 2 - WDOG0
        DCD     0                           ; 3 - Reserved
        DCD     0                           ; 4 - Reserved
        DCD     0                           ; 5 - Reserved
        DCD     0                           ; 6 - Reserved
        DCD     0                           ; 7 - Reserved
        DCD     LDMA_IRQHandler             ; 8 - LDMA
        DCD     GPIO_EVEN_IRQHandler        ; 9 - GPIO_EVEN
        DCD     TIMER0_IRQHandler           ; 10 - TIMER0
        DCD     USART0_RX_IRQHandler        ; 11 - USART0_RX
        DCD     USART0_TX_IRQHandler        ; 12 - USART0_TX
        DCD     ACMP0_IRQHandler            ; 13 - ACMP0
        DCD     ADC0_IRQHandler             ; 14 - ADC0
        DCD     IDAC0_IRQHandler            ; 15 - IDAC0
        DCD     I2C0_IRQHandler             ; 16 - I2C0
        DCD     GPIO_ODD_IRQHandler         ; 17 - GPIO_ODD
        DCD     TIMER1_IRQHandler           ; 18 - TIMER1
        DCD     USART1_RX_IRQHandler        ; 19 - USART1_RX
        DCD     USART1_TX_IRQHandler        ; 20 - USART1_TX
        DCD     LEUART0_IRQHandler          ; 21 - LEUART0
        DCD     PCNT0_IRQHandler            ; 22 - PCNT0
        DCD     CMU_IRQHandler              ; 23 - CMU
        DCD     MSC_IRQHandler              ; 24 - MSC
        DCD     CRYPTO_IRQHandler           ; 25 - CRYPTO
        DCD     LETIMER0_IRQHandler         ; 26 - LETIMER0
        DCD     0                           ; 27 - Reserved
        DCD     0                           ; 28 - Reserved
        DCD     RTCC_IRQHandler             ; 29 - RTCC
        DCD     0                           ; 30 - Reserved
        DCD     CRYOTIMER_IRQHandler        ; 31 - CRYOTIMER
        DCD     0                           ; 32 - Reserved
        DCD     FPUEH_IRQHandler            ; 33 - FPUEH

__Vectors_End

__Vectors       EQU   __vector_table
__Vectors_Size  EQU   __Vectors_End - __Vectors


;******************************************************************************
; This is the code for exception handlers.
;
        SECTION .text:CODE:REORDER:NOROOT(2)

;******************************************************************************
; This is the code that gets called when theessor first starts execution
; following a reset event.
;
        PUBWEAK Reset_Handler
        EXTERN  SystemInit
        EXTERN  __iar_program_start
        EXTERN  assert_failed

Reset_Handler
        ; relocate the Vector Table
        LDR     r0,=0xE000ED08  ; System Control Block/Vector Table Offset Reg
        LDR     r1,=__relocated_vector_table
        STR     r1,[r0]         ; SCB->VTOR := __relocated_vector_table

        LDR     r0,=SystemInit  ; CMSIS system initialization
        BLX     r0

        ; pre-fill the CSTACK with 0xDEADBEEF...................
        LDR     r0,=0xDEADBEEF
        MOV     r1,r0
        LDR     r2,=sfb(CSTACK)
        LDR     r3,=sfe(CSTACK)
Reset_stackInit_fill:
        STMIA   r2!,{r0,r1}
        CMP     r2,r3
        BLT.N   Reset_stackInit_fill

        LDR     r0,=__iar_program_start ; IAR startup code
        BLX     r0

        ; __iar_program_start calls the main() function, which should not return,
        ; but just in case jump to assert_failed() if main returns.
        LDR     r0,=str_EXIT
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_EXIT
        DCB     "EXIT"
        ALIGNROM 2

;******************************************************************************
        PUBWEAK NMI_Handler
NMI_Handler
        LDR     r0,=str_NMI
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_NMI
        DCB     "NMI"
        ALIGNROM 2

;******************************************************************************
        PUBWEAK HardFault_Handler
HardFault_Handler
        LDR     r0,=str_HardFault
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_HardFault
        DCB     "HardFault"
        ALIGNROM 2

;******************************************************************************
        PUBWEAK MemManage_Handler
MemManage_Handler
        LDR     r0,=str_MemManage
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_MemManage
        DCB     "MemManage"
        ALIGNROM 2

;******************************************************************************
        PUBWEAK BusFault_Handler
BusFault_Handler
        LDR     r0,=str_BusFault
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_BusFault
        DCB     "BusFault"
        ALIGNROM 2

;******************************************************************************
        PUBWEAK UsageFault_Handler
UsageFault_Handler
        LDR     r0,=str_UsageFault
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_UsageFault
        DCB     "UsageFault"
        ALIGNROM 2

;******************************************************************************
;
; Weak non-fault handlers...
;

;******************************************************************************
        PUBWEAK SVC_Handler
SVC_Handler
        LDR     r0,=str_SVC
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_SVC
        DCB     "SVC"
        ALIGNROM 2

;******************************************************************************
        PUBWEAK DebugMon_Handler
DebugMon_Handler
        LDR     r0,=str_DebugMon
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_DebugMon
        DCB     "DebugMon"
        ALIGNROM 2

;******************************************************************************
        PUBWEAK PendSV_Handler
PendSV_Handler
        LDR     r0,=str_PendSV
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_PendSV
        DCB     "PendSV"
        ALIGNROM 2

;******************************************************************************
        PUBWEAK SysTick_Handler
SysTick_Handler
        LDR     r0,=str_SysTick
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_SysTick
        DCB     "SysTick"
        ALIGNROM 2

;******************************************************************************
; Weak IRQ handlers...
;
        PUBWEAK  EMU_IRQHandler
        PUBWEAK  WDOG0_IRQHandler
        PUBWEAK  LDMA_IRQHandler
        PUBWEAK  GPIO_EVEN_IRQHandler
        PUBWEAK  TIMER0_IRQHandler
        PUBWEAK  USART0_RX_IRQHandler
        PUBWEAK  USART0_TX_IRQHandler
        PUBWEAK  ACMP0_IRQHandler
        PUBWEAK  ADC0_IRQHandler
        PUBWEAK  IDAC0_IRQHandler
        PUBWEAK  I2C0_IRQHandler
        PUBWEAK  GPIO_ODD_IRQHandler
        PUBWEAK  TIMER1_IRQHandler
        PUBWEAK  USART1_RX_IRQHandler
        PUBWEAK  USART1_TX_IRQHandler
        PUBWEAK  LEUART0_IRQHandler
        PUBWEAK  PCNT0_IRQHandler
        PUBWEAK  CMU_IRQHandler
        PUBWEAK  MSC_IRQHandler
        PUBWEAK  CRYPTO_IRQHandler
        PUBWEAK  LETIMER0_IRQHandler
        PUBWEAK  RTCC_IRQHandler
        PUBWEAK  CRYOTIMER_IRQHandler
        PUBWEAK  FPUEH_IRQHandler

EMU_IRQHandler
WDOG0_IRQHandler
LDMA_IRQHandler
GPIO_EVEN_IRQHandler
TIMER0_IRQHandler
USART0_RX_IRQHandler
USART0_TX_IRQHandler
ACMP0_IRQHandler
ADC0_IRQHandler
IDAC0_IRQHandler
I2C0_IRQHandler
GPIO_ODD_IRQHandler
TIMER1_IRQHandler
USART1_RX_IRQHandler
USART1_TX_IRQHandler
LEUART0_IRQHandler
PCNT0_IRQHandler
CMU_IRQHandler
MSC_IRQHandler
CRYPTO_IRQHandler
LETIMER0_IRQHandler
RTCC_IRQHandler
CRYOTIMER_IRQHandler
FPUEH_IRQHandler
        LDR     r0,=str_Undefined
        MOVS    r1,#1
        LDR     r2,=sfe(CSTACK)  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_Undefined
        DCB     "Undefined"
        ALIGNROM 2

        END                     ; end of module

