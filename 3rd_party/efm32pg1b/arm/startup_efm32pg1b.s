;/***************************************************************************/
; * @file     startup_efm32pg1b.s for ARM-KEIL ARM assembler
; * @brief    CMSIS Cortex-M4F Core Device Startup File for EFM32-SLSTK3401A
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
; * @note
; * The symbols Stack_Size and Heap_Size should be provided on the command-
; * line options to the assembler, for example as:
; *     --pd "Stack_Size SETA 1024" --pd "Heap_Size SETA 0"


;******************************************************************************
; Allocate space for the stack.
;
        AREA    STACK, NOINIT, READWRITE, ALIGN=3
__stack_base
StackMem
        SPACE   Stack_Size    ; provided in command-line option, for example:
                              ; --pd "Stack_Size SETA 512"
__stack_limit
__initial_sp

;******************************************************************************
; Allocate space for the heap.
;
        AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
HeapMem
        SPACE   Heap_Size     ; provided in command-line option, for example:
                              ; --pd "Heap_Size SETA 0"
__heap_limit

; Indicate that the code in this file preserves 8-byte alignment of the stack.
        PRESERVE8

;******************************************************************************
; The vector table.
;
; Place code into the reset code section.
        AREA   RESET, DATA, READONLY, ALIGN=8
        EXPORT  __Vectors
        EXPORT  __Vectors_End
        EXPORT  __Vectors_Size

__Vectors
    ; Initial Vector Table before relocation
        DCD     __initial_sp                ; Top of Stack
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
        ALIGN  256  ; Extend the initial Vector Table to the 256B boundary

    ; Relocated Vector Table beyond the 256B region around address 0.
    ; That region is used for NULL-pointer protection by the MPU.
__relocated_vector_table
        DCD     __initial_sp                ; Top of Stack
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
        DCD     Default_Handler             ; 1 - Reserved
        DCD     WDOG0_IRQHandler            ; 2 - WDOG0
        DCD     Default_Handler             ; 3 - Reserved
        DCD     Default_Handler             ; 4 - Reserved
        DCD     Default_Handler             ; 5 - Reserved
        DCD     Default_Handler             ; 6 - Reserved
        DCD     Default_Handler             ; 7 - Reserved
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
        DCD     Default_Handler             ; 27 - Reserved
        DCD     Default_Handler             ; 28 - Reserved
        DCD     RTCC_IRQHandler             ; 29 - RTCC
        DCD     Default_Handler             ; 30 - Reserved
        DCD     CRYOTIMER_IRQHandler        ; 31 - CRYOTIMER
        DCD     Default_Handler             ; 32 - Reserved
        DCD     FPUEH_IRQHandler            ; 33 - FPUEH
__Vectors_End

__Vectors_Size  EQU   __Vectors_End - __Vectors


;******************************************************************************
; This is the code for exception handlers.
;
        AREA    |.text|, CODE, READONLY

;******************************************************************************
; This is the code that gets called when the processor first starts execution
; following a reset event.
;
Reset_Handler   PROC
        EXPORT  Reset_Handler  [WEAK]
        IMPORT  SystemInit
        IMPORT  __main
        IMPORT  assert_failed

        ; relocate the Vector Table
        LDR     r0,=0xE000ED08  ; System Control Block/Vector Table Offset Reg
        LDR     r1,=__relocated_vector_table
        STR     r1,[r0]         ; SCB->VTOR := __relocated_vector_table

        LDR     r0,=SystemInit  ; CMSIS system initialization
        BLX     r0

        ; Call the C library enty point that handles startup. This will copy
        ; the .data section initializers from flash to SRAM and zero fill the
        ; .bss section.
        ; NOTE: The __main function clears the C stack as well
        LDR     r0,=__main
        BX      r0

        ; __main calls the main() function, which should not return,
        ; but just in case jump to assert_failed() if main returns.
        LDR     r0,=str_EXIT
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_EXIT
        DCB     "EXIT"
        ALIGN
        ENDP

;******************************************************************************
NMI_Handler     PROC
        EXPORT  NMI_Handler     [WEAK]
        IMPORT  assert_failed
        LDR     r0,=str_NMI
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_NMI
        DCB     "NMI"
        ALIGN
        ENDP

;******************************************************************************
HardFault_Handler PROC
        EXPORT  HardFault_Handler [WEAK]
        IMPORT  assert_failed
        LDR     r0,=str_HardFault
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_HardFault
        DCB     "HardFault"
        ALIGN
        ENDP

;******************************************************************************
MemManage_Handler PROC
        EXPORT  MemManage_Handler     [WEAK]
        IMPORT  assert_failed
        LDR     r0,=str_MemManage
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_MemManage
        DCB     "MemManage"
        ALIGN
        ENDP

