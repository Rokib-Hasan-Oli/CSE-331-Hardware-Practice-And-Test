@ Lab 3 - Q3: divide 50 by 2 using ARM instructions.
@ ARMv7 has no divide instruction; a logical shift right by 1 divides an
@ unsigned value by 2. Expected: r1 = 25. (Submitted answer was correct.)
.global _start
_start:
    mov r1, #50
    lsr r1, r1, #1      @ r1 = 50 / 2 = 25
    swi 0
