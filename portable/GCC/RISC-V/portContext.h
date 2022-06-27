/*
 * FreeRTOS Kernel <DEVELOPMENT BRANCH>
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#ifndef PORTCONTEXT_H
#define PORTCONTEXT_H

#if __riscv_xlen == 64
    #define portWORD_SIZE 8
    #define store_x sd
    #define load_x ld
#elif __riscv_xlen == 32
    #define store_x sw
    #define load_x lw
    #define portWORD_SIZE 4
#else
    #error Assembler did not define __riscv_xlen
#endif

#include "freertos_risc_v_chip_specific_extensions.h"

/* Only the standard core registers are stored by default.  Any additional
 * registers must be saved by the portasmSAVE_ADDITIONAL_REGISTERS and
 * portasmRESTORE_ADDITIONAL_REGISTERS macros - which can be defined in a chip
 * specific version of freertos_risc_v_chip_specific_extensions.h.  See the
 * notes at the top of portASM.S file. */
//除了x0 这个永远是0的寄存器 剩下的还有31个寄存器.
#define portCONTEXT_SIZE ( 31 * portWORD_SIZE )
/*-----------------------------------------------------------*/

.extern pxCurrentTCB
.extern xISRStackTop
.extern xCriticalNesting
.extern pxCriticalNesting
/*-----------------------------------------------------------*/
/*
当发生中断的时候,首先会调用这个函数.
该函数就是将线程上下文保存全部保存到线程栈.
并更新tcb中的pxTopOfStack指针.
*/
.macro portcontextSAVE_CONTEXT_INTERNAL
    addi sp, sp, -portCONTEXT_SIZE   //将sp减小31个寄存器的大小.
    store_x x1, 1 * portWORD_SIZE( sp ) //0(sp)==>offset(addr) 类似与数组的使用. 将x1存入栈中.
    /*
    x2 x3 x4分别是stack pointer global pointer thread pointer.
    不存x2,x3,x4.
    */
    store_x x5, 2 * portWORD_SIZE( sp )
    store_x x6, 3 * portWORD_SIZE( sp )
    store_x x7, 4 * portWORD_SIZE( sp )
    store_x x8, 5 * portWORD_SIZE( sp )
    store_x x9, 6 * portWORD_SIZE( sp )
    store_x x10, 7 * portWORD_SIZE( sp )
    store_x x11, 8 * portWORD_SIZE( sp )
    store_x x12, 9 * portWORD_SIZE( sp )
    store_x x13, 10 * portWORD_SIZE( sp )
    store_x x14, 11 * portWORD_SIZE( sp )
    store_x x15, 12 * portWORD_SIZE( sp )
    store_x x16, 13 * portWORD_SIZE( sp )
    store_x x17, 14 * portWORD_SIZE( sp )
    store_x x18, 15 * portWORD_SIZE( sp )
    store_x x19, 16 * portWORD_SIZE( sp )
    store_x x20, 17 * portWORD_SIZE( sp )
    store_x x21, 18 * portWORD_SIZE( sp )
    store_x x22, 19 * portWORD_SIZE( sp )
    store_x x23, 20 * portWORD_SIZE( sp )
    store_x x24, 21 * portWORD_SIZE( sp )
    store_x x25, 22 * portWORD_SIZE( sp )
    store_x x26, 23 * portWORD_SIZE( sp )
    store_x x27, 24 * portWORD_SIZE( sp )
    store_x x28, 25 * portWORD_SIZE( sp )
    store_x x29, 26 * portWORD_SIZE( sp )
    store_x x30, 27 * portWORD_SIZE( sp )
    store_x x31, 28 * portWORD_SIZE( sp )
    //存储当前中断嵌套的层数.
    load_x  t0, xCriticalNesting         /* Load the value of xCriticalNesting into t0. */
    store_x t0, 29 * portWORD_SIZE( sp ) /* Store the critical nesting value to the stack. */
    //存入mstatus.
    csrr t0, mstatus                     /* Required for MPIE bit. */
    store_x t0, 30 * portWORD_SIZE( sp )
    //存入自定义的寄存器.
    portasmSAVE_ADDITIONAL_REGISTERS     /* Defined in freertos_risc_v_chip_specific_extensions.h to save any registers unique to the RISC-V implementation. */
    /*
    将sp值更新到,pxCurrentTCB->pxTopOfStack
    volatile StackType_t * pxTopOfStack; 
    < Points to the location of the last item placed on the tasks stack.
    */
    load_x  t0, pxCurrentTCB             /* Load pxCurrentTCB. */
    store_x  sp, 0( t0 )                 /* Write sp to first TCB member. */

    .endm
