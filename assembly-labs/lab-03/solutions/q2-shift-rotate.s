@ Lab 3 - Q2: assign 19 to r9, right shift 2 times, then rotate right 2 times.
@ (Submitted answer was correct.)
.global _start
_start:
    mov r9, #19         @ 19 = 0000 ... 0001 0011
    lsr r9, r9, #2      @ LSR #2: bits move 2 places right, the two low bits (11)
                        @ fall off and zeros enter from the left
                        @ -> 0000 ... 0000 0100 = 4   (same as 19 / 4, truncated)
    ror r9, r9, #2      @ ROR #2: bits move 2 places right, but bits leaving on the
                        @ right re-enter on the left. The low bits of 4 are 00, so
                        @ only zeros wrap around -> 0000 ... 0000 0001 = 1
    swi 0
