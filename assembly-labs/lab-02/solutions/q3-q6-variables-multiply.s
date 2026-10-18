@ Lab 2 - Q3..Q6: define Sec_8 = 8 (hex) and CSE331 = 5 (dec), multiply them
@ using general-purpose registers and store the product in Lab2.
@ Expected: Lab2 = 8 * 5 = 40 (0x28). (The submitted answer was already correct.)
.global _start
.data
Sec_8:  .word 0x8
CSE331: .word 5
Lab2:   .word 0
.text
_start:
    ldr r0, =Sec_8
    ldr r5, [r0]        @ r5 = 8
    ldr r1, =CSE331
    ldr r6, [r1]        @ r6 = 5
    mul r8, r5, r6      @ r8 = 40
    ldr r3, =Lab2
    str r8, [r3]        @ Lab2 = 40 (0x28)
    swi 0
