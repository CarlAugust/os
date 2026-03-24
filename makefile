rsb0:
	arm-none-eabi-gcc -mcpu=arm1176jzf-s -fpic -ffreestanding -c boot.S -o boot.o
	arm-none-eabi-ld -Ttext=0x8000 boot.o -o boot.elf
	arm-none-eabi-objcopy boot.elf -O binary kernel.img
	qemu-system-arm -M raspi0 -kernel kernel.img -serial stdio -display none