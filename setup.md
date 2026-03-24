sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi


# Running boot.S

Make a object file
arm-none-eabi-gcc -mcpu=arm1176jzf-s -fpic -ffreestanding -c boot.S -o boot.o

Link
arm-none-eabi-ld -Ttext=0x8000 boot.o -o boot.elf

Strip
arm-none-eabi-objcopy boot.elf -O binary kernel.img

Run in QEMU
qemu-system-arm -M raspi0 -kernel kernel.img -serial stdio -display none
With debugger
qemu-system-arm -M raspi0 -kernel kernel.img -serial stdio -display none -s -S

DEBUGING!!
arm-none-eabi-gdb boot.elf
