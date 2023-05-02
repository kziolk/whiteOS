FILES = ./build/boot/boot.asm.o ./build/kernel.o ./build/idt/idt.asm.o ./build/idt/idt.o \
		./build/io/io.asm.o ./build/io/io.o ./build/pic/pic.o \
		./build/whitelib/string.o ./build/whitelib/programs/defscreen.o ./build/whitelib/system.o \
		./build/keyboard/keyboard.o ./build/terminal/terminal.o \
		./build/memory/heap/heap.o ./build/memory/heap/kheap.o ./build/memory/memory.o \
		./build/memory/paging/paging.o ./build/memory/paging/paging.asm.o \
		./build/disk/disk.o ./build/fs/pparser.o ./build/disk/streamer.o

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

./build/memory/memory.o: ./src/memory/memory.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -I./src/memory -std=gnu99 -c ./src/memory/memory.c -o ./build/memory/memory.o

./build/idt/isr.asm.o: ./src/idt/isr.asm
	nasm -f elf -g src/idt/isr.asm -o build/idt/isr.asm.o

./build/idt/isr.o: ./src/idt/isr.c
	i686-elf-gcc $(FLAGS) $(INCLUDES) -I./src/idt -c src/idt/isr.c -o build/idt/isr.o -std=gnu99

./build/io/io.o: ./src/io/io.c
	i686-elf-gcc $(FLAGS) $(INCLUDES) -I./src/io -c src/io/io.c -o build/io/io.o -std=gnu99

./build/io/io.asm.o: ./src/io/io.asm
	nasm -f elf -g src/io/io.asm -o build/io/io.asm.o

./build/pic/pic.o: ./src/pic/pic.c
	i686-elf-gcc $(FLAGS) $(INCLUDES) -I./src/pic -c src/pic/pic.c -o build/pic/pic.o -std=gnu99

./build/whitelib/string.o: ./src/whitelib/string/string.c
	i686-elf-gcc $(FLAGS) $(INCLUDES) -I./src/whitelib -c src/whitelib/string/string.c -o build/whitelib/string.o -std=gnu99

./build/whitelib/programs/defscreen.o: ./src/whitelib/programs/defscreen.c
	i686-elf-gcc $(FLAGS) $(INCLUDES) -I./src/whitelib -c src/whitelib/programs/defscreen.c -o build/whitelib/programs/defscreen.o -std=gnu99

./build/whitelib/system.o: ./src/whitelib/system/system.c
	i686-elf-gcc $(FLAGS) $(INCLUDES) -I./src/whitelib -c src/whitelib/system/system.c -o build/whitelib/system.o -std=gnu99

./build/terminal/terminal.o: ./src/terminal/terminal.c
	i686-elf-gcc $(FLAGS) $(INCLUDES) -I./src/terminal -c src/terminal/terminal.c -o build/terminal/terminal.o -std=gnu99

./build/keyboard/keyboard.o: ./src/keyboard/keyboard.c
	i686-elf-gcc $(FLAGS) $(INCLUDES) -I./src/keyboard -c src/keyboard/keyboard.c -o build/keyboard/keyboard.o -std=gnu99

./build/memory/heap/heap.o: ./src/memory/heap/heap.c
	i686-elf-gcc $(FLAGS) $(INCLUDES) -I./src/memory/heap -c src/memory/heap/heap.c -o build/memory/heap/heap.o -std=gnu99

./build/memory/heap/kheap.o: ./src/memory/heap/kheap.c
	i686-elf-gcc $(FLAGS) $(INCLUDES) -I./src/memory/heap -c src/memory/heap/kheap.c -o build/memory/heap/kheap.o -std=gnu99

./build/memory/paging/paging.asm.o: ./src/memory/paging/paging.asm
	nasm -f elf -g src/memory/paging/paging.asm -o build/memory/paging/paging.asm.o

./build/memory/paging/paging.o: ./src/memory/paging/paging.c
	i686-elf-gcc $(FLAGS) $(INCLUDES) -I./src/memory/paging -c src/memory/paging/paging.c -o build/memory/paging/paging.o -std=gnu99

./build/disk/disk.o: ./src/disk/disk.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -I./src/disk -std=gnu99 -c ./src/disk/disk.c -o ./build/disk/disk.o

./build/fs/pparser.o: ./src/fs/pparser.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -I./src/fs -std=gnu99 -c ./src/fs/pparser.c -o ./build/fs/pparser.o

./build/disk/streamer.o: ./src/disk/streamer.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/disk/streamer.c -o ./build/disk/streamer.o





clean:
	rm -rf ./bin/whiteos.bin
	rm -rf ${FILES}
	rm -rf whiteos.iso
	rm -rf isodir/boot/whiteos.bin
