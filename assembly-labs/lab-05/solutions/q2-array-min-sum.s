@ Lab 5 - Q2: ray2 = {2, 4, 1, 3, 0}. Find the minimum and the sum; store the
@ minimum in array_min (and the sum in array_sum).
@ Expected: array_min = 0, array_sum = 10
@ (No answer for Lab 5 was in the archive - this solution is new.)
.global _start
.data
ray2:      .word 2, 4, 1, 3, 0
array_min: .word 0
array_sum: .word 0
.text
_start:
    ldr r0, =ray2
    mov r11, #5             @ element counter
    ldr r10, [r0]           @ min = first element
    mov r4, #0              @ sum = 0
loop1:
    ldr r3, [r0]
    cmp r3, r10
    bge adding              @ element >= min: keep the current min
    mov r10, r3             @ element < min: new minimum
adding:
    add r4, r4, r3          @ sum += element
    add r0, r0, #4
    subs r11, r11, #1
    bne loop1

    ldr r1, =array_min
    str r10, [r1]           @ array_min = 0
    ldr r2, =array_sum
    str r4, [r2]            @ array_sum = 10
end:
    b end
