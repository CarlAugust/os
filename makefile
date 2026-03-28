rsb0:
	arm-none-eabi-gcc -mcpu=arm1176jzf-s -fpic -ffreestanding -c src/boot/bootpi0.S -o out/boot.o
	arm-none-eabi-gcc -mcpu=arm1176jzf-s -fpic -ffreestanding -std=gnu99 -c src/kernel.c -o out/kernel.o -O2 -Wall -Wextra
	arm-none-eabi-gcc -T linker.ld -o out/kernel.elf -ffreestanding -O2 -nostdlib out/boot.o out/kernel.o -lgcc
	arm-none-eabi-objcopy out/kernel.elf -O binary out/kernel7.img
	qemu-system-arm -M raspi0 -serial stdio -kernel out/kernel.elf -display none