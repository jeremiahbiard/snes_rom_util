#include <stdio.h>
#include <stdlib.h>
#include <byteswap.h>
#include <stdint.h>
#include <string.h>

#include "util.h"
#include "srom_util.h"

#define DEBUG
#define FALSE 0
#define TRUE 1
#define MAKEUP_BYTE_ADDR 0x7fd5
#define TYPE_ADDR 0x7fd6
#define ROM_SIZE_ADDR 0x7fd7

// 0x186c7a
FILE *rom_file;
uint8_t *rom_contents;


rom_header header;

void display_header() {
	printf("Filename: \"%s\"\n", header.filename);	
	printf("Game Title: %s\n", header.game_title);
	//printf("Speed: %s\n", "");
	printf("ROM Makeup Byte: $%02x\n", header.makeup_byte);
	printf("Map Mode: %s\n", header.map_mode_desc);
	//printf("Kart contents: %s\n", "placeholder");
	printf("Header ROM Size: %d\n", header.reported_size);
	printf("Calculated ROM Size: %d\n", header.file_size);
	//printf("SRAM size: %s\n", "placeholder");
	//printf("Actual Checksum: %s\n", "placeholder");
	//printf("Header Checksum: %s\n", "placeholder");
	//printf("Header Checksum Compliment: %s\n", "2465");
	//printf("Output: %s\n", "NTSC 60Hz");
	//printf("CRC32: %s\n", "placeholder");
	//printf("Licensee: %s\n", "placeholder");
	//printf("ROM Version: %s\n", "placeholder");
	printf("Reset Vector: $%04x\n", header.reset_vector);
}

void assemble() {
	int addr = 0x0; // header.reset_vector;
	int byte = rom_contents[addr];
	char opcode[4];
	//memset(opcode, '\0', sizeof(char));

	switch (byte) {
		case 0x78:
			strcpy(opcode, "SEI");		
			break;
		default:
			strcpy(opcode, "NOP");
			break;
	}
	printf("$%04x\t$%02x\t%s\n", addr, byte, opcode);
}

uint64_t load_rom(char *filename) {
	
	rom_file = fopen(filename, "rb");

	uint64_t rom_length;
	
	if (rom_file == NULL) {
		fprintf(stderr, "Can't open input file %s.\n", filename);
		exit(EXIT_FAILURE);
	}

	#ifdef DEBUG
	printf("Opened %s.\n", filename);
	#endif
	strcpy(header.filename, filename);
  
	// Get file length
	fseek(rom_file, 0, SEEK_END);
	rom_length = ftell(rom_file);
	fseek(rom_file, 0, SEEK_SET);
	
	rom_contents = (uint8_t *)calloc(rom_length + 1, sizeof(uint8_t));
  
	if (!rom_contents) {
		fprintf(stderr, "Memory error!");
		fclose(rom_file);
		exit(EXIT_FAILURE);
	}

	// Read rom contents into ram buffer
	fread(rom_contents, rom_length, 1, rom_file);
	fclose(rom_file);
	

	// Write header info
	header.makeup_byte = rom_contents[MAKEUP_BYTE_ADDR];
	header.map_mode = rom_contents[MAKEUP_BYTE_ADDR] & 0b00000001;
	header.file_size = rom_length;
	if (header.map_mode == 1) {
		strcpy(header.map_mode_desc, "hirom");
	} else {
		strcpy(header.map_mode_desc, "lorom");
	}
	header.reported_size = rom_contents[ROM_SIZE_ADDR];
	header.reset_vector = rom_contents[0x7ffc] | (rom_contents[0x7ffd] << 8);

	int i;
	for (i = 0; i < 21; i++) {
		header.game_title[i] = rom_contents[0x7fc0 + i];
	}

	header.game_title[21] = '\0';

	return rom_length;
}

