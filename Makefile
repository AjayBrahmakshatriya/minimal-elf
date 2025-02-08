all: run

elf_gen: elf.c
	$(CC) elf.c -o elf_gen

program_t: program.c
	$(CC) -c -O3 program.c -o program.o  -fcf-protection=none
	objcopy -O binary -j .text program.o program_t

program: elf_gen program_t
	./elf_gen program_t program
	chmod +x program

.PHONY: run
run: program
	ls -l program
	./program


clean:
	- rm program elf_gen
	- rm program.o program_t
