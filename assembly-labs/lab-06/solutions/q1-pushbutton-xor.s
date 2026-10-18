@ Lab 6 - Q1: take two numbers one after the other from the push buttons, XOR
@ them and show the result on the LEDs.
@ How to use in CPUlator: run, press/hold the KEY buttons for the first number
@ and release them, then do the same for the second number.
@ Fix: the submitted answer read the KEY register twice back-to-back. Both reads
@      happen within nanoseconds, so they always return the same value and the
@      XOR was always 0. Each number is now captured as its own press-and-release.
.global _start
.equ LEDR, 0xFF200000
.equ KEY,  0xFF200050
_start:
    ldr r0, =LEDR
    ldr r1, =KEY

@ ---- first number ----
wait1:
    ldr r2, [r1]            @ wait until at least one button is pressed
    cmp r2, #0
    beq wait1
hold1:
    ldr r5, [r1]            @ collect every button held down ...
    orr r2, r2, r5
    cmp r5, #0
    bne hold1               @ ... until all buttons are released

@ ---- second number ----
wait2:
    ldr r3, [r1]
    cmp r3, #0
    beq wait2
hold2:
    ldr r5, [r1]
    orr r3, r3, r5
    cmp r5, #0
    bne hold2

    eor r4, r2, r3          @ XOR of the two numbers
    str r4, [r0]            @ show on the LEDs
end:
    b end
