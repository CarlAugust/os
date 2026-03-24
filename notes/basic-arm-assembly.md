CHEAT SHEET GENERATED WITH CLAUDE... I AM LAZY MAN

Name	Alias	Purpose
r0–r3	a1–a4	Args / return / scratch (caller-saved)
r4–r11	v1–v8	General purpose (callee-saved)
r12	ip	Intra-procedure scratch
r13	sp	Stack pointer important
r14	lr	Link register — return address
r15	pc	Program counter careful

Data movement
Instruction	Example
MOV	MOV r0, #5
MVN	MVN r0, #0 bitwise NOT
LDR	LDR r0, [r1]
STR	STR r0, [r1]
LDR =	LDR r0, =0x9000 big constants
PUSH	PUSH {r4, lr}
POP	POP {r4, pc}
Arithmetic

Instruction	Example
ADD	ADD r2, r0, r1
SUB	SUB r2, r0, #1
MUL	MUL r2, r0, r1
RSB	RSB r0, r0, #0 negate
CMP	CMP r0, #10
ADDS	ADDS r0, r0, r1 sets flags

Bitwise
Instruction	Example
AND	AND r0, r0, #0xFF
ORR	ORR r0, r0, #(1<<3)
EOR	EOR r0, r0, #1
BIC	BIC r0, r0, #0xF bit clear
LSL	LSL r0, r0, #2
LSR	LSR r0, r0, #1
ASR	ASR r0, r0, #1 sign-extend

Branches
Instruction	Meaning
B	Branch always
BL	Branch + save lr
BX lr	Return from function
BEQ	Branch if equal
BNE	Branch if not equal
BLT	Branch if less than
BGT	Branch if greater than
BGE / BLE	Greater/less or equal

Addressing modes
Syntax	Meaning
[r1]	Address in r1
[r1, #4]	r1 + 4, no writeback
[r1, #4]!	r1 + 4, then r1 += 4 (pre-index)
[r1], #4	r1, then r1 += 4 (post-index)
[r1, r2, LSL #2]	r1 + (r2 << 2)

Condition code suffixes (append to any instruction)
Suffix	Condition	Suffix	Condition
EQ	Z=1 (equal)	NE	Z=0 (not equal)
GT	Greater (signed)	LT	Less (signed)
GE	Greater or equal	LE	Less or equal
MI	Negative	PL	Positive
CS	Carry set	VS	Overflow set

Common gotchas
Gotcha	What to do instead
MOV for big values 	MOV only fits 8-bit immediates. Use LDR r0, =0x9000 for anything larger.
No hang loop 	CPU runs off into garbage bytes. Always end with hang: b hang.
Forgetting flags 	CMP sets flags but most instructions don't. Add S suffix (ADDS, SUBS) to set flags from arithmetic.
Writing to pc 	MOV pc, r0 jumps there. Handy for returns, dangerous by accident.
Peripheral base 	Pi Zero (BCM2835) uses 0x20000000, not 0x3F000000 (that's Pi 2/3).
Callee-saved regs 	r4–r11 must be preserved across function calls. Always PUSH {r4, lr} at entry, POP {r4, pc} at exit.

Minimal function template
my_func: @ args in r0, r1 — result in r0
  PUSH {r4, lr} @ save regs + return address
  MOV r4, r0 @ move arg to callee-saved reg
  ADD r0, r4, r1 @ do work
  POP {r4, pc} @ restore and return

GDB quick reference
target remote :1234 @ connect to QEMU
layout regs @ show registers live
break *0x8000 @ break at your entry point
continue @ run until breakpoint
stepi @ execute one instruction
info registers @ dump all registers
x/4xw 0x9000 @ inspect 4 words at address
disas _start @ disassemble from label
