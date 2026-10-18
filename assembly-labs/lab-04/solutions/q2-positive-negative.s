@ Lab 4 - Q2: check whether r6 is positive or negative using BMI.
@ Output in r1: 0 = positive, 1 = negative.
@ Fix: the submitted answer had no branch after "mov r1,#0", so execution fell
@      through into "negative:" and r1 always ended up 1 - even for r6 = 5.
.global _start
_start:
    mov r6, #-5             @ value to test - try #5 as well
    cmp r6, #0
    bmi negative            @ N flag set -> r6 < 0
    mov r1, #0              @ positive (or zero)
    b end                   @ skip the negative case
negative:
    mov r1, #1              @ negative
end:
    b end
