# ARM Assembly Labs (CPUlator)

Labs 2–6 of CSE331L were written in **ARMv7 assembly** and run on the **[CPUlator ARMv7 DE1-SoC simulator](https://cpulator.01xz.net/?sys=arm-de1soc)**.

Each lab folder contains:

- `Lab-N-Task.png`: the task sheet
- `Lab-N-Answer-Submitted.pdf`: the answer sheet as originally submitted (kept unchanged)
- `solutions/*.s`: cleaned-up, **corrected** programs you can paste straight into CPUlator

| Lab | Topics | Solutions |
|-----|--------|-----------|
| [Lab 2](lab-02) | `MOV`, `ADD`, `SUB(S)`, CPSR flags, `.data` variables, `LDR`/`STR`, `MUL` | [solutions](lab-02/solutions) |
| [Lab 3](lab-03) | `EOR`, `LSR`, `ROR`, divide by shifting, arrays, unconditional branch | [solutions](lab-03/solutions) |
| [Lab 4](lab-04) | `CMP`, conditional branches (`BGT`, `BMI`, …), loops, functions (`BL` / `BX LR`) | [solutions](lab-04/solutions) |
| [Lab 5](lab-05) | if/else, min/sum over an array, LEDs + seven-segment output | [solutions](lab-05/solutions) |
| [Lab 6](lab-06) | Memory-mapped I/O: switches, push buttons, LEDs, seven-segment display | [solutions](lab-06/solutions) |

## How to run a solution

1. Open <https://cpulator.01xz.net/?sys=arm-de1soc>.
2. Paste the `.s` file into the editor and press **Compile and Load** (F5).
3. Press **Continue** (F3), or **Step Over** to watch registers change.
4. For Lab 6, use the **Switches** and **Push buttons** panels on the right. To enter a value, hold the buttons and then release them.

DE1-SoC addresses used: LEDs `0xFF200000`, HEX3–0 `0xFF200020`, switches `0xFF200040`, push buttons `0xFF200050`.

## Corrections

Each submitted answer was checked against its task: by running it in an instruction-level simulator of these programs, and with spot checks in the real CPUlator. These are the problems found and fixed. Each `.s` file also explains its fix in the header comment.

| Lab / Q | Problem in the submitted answer | Fix |
|---------|---------------------------------|-----|
| 2 Q1 | Used a register add (`add r2,r1,r0`); the task asks for the **immediate** add only | `add r0, r0, #0x11` |
| 2 Q2 | Explanation said the result is **−66**, but `0x11` is 17, so 17 − 77 = **−60** (`0xFFFFFFC4`) | Corrected value + full N/Z/C/V explanation (C = 0 means a borrow on ARM) |
| 3 Q4 | `b loop1` never ended, so it kept reading memory past the end of `arr1` forever and `swi 0` was unreachable | Counter stops after 5 elements; still loops with an unconditional `b` |
| 4 Q1 | `cmp r10,r3 / bgt update` replaced the max when it was *bigger* than the element (the logic of a minimum), so `array_min` stayed 0. The loop never ended and even read `array_sum` back as an element (sum doubled to 140) | Correct comparison, 6-element counter → `array_min` = 23, `array_sum` = 70 |
| 4 Q2 | No branch after `mov r1,#0`, so it fell through to `mov r1,#1`, giving **r1 = 1 even for positive numbers** | Added `b end` |
| 4 Q3 | `8_diya_vaag` is a **syntax error**: GNU as symbols can't start with a digit (CPUlator: *"garbage following instruction"*). That is why it looked like CPUlator "doesn't support functions". The code after `bl` also just fell into the function body | Renamed to `_8_diye_vaag`; a real function that returns with `bx lr` → r8 = 2 |
| 5 Q1–Q2 | No answer was in the archive | New solutions added |
| 5 Q3 (submitted as Lab 6 *Class Task Q1*) | Tested the triangle inequality instead of Pythagoras, and its last branch was unconditional, so **3, 3, 5 was reported as a right triangle** | Checks a² + b² = c² for every ordering → 3, 3, 5 shows "b" / 010; 3, 4, 5 shows "A" / 101 |
| 6 Q1 | Read the push buttons twice back-to-back (same instant, same value), so the **XOR was always 0** | Each number is captured as its own press and release |
| 6 Q2 | **Multiplied** the two inputs; the task asks for the **sum** (e.g. 2 + 3 = 5 is odd, but it showed "EA" for even because 2 × 3 = 6) | `add` instead of `mul` |
| 6 Class Task Q3 | Read the switches twice back-to-back, so it always squared one number | A KEY0 press confirms each of the two numbers |
| Several | Ended with `end: .end`. `.end` only tells the assembler the file is over; the CPU kept executing whatever memory came next | Programs stop in a halt loop: `end: b end` |

Lab 2 Q3–Q6 and Lab 3 Q1–Q3 were already correct; they are included in cleaned-up, commented form.
