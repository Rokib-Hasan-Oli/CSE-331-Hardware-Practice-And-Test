@ Lab 6 - Q2: take one input from the switches and one from the push buttons,
@ SUM them and check whether the result is even or odd:
@     even -> 7-segment "EA", LEDs 1010
@     odd  -> 7-segment "OH", LEDs 0101
@ How to use in CPUlator: set the switches, then press and release the KEY
@ buttons for the second number.
@ Fixes vs. the submitted answer:
@   * it multiplied the inputs (mul) - the task asks for their sum.
@   * the program now stops in a halt loop instead of running off the end.
@ Note: "mov r4, #0x7977" is fine in CPUlator - the value doesn't fit a normal
@       ARM immediate, so the assembler emits MOVW (ARMv7) for it automatically.
@ 7-segment bits: a=bit0 b=bit1 c=bit2 d=bit3 e=bit4 f=bit5 g=bit6;
@ HEX1 is bits 15..8, HEX0 is bits 7..0.
@     E = 0x79  A = 0x77  ->  "EA" = 0x7977
@     O = 0x3F  H = 0x76  ->  "OH" = 0x3F76
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

wait_key:                   @ input 2: push buttons (press, then release)
    ldr r12, [r9]
    cmp r12, #0
    beq wait_key
hold_key:
    ldr r5, [r9]
    orr r12, r12, r5
    cmp r5, #0
    bne hold_key

    ldr r11, [r8]           @ input 1: switches
    add r10, r11, r12       @ SUM of the two inputs

    and r6, r10, #1         @ lowest bit: 0 = even, 1 = odd
    cmp r6, #1
    beq odd
    mov r3, #0xA            @ LEDs 1010
    str r3, [r0]
    mov r4, #0x7977         @ "EA"
    str r4, [r1]
    b end
odd:
    mov r3, #0x5            @ LEDs 0101
    str r3, [r0]
    mov r4, #0x3F76         @ "OH"
    str r4, [r1]
end:
    b end
