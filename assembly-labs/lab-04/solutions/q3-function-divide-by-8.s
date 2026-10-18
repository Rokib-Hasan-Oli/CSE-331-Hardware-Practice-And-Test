@ Lab 4 - Q3: function "8_diye_vaag" (divide by 8) that uses a shift. Set a GP
@ register to 3, divide it with the function, then: zero -> r8 = 2, else r8 = 3.
@ Expected: 3 / 8 = 0, so r8 = 2.
@ Fixes vs. the submitted answer:
@   * GNU as (used by CPUlator) does not allow a symbol to start with a digit, so
@     "8_diya_vaag" is a syntax error - that is why it looked like "CPUlator
@     doesn't support functions". The function is named _8_diye_vaag instead.
@   * the code after "bl" fell straight into the function body; a real function
@     now lives on its own and returns to the caller with "bx lr".
.global _start
_start:
    mov r1, #3              @ value to divide
    bl _8_diye_vaag         @ call: r1 = r1 / 8, return address saved in lr
    cmp r1, #0
    beq is_zero
    mov r8, #3              @ result is not zero
    b end
is_zero:
    mov r8, #2              @ result is zero (3 / 8 = 0)
end:
    b end

@ ---- function: divide r1 by 8 (8 = 2^3, so shift right by 3) ----
_8_diye_vaag:
    lsr r1, r1, #3
    bx lr                   @ return to the instruction after "bl"
