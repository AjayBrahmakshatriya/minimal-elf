all: run
build: elf_gen
elf_gen: elf.c
	$(CC) elf.c -o elf_gen

run: build
	./elf_gen program
	chmod +x program
	ls -l program
	./program


clean:
	- rm program elf_gen
