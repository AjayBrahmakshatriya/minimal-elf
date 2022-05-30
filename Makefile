all: run
build: elf_gen

elf_gen: elf.c program.S
	$(CC) program.S -c -o program.o
	objcopy -O binary -j .text program.o program_t
	$(CC) elf.c -o elf_gen

run: build
	./elf_gen program_t program
	chmod +x program
	ls -l program
	./program


clean:
	- rm program elf_gen
	- rm program.o program_t
