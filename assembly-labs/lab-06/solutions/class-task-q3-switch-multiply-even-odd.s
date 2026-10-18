@ Lab 6 - Class Task Q3: take two numbers from the switches, multiply them and
@ show whether the product is even or odd:
@     even -> "A", LEDs 101   odd -> "b", LEDs 010
@ How to use in CPUlator: set the switches for the first number and press and
@ release KEY0; set the second number and press and release KEY0 again.
@ (Output codes kept as submitted; the task sheet for this class task is not
@  in the archive.)
@ Fixes vs. the submitted answer:
@   * the switches were read twice back-to-back, which always returns the same
@     value - so the program only ever squared one number. A KEY0 press now
@     confirms each number before it is read.
@   * the program now stops in a halt loop instead of running off the end.
.global _start
.equ LEDR, 0xFF200000
.equ HEX,  0xFF200020
.equ SW,   0xFF200040
.equ KEY,  0xFF200050
_start:
    ldr r0, =LEDR
    ldr r1, =HEX
    ldr r8, =SW
    ldr r9, =KEY

press1:                     @ wait for KEY0 press ...
    ldr r5, [r9]
    cmp r5, #0
    beq press1
release1:                   @ ... and release
    ldr r5, [r9]
    cmp r5, #0
    bne release1
    ldr r11, [r8]           @ first number

press2:
    ldr r5, [r9]
    cmp r5, #0
    beq press2
release2:
    ldr r5, [r9]
    cmp r5, #0
    bne release2
    ldr r12, [r8]           @ second number

    mul r10, r11, r12       @ product
    and r6, r10, #1         @ lowest bit: 0 = even, 1 = odd
    cmp r6, #1
    beq odd
    mov r3, #0x5            @ LEDs 101
    str r3, [r0]
    mov r4, #0x77           @ "A"
    str r4, [r1]
    b end
odd:
    mov r3, #0x2            @ LEDs 010
    str r3, [r0]
    mov r4, #0x7C           @ "b"
    str r4, [r1]
end:
    b end