void dump_memory() {
	#ifdef DEBUG
	printf("dump_memory()\n");
	#endif  
	
	uint8_t x;

	char ascii[16];
	char format_string[100];
	char final_string[100];

	memset(format_string, '\0', sizeof(format_string));
	memset(final_string, '\0', sizeof(final_string));

	strcpy(format_string, "%08x|%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x|%s\n");

	unsigned int i, j, k;
	unsigned int lines = 8;
	unsigned int base = 0x7fbc;
	for (i = 0; i < lines * 15; i += 15) {
		for (j = 0; j< 15; j++) {
			ascii[j] = rom_contents[base + j + i];
			if (ascii[j] < 32 || ascii[j] > 126) {
				ascii[j] = '.';
			} 	  
		}
	  
		sprintf(final_string, format_string, base + i, 
			rom_contents[base + 0 + i], //ram[0 + i], 
			rom_contents[base + 1 + i], 
			rom_contents[base + 2 + i], 
			rom_contents[base + 3 + i],
			rom_contents[base + 4 + i], 
			rom_contents[base + 5 + i],
			rom_contents[base + 6 + i], 
			rom_contents[base + 7 + i],
			rom_contents[base + 8 + i], 
			rom_contents[base + 9 + i],
			rom_contents[base + 10 + i], 
			rom_contents[base + 11 + i],
			rom_contents[base + 12 + i], 
			rom_contents[base + 13 + i],
			rom_contents[base + 14 + i],
			ascii);
		printf("%s", final_string);
	}
}

int parse_input(char *input) {
	#ifdef DEBUG
	printf("Input: %s\n", input);
	#endif
	char *args[8];

	int i;
	i = str_length(input);

	printf("%d\n", i);

	int p = input[0];
	switch (p) {
			case 'q':
				exit(0);
				break;
			case 'a':
				assemble();
				break;
			case 'h':
				display_header();	
				break;
			case 'l':
				// load_rom("test");
				printf("loading...\n");
				break;
			case 'd':
				dump_memory();
				break;
			default:
				printf("unknown command\n");
				break;
		}
	input = 0;
	return 0;
}

int main(void) {

	char filename[32];
	unsigned long rom_length;

	//strcpy(filename, "SFA.smc");
	strcpy(filename, "Final Fantasy II (USA).sfc");
	load_rom(filename);

	char input[32];
	memset(input, '\0', sizeof(char));
	while (1) {
		// Prompt
		printf("-");
		fgets(input, 32, stdin);
		parse_input(input);
	}
  exit(EXIT_SUCCESS);
}

// Vectors are located at $00:FFE0-$00:FFFF
// Interrupts cause the program to jump immediately to one of the 
// vectors, such as NMI and IRQ. The first 16 bytes are vectors for
// the native mode while the last 16 are for the emulation mode
//
// Rest Vector: When the reset button is pressed or the power button
// has been turned on, SNES automatically jumps to the pointer in the
// emulation vector because the SNES always starts in emulation mode.
//
// NMI: stands for non-maskable interrupt. It runs every V-Blank.
//
// IRQ: Stands for interrupt request. It runs at a certain time while the
// screen is being drawn. The time is defined by some registers and can
// be at the start of a scanline being drawn, or in the middle of one scan-
// line being drawn.
//
// Vector ROM Map (native mode)
// FFE0: 4 Bytes: Unused
// FFE4: 2 Bytes: Coprocessor Empowerment
// FFE6: 2 Bytes: Program Break
// FFE8: 2 Bytes: ABORT
// FFEA: 2 Bytes: NMI
// ffEC: 2 Bytes: Reset (unused)
// FFEE: 2 Bytes: Interrupt Request
//
// Vector ROM Map (emulation mode)
// FFF0: 4 Bytes: Unused
// FFF4: 2 Bytes: Coprocessor Empowerment
// FFF6: 2 Bytes: Program Break
// FFF8: 2 Bytes: ABORT
// FFFA: 2 Bytes: NMI
// FFFC: 2 Bytes: Reset (used)
// FFFE: 2 Bytes: IRQ
/*
 *
 * Memory Mapping:
 *
 * Bank: 64K 0x10000 bytes, 0xAABBCC is bank 0xAA
 * 256 banks in total 0x00 - 0xFF
 *
 * Page: 4KB (4096 or 0x1000 bytes)
 *
 * Used when the machine has to perform mapping tasks (for example, 
 * ensuring that 0xAABBCC and 0xDDBCC point to the same data, if that's
 * how the machine is supposed to work.
 * 
 * 
 */
