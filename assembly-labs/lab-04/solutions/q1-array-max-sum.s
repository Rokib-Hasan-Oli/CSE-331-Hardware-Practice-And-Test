@ Lab 4 - Q1: SONGKHA = {1, 0x13, 5, 4, 0x12, 0x17}. Find the MAXIMUM and the
@ sum; store them in array_min and array_sum (variable names as given in the task).
@ Expected: array_min = 0x17 (23), array_sum = 0x46 (70)
@ Fixes vs. the submitted answer:
@   * "cmp r10,r3 / bgt update" replaced the max when it was BIGGER than the
@     element (that tracks a minimum) - now the element must be bigger.
@   * the loop had no counter, so it ran forever past the end of the array
@     (it even read array_sum back in as an element, doubling the sum).
@   * the program now stops in a halt loop instead of running off the end.
.global _start
.data
SONGKHA:   .word 1, 0x13, 5, 4, 0x12, 0x17
array_min: .word 0          @ holds the maximum (name given by the task)
array_sum: .word 0
.text
_start:
    ldr r0, =SONGKHA
    mov r11, #6             @ element counter
    ldr r10, [r0]           @ max = first element
    mov r4, #0              @ sum = 0
loop1:
    ldr r3, [r0]            @ r3 = current element
    cmp r3, r10
    ble adding              @ element <= max: keep the current max
    mov r10, r3             @ element > max: new maximum
adding:
    add r4, r4, r3          @ sum += element
    add r0, r0, #4          @ next element
    subs r11, r11, #1
    bne loop1               @ repeat until all 6 elements are done

    ldr r1, =array_min
    str r10, [r1]           @ array_min = 23 (0x17)
    ldr r2, =array_sum
    str r4, [r2]            @ array_sum = 70 (0x46)
end:
    b end                   @ halt here (".end" alone does not stop the CPU)
