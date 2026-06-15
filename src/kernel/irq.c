#include <kernel/kernel.h>
#include <kernel/printf.h>

void irq_init(void) {
    printf("Initilizing IRQ interupts the mmio base is 0x%x\n", MMIO_BASE);
    // This enables irq... maybe...
    __asm__ volatile ("cpsie i");

    mmio_write(ENABLE_IRQS_1, (1 << 1)); // Enable timer C1
}

void c_irq_handler(void) {
    printf("IRQ handles\n");
}