/*-----------------------------------------------------------*/

.macro portcontextSAVE_EXCEPTION_CONTEXT
    portcontextSAVE_CONTEXT_INTERNAL
    csrr a0, mcause
    csrr a1, mepc
    addi a1, a1, 4                      /* Synchronous so update exception return address to the instruction after the instruction that generated the exception. */
    store_x a1, 0( sp )                 /* Save updated exception return address. */
    load_x sp, xISRStackTop             /* Switch to ISR stack. */
    .endm
/*-----------------------------------------------------------*/

.macro portcontextSAVE_INTERRUPT_CONTEXT
    portcontextSAVE_CONTEXT_INTERNAL
    csrr a0, mcause
    csrr a1, mepc
    store_x a1, 0( sp )                 /* Asynchronous interrupt so save unmodified exception return address. */
    load_x sp, xISRStackTop             /* Switch to ISR stack. */
    .endm
/*-----------------------------------------------------------*/
//将栈指针和上下文恢复到中断发生前的值.
.macro portcontextRESTORE_CONTEXT
    //拿到当前进程上下文的指针.
    load_x  t1, pxCurrentTCB                /* Load pxCurrentTCB. */
    //赋值给sp寄存器.
    load_x  sp, 0( t1 )                 /* Read sp from first TCB member. */

    /* Load mepc with the address of the instruction in the task to run next. */
    //进入中断的时候将mepc存到了栈顶.
    load_x t0, 0( sp )
    //重新更新mepc.
    csrw mepc, t0

    /* Defined in freertos_risc_v_chip_specific_extensions.h to restore any registers unique to the RISC-V implementation. */
    portasmRESTORE_ADDITIONAL_REGISTERS

    /* Load mstatus with the interrupt enable bits used by the task. */
    load_x  t0, 30 * portWORD_SIZE( sp )
    csrw mstatus, t0                        /* Required for MPIE bit. */

    load_x  t0, 29 * portWORD_SIZE( sp )    /* Obtain xCriticalNesting value for this task from task's stack. */
    load_x  t1, pxCriticalNesting           /* Load the address of xCriticalNesting into t1. */
    store_x t0, 0( t1 )                     /* Restore the critical nesting value for this task. */

    load_x  x1, 1 * portWORD_SIZE( sp )
    load_x  x5, 2 * portWORD_SIZE( sp )
    load_x  x6, 3 * portWORD_SIZE( sp )
    load_x  x7, 4 * portWORD_SIZE( sp )
    load_x  x8, 5 * portWORD_SIZE( sp )
    load_x  x9, 6 * portWORD_SIZE( sp )
    load_x  x10, 7 * portWORD_SIZE( sp )
    load_x  x11, 8 * portWORD_SIZE( sp )
    load_x  x12, 9 * portWORD_SIZE( sp )
    load_x  x13, 10 * portWORD_SIZE( sp )
    load_x  x14, 11 * portWORD_SIZE( sp )
    load_x  x15, 12 * portWORD_SIZE( sp )
    load_x  x16, 13 * portWORD_SIZE( sp )
    load_x  x17, 14 * portWORD_SIZE( sp )
    load_x  x18, 15 * portWORD_SIZE( sp )
    load_x  x19, 16 * portWORD_SIZE( sp )
    load_x  x20, 17 * portWORD_SIZE( sp )
    load_x  x21, 18 * portWORD_SIZE( sp )
    load_x  x22, 19 * portWORD_SIZE( sp )
    load_x  x23, 20 * portWORD_SIZE( sp )
    load_x  x24, 21 * portWORD_SIZE( sp )
    load_x  x25, 22 * portWORD_SIZE( sp )
    load_x  x26, 23 * portWORD_SIZE( sp )
    load_x  x27, 24 * portWORD_SIZE( sp )
    load_x  x28, 25 * portWORD_SIZE( sp )
    load_x  x29, 26 * portWORD_SIZE( sp )
    load_x  x30, 27 * portWORD_SIZE( sp )
    load_x  x31, 28 * portWORD_SIZE( sp )
    addi sp, sp, portCONTEXT_SIZE
    /*
    At this point control is handed over to software where the interrupt processing begins. At the
    end of the interrupt handler, the mret instruction will do the following.
    • Restore mepc to pc
    • Restore mstatus.mpp to Priv
    • Restore mstatus.mpie to mie
    */
    mret
    .endm
/*-----------------------------------------------------------*/

#endif /* PORTCONTEXT_H */
