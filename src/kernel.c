#include <stddef.h>
#include <stdint.h>
#include "kernel.h"
#include "graphics.h"
#include "helper.h"
#include "units.h"

static uint32_t MMIO_BASE;

// The MMIO area base address, depends on board type
static inline void mmio_init(int raspi)
{
    switch (raspi) {
        case 2:
        case 3:  MMIO_BASE = 0x3F000000; break; // for raspi2 & 3
        case 4:  MMIO_BASE = 0xFE000000; break; // for raspi4
        default: MMIO_BASE = 0x20000000; break; // for raspi1, raspi zero etc.
    }
}

// Memory-Mapped I/O output
static inline void mmio_write(uint32_t reg, uint32_t data)
{
	*(volatile uint32_t*)(MMIO_BASE + reg) = data;
}

// Memory-Mapped I/O input
static inline uint32_t mmio_read(uint32_t reg)
{
	return *(volatile uint32_t*)(MMIO_BASE + reg);
}

// Adress from mmio
static inline void mmio_address(uint32_t reg) {
	printf("You tried to access adress: %i\r\n", (uint32_t*)(MMIO_BASE + reg));
}

// Loop <delay> times in a way that the compiler won't optimize away
static inline void delay(int32_t count)
{
	asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
		 : "=r"(count): [count]"0"(count) : "cc");
}

// A Mailbox message with set clock rate of PL011 to 3MHz tag
volatile unsigned int  __attribute__((aligned(16))) mbox[9] = {
    9*4, 0, 0x38002, 12, 8, 2, 3000000, 0 ,0
};

void uart_init(int raspi)
{
	mmio_init(raspi);

	// Disable UART0.
	mmio_write(UART0_CR, 0x00000000);
	// Setup the GPIO pin 14 && 15.

	// Disable pull up/down for all GPIO pins & delay for 150 cycles.
	mmio_write(GPPUD, 0x00000000);
	delay(150);

	// Disable pull up/down for pin 14,15 & delay for 150 cycles.
	mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
	delay(150);

	// Write 0 to GPPUDCLK0 to make it take effect.
	mmio_write(GPPUDCLK0, 0x00000000);

	// Clear pending interrupts.
	mmio_write(UART0_ICR, 0x7FF);

	// Set integer & fractional part of baud rate.
	// Divider = UART_CLOCK/(16 * Baud)
	// Fraction part register = (Fractional part * 64) + 0.5
	// Baud = 115200.

	// For Raspi3 and 4 the UART_CLOCK is system-clock dependent by default.
	// Set it to 3Mhz so that we can consistently set the baud rate
	if (raspi >= 3) {
		// UART_CLOCK = 30000000;
		unsigned int r = (((unsigned int)(&mbox) & ~0xF) | 8);
		// wait until we can talk to the VC
		while ( mmio_read(MBOX_STATUS) & 0x80000000 ) { }
		// send our message to property channel and wait for the response
		mmio_write(MBOX_WRITE, r);
		while ( (mmio_read(MBOX_STATUS) & 0x40000000) || mmio_read(MBOX_READ) != r ) { }
	}

	// Divider = 3000000 / (16 * 115200) = 1.627 = ~1.
	mmio_write(UART0_IBRD, 1);
	// Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
	mmio_write(UART0_FBRD, 40);

	// Enable FIFO & 8 bit data transmission (1 stop bit, no parity).
	mmio_write(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

	// Mask all interrupts.
	mmio_write(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) |
	                       (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));

	// Enable UART0, receive & transfer part of UART.
	mmio_write(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
}

void uart_putc(unsigned char c)
{
	// Wait for UART to become ready to transmit.
	while ( mmio_read(UART0_FR) & (1 << 5) ) { }
	mmio_write(UART0_DR, c);
}

unsigned char uart_getc()
{
    // Wait for UART to have received something.
    while ( mmio_read(UART0_FR) & (1 << 4) ) { }
    return mmio_read(UART0_DR);
}

void uart_puts(const char* str)
{
	for (size_t i = 0; str[i] != '\0'; i++) {
		uart_putc((unsigned char)str[i]);
	}
}

inline uint32_t mbox_read(uint32_t channel) {
	uint32_t r = 0;
	do {
		// Loop untill mbox data is ready
		while (mmio_read(MBOX_STATUS) & (1U << 30));
		r = mmio_read(MBOX_READ);
	} while ((r & 0xF) != channel);

	return r & 0xFFFFFFF0;
}

inline void mbox_write(uint32_t v, uint32_t channel) {
	// Loop untill mbox isnt full
	while (mmio_read(MBOX_STATUS) & (1U << 31));
	mmio_write(MBOX_WRITE, channel | (v & 0xFFFFFFF0));

}

uint32_t timer_read_us() {
	return mmio_read(SYSTEM_TIMER_CLO);
}

void time_wait_us(uint32_t us) {
	uint32_t start = timer_read_us();	
	while (timer_read_us() - start < us);
}

#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif

#ifdef AARCH64
// arguments for AArch64
void kernel_main(uint64_t dtb_ptr32, uint64_t x1, uint64_t x2, uint64_t x3)
#else
// arguments for AArch32
void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags)
#endif
{
	// Remove unused warnings
	(void)r0;
	(void)r1;
	(void)atags;

	// initialize UART for Raspi0
	uart_init(5);

	printf("Welcome to this very beautiful game console firmware for raspi2 and raspi0\r\nThough its not really tested on actual hardware yet -_-\r\n");

	// Frame buffer init
	while (init_frame_buffer() < 0);

	uint32_t start = timer_read_us();


	while (1) {
		uint32_t next = timer_read_us();
		if (next - start > SECOND_30 * 4) {
			
			draw_circle(100, 100, 20, (rgb){100,23,134});

			start = next;
			frame_buffer_swap();
		}
	}
}