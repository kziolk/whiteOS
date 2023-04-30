FILES = ./build/boot/boot.asm.o ./build/kernel.o ./build/idt/idt.asm.o ./build/idt/idt.o \
		./build/io/io.asm.o ./build/io/io.o

INCLUDES = -I./src

FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

all: ./bin/whiteos.bin 
	cp bin/whiteos.bin isodir/boot/whiteos.bin
	grub-mkrescue -o whiteos.iso isodir

./bin/whiteos.bin: $(FILES)
	i686-elf-ld -g -relocatable $(FILES) -o ./build/kernelfull.o
	i686-elf-gcc $(FLAGS) -T src/linker.ld -o bin/whiteos.bin build/kernelfull.o -lgcc

./build/boot/boot.asm.o: ./src/boot/boot.asm
	nasm -f elf -g src/boot/boot.asm -o build/boot/boot.asm.o

./build/kernel.o: ./src/kernel.c
	i686-elf-gcc $(FLAGS) $(INCLUDES) -c src/kernel.c -o build/kernel.o -std=gnu99

./build/idt/idt.asm.o: ./src/idt/idt.asm
	nasm -f elf -g src/idt/idt.asm -o build/idt/idt.asm.o

./build/idt/idt.o: ./src/idt/idt.c
	i686-elf-gcc $(FLAGS) $(INCLUDES) -I./src/idt -c src/idt/idt.c -o build/idt/idt.o -std=gnu99

./build/gdt/gdt.asm.o: ./src/gdt/gdt.asm
	nasm -f elf -g src/gdt/gdt.asm -o build/gdt/gdt.asm.o

./build/gdt/gdt.o: ./src/gdt/gdt.c
	i686-elf-gcc $(FLAGS) $(INCLUDES) -I./src/gdt -c src/gdt/gdt.c -o build/gdt/gdt.o -std=gnu99

./build/idt/isr.asm.o: ./src/idt/isr.asm
	nasm -f elf -g src/idt/isr.asm -o build/idt/isr.asm.o

./build/idt/isr.o: ./src/idt/isr.c
	i686-elf-gcc $(FLAGS) $(INCLUDES) -I./src/idt -c src/idt/isr.c -o build/idt/isr.o -std=gnu99

./build/io/io.o: ./src/io/io.c
	i686-elf-gcc $(FLAGS) $(INCLUDES) -I./src/io -c src/io/io.c -o build/io/io.o -std=gnu99

./build/io/io.asm.o: ./src/io/io.asm
	nasm -f elf -g src/io/io.asm -o build/io/io.asm.o


clean:
	rm -rf ./bin/whiteos.bin
	rm -rf ${FILES}
	rm -rf whiteos.iso
	rm -rf isodir/boot/whiteos.bin
