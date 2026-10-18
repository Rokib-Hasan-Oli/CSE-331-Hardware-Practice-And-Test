@ Lab 5 - Q1: convert to ARM assembly
@     if (r0 != 5)  R3 = r0 * 5;
@     else          R3 = r0 + 10;
@ Expected: r0 = 4 -> r3 = 20;  r0 = 5 -> r3 = 15
@ (No answer for Lab 5 was in the archive - this solution is new.)
.global _start
_start:
    mov r0, #4              @ test value - try #5 for the else branch
    cmp r0, #5
    beq else_part           @ r0 == 5 -> else branch
    mov r2, #5              @ MUL cannot take an immediate, so load 5 first
    mul r3, r0, r2          @ R3 = r0 * 5
    b end
else_part:
    add r3, r0, #10         @ R3 = r0 + 10
end:
    b end
