#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef uint64_t Elf64_Addr;
typedef uint16_t Elf64_Half;
typedef uint64_t Elf64_Off;
typedef int32_t  Elf64_Sword;
typedef uint32_t Elf64_Word;
typedef uint64_t Elf64_Xword;
typedef int64_t  Elf64_Sxword;

#define EI_NIDENT 16


typedef struct {
	unsigned char e_ident[EI_NIDENT];
	Elf64_Half e_type;
	Elf64_Half e_machine;
	Elf64_Word e_version;
	Elf64_Addr e_entry;
	Elf64_Off e_phoff;
	Elf64_Off e_shoff;
	Elf64_Word e_flags;
	Elf64_Half e_ehsize;
	Elf64_Half e_phentsize;
	Elf64_Half e_phnum;
	Elf64_Half e_shentsize;
	Elf64_Half e_shnum;
	Elf64_Half e_shstrndx;
} Elf64_Ehdr;

typedef struct {
	Elf64_Word p_type;
	Elf64_Word p_flags;
	Elf64_Off p_offset;
	Elf64_Addr p_vaddr;
	Elf64_Addr p_paddr;
	Elf64_Xword p_filesz;
	Elf64_Xword p_memsz;
	Elf64_Xword p_align;
} Elf64_Phdr;

static void create_e_ident (Elf64_Ehdr *hdr) {
	// Set magic
	hdr->e_ident[0] = '\x7f';
	hdr->e_ident[1] = 'E';
	hdr->e_ident[2] = 'L';
	hdr->e_ident[3] = 'F';
	// Set class	
	hdr->e_ident[4] = 2; // 64 bit
	// Set data (byte ordering)
	hdr->e_ident[5] = 1; // Little Endian
	// Set version
	hdr->e_ident[6] = 1; // Current
	// Set OS ABI
	hdr->e_ident[7] = 0; // Sys V
	hdr->e_ident[8] = 0; // ABI specific parameter, Sys V requires 0
	// Set padding
	hdr->e_ident[9] = 0;
	hdr->e_ident[10] = 0;
	hdr->e_ident[11] = 0;
	hdr->e_ident[12] = 0;
	hdr->e_ident[13] = 0;
	hdr->e_ident[14] = 0;
	hdr->e_ident[15] = 0;
	return;	
}

#define VADDR_START (0x400000ull)
#define PAGE_ALIGN (4096)

/* DEFAULT PROGRAM TO BE ENCODED 
\x48\x65\x6c\x6c\x6f\x20\x57\x6f\x72\x6c\x64\x0a\xb8\x01\x00\x00\x00\xbf\x01\x00
\x00\x00\x48\xbe\x00\x00\x00\x00\x00\x00\x00\x00\xba\x0c\x00\x00\x00\x0f\x05\xb8
\x3c\x00\x00\x00\x48\x31\xff\x0f\x05
 ---------------------------- */
#define DEFAULT_PROGRAM_PATCH_ADDRESS_INDEX (24)
#define DEFAULT_PROGRAM_ENTRY_POINT (0xc)


unsigned char default_program[] = "\x48\x65\x6c\x6c\x6f\x20\x57\x6f\x72\x6c\x64\x0a\xb8\x01\x00\x00\x00\xbf\x01\x00"
"\x00\x00\x48\xbe\x00\x00\x00\x00\x00\x00\x00\x00\xba\x0c\x00\x00\x00\x0f\x05\xb8" 
"\x3c\x00\x00\x00\x48\x31\xff\x0f\x05";

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("Usage: %s <gen_file_name>\n", argv[0]);
		return -1;
	}
	// Calculate file size 
	uint32_t total_size = 0;
	
	// ELF Header
	total_size += sizeof(Elf64_Ehdr);	

	// Program header table with 1 entry
	uint32_t phoffset = total_size;
	total_size += sizeof(Elf64_Phdr) * 1;

	// Actual program section
	uint32_t program_offset = total_size;
	total_size += sizeof(default_program);


	// Allocate a buffer 	
	char* new_file = malloc(total_size);

	// Start setting parts of file

	// ELF Header
	Elf64_Ehdr *hdr = (void*)new_file;
	create_e_ident(hdr);
	hdr->e_type = 2; // Executable
	hdr->e_machine = 62; // AMD 64
	hdr->e_version = 1; // Current
	hdr->e_entry = VADDR_START + program_offset + DEFAULT_PROGRAM_ENTRY_POINT; // Entry point is start of vaddress
	hdr->e_phoff = phoffset;
	hdr->e_shoff = 0; // Currently unset, patch in later
	hdr->e_flags = 0; // Processor specific, 0 for ld produced binaries
	hdr->e_ehsize = sizeof(*hdr);
	hdr->e_phentsize = sizeof(Elf64_Phdr);
	hdr->e_phnum = 1;
	hdr->e_shentsize = 0;
	hdr->e_shentsize = 0;

	// Program Header Table
	// We have a single entry
	Elf64_Phdr *phdr = (void*)(new_file + phoffset);
	phdr->p_type = 1; // LOADABLE
	phdr->p_flags = 0x4 | 0x1 | 0x2; // READ + WRITE + EXECUTE
	phdr->p_offset = program_offset; 
	phdr->p_vaddr = VADDR_START + program_offset; // Chosen virtual address for executable
	phdr->p_paddr = 0;
	phdr->p_filesz = sizeof(default_program); // Size of code segment patch in later
	phdr->p_memsz = phdr->p_filesz; // Same as the size of file
	phdr->p_align = PAGE_ALIGN; // No alignment requirement
	
	
	// Now the actual program
	unsigned char* program_start = new_file + program_offset;
	memcpy(program_start, default_program, sizeof(default_program));
	// Patch in the required address
	*(unsigned long long*)(&program_start[DEFAULT_PROGRAM_PATCH_ADDRESS_INDEX]) = VADDR_START + program_offset;


	// Finally open file and write	
	FILE* outfile = fopen(argv[1], "wb");
	fwrite(new_file, 1, total_size, outfile);
	fclose(outfile);
	free(new_file);

	return 0;
}
