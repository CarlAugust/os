#include <kernel/kernel.h>
#include <kernel/printf.h>

void irq_init(void) {
    __asm__ volatile (
        "mrs r0, cpsr\n"
        "bic r0, r0, #0x80\n"   // enable IRQ (clear I bit)
        "msr cpsr_c, r0\n"
    );
    
    mmio_write(ENABLE_IRQS_1, (1 << 1));
}

void c_irq_handler(void) {
    uint32_t irq = mmio_read(IRQ_BASIC_PENDING);
    switch (irq) {

        default:
            printf("Irq event %i was called\n", irq);
    }
}