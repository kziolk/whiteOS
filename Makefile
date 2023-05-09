# Important: kernel.asm.o should be first on the FILES list
# Linker needs to put this file at the beginning of kernel.bin, because we jump to const addr = 0x00100000
FILES =	./build/kernel.asm.o ./build/kernel.o \
		./build/idt/idt.asm.o ./build/idt/idt.o \
		./build/io/io.asm.o ./build/io/io.o ./build/pic/pic.o \
		./build/whitelib/string.o ./build/whitelib/programs/ramscroller.o ./build/whitelib/programs/fileexplorer.o ./build/whitelib/system.o \
		./build/keyboard/keyboard.o ./build/terminal/terminal.o \
		./build/memory/heap/heap.o ./build/memory/heap/kheap.o ./build/memory/memory.o \
		./build/memory/paging/paging.o ./build/memory/paging/paging.asm.o \
		./build/disk/disk.o ./build/fs/pparser.o ./build/disk/streamer.o ./build/fs/file.o ./build/fs/fat/fat16.o

FOLDERS = boot disk fs fs/fat gdt idt io keyboard memory memory/heap memory/paging pic \
		  terminal whitelib whitelib/programs

INCLUDES = -I./src

FLAGS = -g -m32 -fno-pie -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

all: ./bin/kernel.bin ./bin/boot.bin
	rm -rf ./bin/whiteos.bin
	dd if=./bin/boot.bin >> ./bin/whiteos.bin
	dd if=./bin/kernel.bin >> ./bin/whiteos.bin
	dd if=/dev/zero bs=1048576 count=16 >> ./bin/whiteos.bin

./bin/kernel.bin: build
	gcc $(FLAGS) -T src/linker.ld -o bin/kernel.bin ./build/kernelfull.o

./bin/boot.bin: ./src/boot/boot.asm
	nasm -f bin src/boot/boot.asm -o bin/boot.bin

build: $(FILES)
	ld -g -melf_i386 -relocatable $(FILES) -o ./build/kernelfull.o

./build/kernel.asm.o: ./src/kernel.asm
	nasm -f elf -g src/kernel.asm -o build/kernel.asm.o

./build/kernel.o: ./src/kernel.c
	gcc $(FLAGS) $(INCLUDES) -c src/kernel.c -o build/kernel.o -std=gnu99

./build/idt/idt.asm.o: ./src/idt/idt.asm
	nasm -f elf -g src/idt/idt.asm -o build/idt/idt.asm.o

./build/idt/idt.o: ./src/idt/idt.c
	gcc $(FLAGS) $(INCLUDES) -I./src/idt -c src/idt/idt.c -o build/idt/idt.o -std=gnu99

./build/idt/isr.asm.o: ./src/idt/isr.asm
	nasm -f elf -g src/idt/isr.asm -o build/idt/isr.asm.o

./build/idt/isr.o: ./src/idt/isr.c
	gcc $(FLAGS) $(INCLUDES) -I./src/idt -c src/idt/isr.c -o build/idt/isr.o -std=gnu99

./build/io/io.o: ./src/io/io.c
	gcc $(FLAGS) $(INCLUDES) -I./src/io -c src/io/io.c -o build/io/io.o -std=gnu99

./build/io/io.asm.o: ./src/io/io.asm
	nasm -f elf -g src/io/io.asm -o build/io/io.asm.o

./build/pic/pic.o: ./src/pic/pic.c
	gcc $(FLAGS) $(INCLUDES) -I./src/pic -c src/pic/pic.c -o build/pic/pic.o -std=gnu99

./build/whitelib/string.o: ./src/whitelib/string/string.c
	gcc $(FLAGS) $(INCLUDES) -I./src/whitelib -c src/whitelib/string/string.c -o build/whitelib/string.o -std=gnu99

./build/whitelib/programs/ramscroller.o: ./src/whitelib/programs/ramscroller.c
	gcc $(FLAGS) $(INCLUDES) -I./src/whitelib -c src/whitelib/programs/ramscroller.c -o build/whitelib/programs/ramscroller.o -std=gnu99

./build/whitelib/programs/fileexplorer.o: ./src/whitelib/programs/fileexplorer.c
	gcc $(FLAGS) $(INCLUDES) -I./src/whitelib -c src/whitelib/programs/fileexplorer.c -o build/whitelib/programs/fileexplorer.o -std=gnu99

./build/whitelib/system.o: ./src/whitelib/system/system.c
	gcc $(FLAGS) $(INCLUDES) -I./src/whitelib -c src/whitelib/system/system.c -o build/whitelib/system.o -std=gnu99

./build/terminal/terminal.o: ./src/terminal/terminal.c
	gcc $(FLAGS) $(INCLUDES) -I./src/terminal -c src/terminal/terminal.c -o build/terminal/terminal.o -std=gnu99

./build/keyboard/keyboard.o: ./src/keyboard/keyboard.c
	gcc $(FLAGS) $(INCLUDES) -I./src/keyboard -c src/keyboard/keyboard.c -o build/keyboard/keyboard.o -std=gnu99

./build/memory/memory.o: ./src/memory/memory.c
	gcc $(FLAGS) $(INCLUDES) -I./src/memory -c src/memory/memory.c -o build/memory/memory.o -std=gnu99

./build/memory/heap/heap.o: ./src/memory/heap/heap.c
	gcc $(FLAGS) $(INCLUDES) -I./src/memory/heap -c src/memory/heap/heap.c -o build/memory/heap/heap.o -std=gnu99

./build/memory/heap/kheap.o: ./src/memory/heap/kheap.c
	gcc $(FLAGS) $(INCLUDES) -I./src/memory/heap -c src/memory/heap/kheap.c -o build/memory/heap/kheap.o -std=gnu99

./build/memory/paging/paging.asm.o: ./src/memory/paging/paging.asm
	nasm -f elf -g src/memory/paging/paging.asm -o build/memory/paging/paging.asm.o

./build/memory/paging/paging.o: ./src/memory/paging/paging.c
	gcc $(FLAGS) $(INCLUDES) -I./src/memory/paging -c src/memory/paging/paging.c -o build/memory/paging/paging.o -std=gnu99

./build/disk/disk.o: ./src/disk/disk.c
	gcc $(INCLUDES) $(FLAGS) -I./src/disk -std=gnu99 -c ./src/disk/disk.c -o ./build/disk/disk.o

./build/fs/pparser.o: ./src/fs/pparser.c
	gcc $(INCLUDES) $(FLAGS) -I./src/fs -std=gnu99 -c ./src/fs/pparser.c -o ./build/fs/pparser.o

./build/disk/streamer.o: ./src/disk/streamer.c
	gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/disk/streamer.c -o ./build/disk/streamer.o

./build/fs/file.o: ./src/fs/file.c
	gcc $(INCLUDES) $(FLAGS) -I./src/fs -std=gnu99 -c ./src/fs/file.c -o ./build/fs/file.o

./build/fs/fat/fat16.o: ./src/fs/fat/fat16.c
	gcc $(INCLUDES) $(FLAGS) -I./src/fs -std=gnu99 -c ./src/fs/fat/fat16.c -o ./build/fs/fat/fat16.o

clean:
	rm -rf ./bin/whiteos.bin
	rm -rf ./bin/kernel.bin
	rm -rf ./bin/boot.bin
	rm -rf ${FILES}
	rm -rf ./build/kernelfull.o
	cd build && mkdir -p ${FOLDERS} && cd ..
