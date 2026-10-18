@ Lab 3 - Q1: load 5 (hex) and 10 (hex) into r0 and r1 with immediate
@ instructions, XOR them and store the result in XOR_ans.
@ Expected: XOR_ans = 0x05 ^ 0x10 = 0x15 (21). (Submitted answer was correct.)
.global _start
.data
XOR_ans: .word 0
.text
_start:
    mov r0, #0x5
    mov r1, #0x10
    eor r2, r1, r0      @ r2 = 0x15
    ldr r3, =XOR_ans
    str r2, [r3]
    swi 0
