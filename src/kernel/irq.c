#include <kernel/base.h>
#include <kernel/printf.h>

uint32_t timer_read_us() {
	return mmio_read(SYSTEM_TIMER_CLO);
}

void time_wait_us(uint32_t us) {
	uint32_t start = timer_read_us();	
	while (timer_read_us() - start < us);
}

void irq_init_timer(uint32_t time_interval_ms) {
	uint32_t time = timer_read_us();
	mmio_write(SYSTEM_TIMER_CS, (1 << 1));
	mmio_write(SYSTEM_TIMER_C1, time + time_interval_ms);
}

void irq_init(void) {
    printf("Initilizing IRQ interupts\n", MMIO_BASE);
    // This enables irq... maybe...

    mmio_write(ENABLE_IRQS_1, (1 << 1)); // Enable timer C1
    mmio_write(ENABLE_IRQS_2, (1 << 25)); // Enable UART RX    

    // Now interupts can happen?
    __asm__ volatile ("cpsie i");

    printf("IRQ interupts have now been activated\n");
}

void c_irq_handler(void) {
    uint32_t irq_1 = mmio_read(IRQ_PENDING_1); 

    if ((irq_1 & 2)) {
        printf("Timer event\n");

        uint32_t time = mmio_read(SYSTEM_TIMER_CLO);
        mmio_write(SYSTEM_TIMER_C1, time + 1000000);
	    mmio_write(SYSTEM_TIMER_CS, (1 << 1));

    }

    uint32_t uart_event = mmio_read(UART0_MIS);
    if (uart_event & ((1 << 4) | (1 << 6))) {
        
        while (!(mmio_read(UART0_FR) & (1 << 4))) {
            char c = mmio_read(UART0_DR); 
            printf("%c", c); 
        }

        mmio_write(UART0_ICR, (1 << 4) | (1 << 6));
    }

}