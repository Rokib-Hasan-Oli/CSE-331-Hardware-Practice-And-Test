@ Lab 2 - Q2: Subtract 11 (hex) and 77 (dec). Explain the output using CPSR.
@ Fix: 0x11 is 17 in decimal (not 11), so 0x11 - 77 = 17 - 77 = -60, not -66.
.global _start
_start:
    mov r0, #0x11       @ r0 = 0x11 = 17
    mov r1, #77         @ r1 = 77 (0x4D)

    subs r3, r1, r0     @ 77 - 17 = 60 (0x3C)        -> N=0 Z=0 C=1 V=0
                        @ C=1 on ARM means "no borrow" (77 >= 17)

    subs r2, r0, r1     @ 17 - 77 = -60 = 0xFFFFFFC4 (two's complement)
                        @ CPSR after this SUBS:
                        @   N = 1  result is negative (bit 31 is set)
                        @   Z = 0  result is not zero
                        @   C = 0  a borrow happened (17 < 77)
                        @   V = 0  no signed overflow
    swi 0
