// clang-format off
#include <context.h>

/*
 * r0, r1 are scratch
 */

.text
.global switch_user
.type switch_user STT_FUNC
switch_user:
    LDR R0, #kernel_reg_addr
    STMIA R0, {R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12}
    STR R14, [R0, #52] // save lr as pc
    STR R13, [R0, #56] // save sp
    STR R14, [R0, #60] // save lr
    MRS R1, CPSR
    STR R1, [R0, #64]
    // load user regs
    LDR R0, #user_reg_addr_addr
    LDR R0, [R0]
    LDR R1, [R0, #64]
    MSR SPSR_cxsf, R1
    ADD R1, R0, #56
    LDMIA R1, {R13, R14}^
    LDMIA R0, {R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R15}^

.text
.global return_irq
.type return_irq STT_FUNC
return_irq:
    SUB R14, R14,#3 // irq have odd pc
.text
.global return_swi
.type return_swi STT_FUNC
return_swi:
    STMDB SP, {R0, R1}
    LDR R0, #user_reg_addr_addr
    LDR R0, [R0]
    LDR R1, [SP, #-8] // save R0
    STR R1, [R0]
    LDR R1, [SP, #-4] // save R1
    STR R1, [R0, #4]
    ADD R1, R0, #8
    STMIA R1, {R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12}^
    STR R14, [R0, #52] // exception lr is user pc
    ADD R1, R0, #56
    STMIA R1, {R13, R14}^
    MRS R1, SPSR
    STR R1, [R0, #64]
    LDR R0, #kernel_reg_addr
    LDR R1, [R0, #64]
    MSR CPSR_cxsf, R1
    LDR R13, [R0, #56] // load sp
    LDR R14, [R0, #60] // load lr
    LDMIA R0, {R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R15}

.text
.global kernel_reg
kernel_reg_addr:
.int kernel_reg

.text
.global user_reg
user_reg_addr_addr:
.int user_reg
