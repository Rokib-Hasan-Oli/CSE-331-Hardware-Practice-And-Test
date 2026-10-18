@ Lab 3 - Q4: create arr1 = {11, 0x12, 0x13, 14, 0x15} and read the numbers
@ using an unconditional branch and the r8 / r9 registers.
@ Fix: the submitted loop (b loop1) never stopped, so after the 5th element it
@      kept reading whatever memory came next, forever, and "swi 0" was never
@      reached. A counter now ends the loop after 5 reads; the loop itself still
@      repeats with the unconditional branch "b loop1".
.global _start
.data
arr1: .word 11, 0x12, 0x13, 14, 0x15
.text
_start:
    ldr r8, =arr1       @ r8 = address of the current element
    mov r2, #5          @ number of elements
loop1:
    cmp r2, #0
    beq done            @ all 5 numbers read
    ldr r9, [r8]        @ r9 = current number: 11, 18, 19, 14, 21
    add r8, r8, #4      @ next word (each .word is 4 bytes)
    sub r2, r2, #1
    b loop1             @ unconditional branch back to the top
done:
    swi 0
