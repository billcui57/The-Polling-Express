// clang-format off
#include <context.h>

/*
 * r9 stores the address of the register frame
 */

.text
.global switch_user
.type switch_user STT_FUNC
switch_user:
    STMIA R9, {R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12}
    STR R14, [R9, #52] // save lr as pc
    STR R13, [R9, #56] // save sp
    STR R14, [R9, #60] // save lr
    MRS R1, CPSR
    STR R1, [R9, #64]
    MOV R9, R0
    LDR R1, [R9, #64]
    MSR SPSR_cxsf, R1
    ADD R1, R9, #56
    LDMIA R1, {R13, R14}^
    LDMIA R9, {R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R15}^

.text
.global return_irq
.type return_irq STT_FUNC
return_irq:
    SUB R14, R14,#3 // irq have odd pc
.text
.global return_swi
.type return_swi STT_FUNC
return_swi:
    STMIA R9, {R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12}^
    STR R14, [R9, #52] // exception lr is user pc
    ADD R1, R9, #56
    STMIA R1, {R13, R14}^
    MRS R1, SPSR
    STR R1, [R9, #64]
    LDR R9, #kernel_reg_addr
    LDR R1, [R9, #64]
    MSR CPSR_cxsf, R1
    LDR R13, [R9, #56] // load sp
    LDR R14, [R9, #60] // load lr
    LDMIA R9, {R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R15}
.text
.global kernel_reg
kernel_reg_addr:
.int kernel_reg