;******************************************************************************
BusFault_Handler PROC
        EXPORT  BusFault_Handler     [WEAK]
        IMPORT  assert_failed
        LDR     r0,=str_BusFault
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_BusFault
        DCB     "BusFault"
        ALIGN
        ENDP

;******************************************************************************
UsageFault_Handler PROC
        EXPORT  UsageFault_Handler   [WEAK]
        IMPORT  assert_failed
        LDR     r0,=str_UsageFault
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_UsageFault
        DCB     "UsageFault"
        ALIGN
        ENDP


;******************************************************************************
;
; Weak non-fault handlers...
;

;******************************************************************************
SVC_Handler PROC
        EXPORT  SVC_Handler   [WEAK]
        IMPORT  assert_failed
        LDR     r0,=str_SVC
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_SVC
        DCB     "SVC"
        ALIGN
        ENDP

;******************************************************************************
DebugMon_Handler PROC
        EXPORT  DebugMon_Handler     [WEAK]
        IMPORT  assert_failed
        LDR     r0,=str_DebugMon
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_DebugMon
        DCB     "DebugMon"
        ALIGN
        ENDP

;******************************************************************************
PendSV_Handler PROC
        EXPORT  PendSV_Handler       [WEAK]
        IMPORT  assert_failed
        LDR     r0,=str_PendSV
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_PendSV
        DCB     "PendSV"
        ALIGN
        ENDP

;******************************************************************************
SysTick_Handler PROC
        EXPORT  SysTick_Handler     [WEAK]
        IMPORT  assert_failed
        LDR     r0,=str_SysTick
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_SysTick
        DCB     "SysTick"
        ALIGN
        ENDP

;******************************************************************************
Default_Handler PROC
        EXPORT  EMU_IRQHandler              [WEAK]
        EXPORT  WDOG0_IRQHandler            [WEAK]
        EXPORT  LDMA_IRQHandler             [WEAK]
        EXPORT  GPIO_EVEN_IRQHandler        [WEAK]
        EXPORT  TIMER0_IRQHandler           [WEAK]
        EXPORT  USART0_RX_IRQHandler        [WEAK]
        EXPORT  USART0_TX_IRQHandler        [WEAK]
        EXPORT  ACMP0_IRQHandler            [WEAK]
        EXPORT  ADC0_IRQHandler             [WEAK]
        EXPORT  IDAC0_IRQHandler            [WEAK]
        EXPORT  I2C0_IRQHandler             [WEAK]
        EXPORT  GPIO_ODD_IRQHandler         [WEAK]
        EXPORT  TIMER1_IRQHandler           [WEAK]
        EXPORT  USART1_RX_IRQHandler        [WEAK]
        EXPORT  USART1_TX_IRQHandler        [WEAK]
        EXPORT  LEUART0_IRQHandler          [WEAK]
        EXPORT  PCNT0_IRQHandler            [WEAK]
        EXPORT  CMU_IRQHandler              [WEAK]
        EXPORT  MSC_IRQHandler              [WEAK]
        EXPORT  CRYPTO_IRQHandler           [WEAK]
        EXPORT  LETIMER0_IRQHandler         [WEAK]
        EXPORT  RTCC_IRQHandler             [WEAK]
        EXPORT  CRYOTIMER_IRQHandler        [WEAK]
        EXPORT  FPUEH_IRQHandler            [WEAK]

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

        IMPORT  assert_failed
        LDR     r0,=str_Undefined
        MOVS    r1,#1
        LDR     r2,=__initial_sp  ; re-set the SP in case of stack overflow
        MOV     sp,r2
        LDR     r2,=assert_failed
        BX      r2
str_Undefined
        DCB     "Undefined"
        ALIGN
        ENDP

        ALIGN               ; make sure the end of this section is aligned

;******************************************************************************
; The function expected of the C library startup code for defining the stack
; and heap memory locations.  For the C library version of the startup code,
; provide this function so that the C library initialization code can find out
; the location of the stack and heap.
;
    IF :DEF: __MICROLIB
        EXPORT  __initial_sp
        EXPORT  __stack_limit
        EXPORT  __heap_base
        EXPORT  __heap_limit
    ELSE
        IMPORT  __use_two_region_memory
        EXPORT  __user_initial_stackheap

__user_initial_stackheap PROC
        LDR     R0, =__heap_base
        LDR     R1, =__stack_limit
        LDR     R2, =__heap_limit
        LDR     R3, =__stack_base
        BX      LR
        ENDP
    ENDIF
        ALIGN               ; make sure the end of this section is aligned

    END                     ; end of module

