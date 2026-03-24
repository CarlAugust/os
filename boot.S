.section ".text.boot"
.globl _start

_start:
    mov sp, #0x8000
    BL uart_init
    ldr r0, =0x41
    BL uart_write_char

    ldr r0, =0x41
    BL uart_write_char

    ldr r0, =0x41
    BL uart_write_char

    ldr r0, =0x41
    BL uart_write_char
    

hang:
    b hang

uart_init:
    push {r4, lr}

    ldr r1, =0x20201000 @ Address for UART
    mov r0, #0
    str r0, [r1, #0x30] @ store r0 into adress r1 + 0x30, Disalbe UART

    mov r0, #1
    str r0, [r1, #0x24] @ store r0 into adress r1 + 0x24, SET IBRD

    mov r0, #40
    str r0, [r1, #0x28] @ store r0 into adress r1 + 0x28    , SET FBRD

    mov r0, #0x70 @ (1<<4 | (1<<5) | (1<<6)
    str r0, [r1, #0x2C]

    ldr r0, =0x301 @ (1<<0) | (1<<8)
    str r0, [r1, #0x30]

    pop {r4, pc}

uart_write_char: @r0 is character
    push {r4, lr}

    ldr r1, =0x20201000 @ Adress for UART

    wait: 
        ldr r2, [r1, #0x18]
        AND r2, r2, #0x20
        BNE wait
    
    str r0, [r1, #0x00] @ Writes r0


    pop {r4, pc}