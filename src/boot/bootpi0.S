// AArch32 mode

// To keep this in the first portion of the binary.
.section ".text.boot"

// Make _start global.
.global _start

        .org 0x8000
// Entry point for the kernel.
// r15 -> should begin execution at 0x8000.
// r0 -> 0x00000000
// r1 -> 0x00000C42 - machine id
// r2 -> 0x00000100 - start of ATAGS
// preserve these registers as argument for kernel_main
_start:	
	b reset_handler      @ 0x8000 : Reset
    b undef_handler      @ 0x8004 : Undefined Instruction
    b swi_handler        /* 0x8008 : Software Interrupt (SVC) */
    b prefetch_handler   /* 0x800C : Prefetch Abort */
    b data_handler       /* 0x8010 : Data Abort */
    b reserved_handler   /* 0x8014 : Reserved slot */
    b irq_handler        /* 0x8018 : IRQ (Normal Interrupt) */
    b fiq_handler        /* 0x801C : FIQ (Fast Interrupt) */

reset_handler:

	// VBAR is at 0x8000
	ldr r3, =_start
	mcr p15, 0, r3, c12, c0, 0

	// Setup the stack.
	mov sp, #0x8000

	// Clear out bss.
	ldr r4, =__bss_start
	ldr r9, =__bss_end
	mov r5, #0
	mov r6, #0
	mov r7, #0
	mov r8, #0
	b       2f

1:
	// store multiple at r4.
	stmia r4!, {r5-r8}

	// If we are still below bss_end, loop.
2:
	cmp r4, r9
	blo 1b

	// Call kernel_main
	bl kernel_main

	// halt
halt:
	wfe
	b halt


// Interupt handlers

.globl undef_handler
undef_handler:
    push {r0-r12, lr}
    bl c_undef_handler
    pop {r0-r12, lr}
    subs pc, lr, #0

.globl swi_handler
swi_handler:
    push {r0-r12, lr}
    bl c_swi_handler
    pop {r0-r12, lr}
    subs pc, lr, #4

.globl prefetch_handler
prefetch_handler:
    push {r0-r12, lr}
    bl c_prefetch_handler
    pop {r0-r12, lr}
    subs pc, lr, #4

.globl data_handler
data_handler:
    push {r0-r12, lr}
    bl c_data_handler
    pop {r0-r12, lr}
    subs pc, lr, #8

.globl reserved_handler
reserved_handler:
    push {r0-r12, lr}
    bl c_reserved_handler
    pop {r0-r12, lr}
    subs pc, lr, #4

.globl irq_handler
irq_handler:
    push {r0-r12, lr}
    bl c_irq_handler
    pop {r0-r12, lr}
    subs pc, lr, #4

.globl fiq_handler
fiq_handler:
    push {r0-r12, lr}
    bl c_fiq_handler
    pop {r0-r12, lr}
    subs pc, lr, #4