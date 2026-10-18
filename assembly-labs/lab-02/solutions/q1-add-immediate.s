@ Lab 2 - Q1: Add 7 (dec) and 11 (hex) using the immediate ADD instruction only.
@ Expected: r0 = 7 + 0x11 = 24 (0x18)
@ Fix: the submitted answer used a register add (add r2,r1,r0); the task asks
@      for the immediate form, where the second operand is a #constant.
.global _start
_start:
    mov r0, #7          @ r0 = 7
    add r0, r0, #0x11   @ immediate add: r0 = 7 + 17 = 24 (0x18)
    swi 0
