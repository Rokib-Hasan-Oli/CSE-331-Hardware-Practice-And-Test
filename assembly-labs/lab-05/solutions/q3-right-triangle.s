@ Lab 5 - Q3: sides 3, 3, 5 are in r0, r1, r2. Show whether it is a right
@ triangle using the LEDs and the seven-segment display:
@     right triangle      -> HEX0 "A", LEDs 101
@     not right triangle  -> HEX0 "b", LEDs 010
@ Expected for 3, 3, 5: 3^2 + 3^2 = 18, 5^2 = 25 -> NOT right -> "b", 010
@ (This was submitted as "Class Task Q1" in the Lab 6 answer sheet.)
@ Fixes vs. the submitted answer:
@   * it tested the triangle inequality (a + b > c) - that only says the sides
@     form *a* triangle. A right triangle needs Pythagoras: a^2 + b^2 = c^2.
@   * "b RT" after the last compare was unconditional, so every input that
@     passed the first two tests was reported as a right triangle.
@ Note: an upper-case B looks exactly like 8 on a 7-segment display, so the
@       usual lower-case "b" (0x7C) is shown.
.global _start
.equ LEDR, 0xFF200000
.equ HEX,  0xFF200020
_start:
    mov r0, #3              @ side a
    mov r1, #3              @ side b
    mov r2, #5              @ side c
    ldr r3, =LEDR
    ldr r5, =HEX

    mul r7, r0, r0          @ a^2
    mul r8, r1, r1          @ b^2
    mul r9, r2, r2          @ c^2

    add r10, r7, r8         @ a^2 + b^2 == c^2 ?
    cmp r10, r9
    beq RT
    add r10, r7, r9         @ a^2 + c^2 == b^2 ?
    cmp r10, r8
    beq RT
    add r10, r8, r9         @ b^2 + c^2 == a^2 ?
    cmp r10, r7
    beq RT
NRT:
    mov r4, #0x2            @ LEDs 010
    str r4, [r3]
    mov r6, #0x7C           @ "b"
    str r6, [r5]
    b end
RT:
    mov r4, #0x5            @ LEDs 101
    str r4, [r3]
    mov r6, #0x77           @ "A"
    str r6, [r5]
end:
    b end
