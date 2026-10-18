@ Lab 6 - Class Task Q2: read a number from the switches and show whether it is
@ even or odd:  even -> "A", LEDs 101   odd -> "b", LEDs 010
@ (Output codes kept exactly as submitted; the task sheet for this class task
@  is not in the archive.)
@ Fix: the submitted program ended with "end: .end". ".end" only tells the
@      assembler the file is over - it is not an instruction, so the CPU kept
@      executing whatever memory came next. It now stops in a halt loop.
.global _start
.equ LEDR, 0xFF200000
.equ HEX,  0xFF200020
.equ SW,   0xFF200040
_start:
    ldr r0, =LEDR
    ldr r1, =HEX
    ldr r8, =SW
    ldr r5, [r8]            @ number from the switches
    and r6, r5, #1          @ lowest bit: 0 = even, 1 = odd
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
