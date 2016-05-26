#include <stdio.h>
#include <stdlib.h>
#include <byteswap.h>
#include <stdint.h>
#include <string.h>

#include "util.h"
#include "srom_util.h"

// #define DEBUG 0
#define FALSE 0
#define TRUE 1
#define MAKEUP_BYTE_ADDR 0x7fd5
#define TYPE_ADDR 0x7fd6
#define ROM_SIZE_ADDR 0x7fd7
#define SRAM_SIZE_ADDR 0x7fd8
#define COUNTRY_ADDR 0x7fd9
#define COMPANY_ADDR 0x7fda
#define VERSION_ADDR 0x7fdb
#define CHECKSUM_COMP_ADDR 0x7fdc
#define CHECKSUM_ADDR 0x7fde

// Later on I'll or these with the last three bytes of the rom header
// address to get the proper address based on wether the rom is LoROM
// or HiROM
#define LOROM 0x7000
#define HIROM 0xf000

#define X 0x8000
//#define X 0x0000

// 0x186c7a
FILE *rom_file;
uint8_t *rom_contents;

rom_header header;

void read_header() {
	
	// read header info

	header.makeup_byte = rom_contents[MAKEUP_BYTE_ADDR | X];
	header.map_mode = header.makeup_byte & 0b00000001;
	header.speed = (header.makeup_byte >> 4) & 0b00000111;

	header.sram_size = rom_contents[SRAM_SIZE_ADDR | X];
	switch (header.sram_size) {
		case 0x0:
			strcpy(header.sram_size_desc, "(none)");
			break;
		case 0x1:
			strcpy(header.sram_size_desc, "16Kb");
			break;
		case 0x2:
			strcpy(header.sram_size_desc, "32Kb");
			break;
		case 0x3:
			strcpy(header.sram_size_desc, "64Kb");
			break;
		default:
			strcpy(header.sram_size_desc, "ERR");
	}

	header.country = rom_contents[COUNTRY_ADDR | X];
	switch (header.country) {
		case 0x01:
			strcpy(header.country_desc, "United States");
			strcpy(header.video_mode, "NTSC");
			break;
		default:
			strcpy(header.country_desc, "ERR");
			strcpy(header.video_mode, "ERR");
			break;
	}

	header.company = rom_contents[COMPANY_ADDR | X];
	switch (header.company) {
		case 0x33:
			strcpy(header.company_desc, "Nintendo");
			break;
		case 0xc3:
			strcpy(header.company_desc, "Square");
			break;
		default:
			strcpy(header.company_desc, "ERR");
			break;
	}

	header.version = rom_contents[VERSION_ADDR | X];
	header.checksum_comp = rom_contents[CHECKSUM_COMP_ADDR | X] | (rom_contents[(CHECKSUM_COMP_ADDR + 1) | X] << 8);
	header.checksum = rom_contents[CHECKSUM_ADDR | X] | (rom_contents[(CHECKSUM_ADDR + 1) | X]<< 8);

	if (header.speed) {
		strcpy(header.speed_desc, "FastROM (120ns)");
	} else {
		strcpy(header.speed_desc, "SlowRom (200ns)");
	}
	
	if (header.map_mode == 1) {
		strcpy(header.map_mode_desc, "HiROM");
	} else {
		strcpy(header.map_mode_desc, "LoROM");
	}

	header.type = rom_contents[TYPE_ADDR | X];
	switch (header.type) {
		case 0x00:
			strcpy(header.type_desc, "ROM");
			break;
		case 0x01:
			strcpy(header.type_desc, "ROM/RAM");
			break;
		case 0x02:
			strcpy(header.type_desc, "ROM/SRAM");
			break;
		case 0x03:
			strcpy(header.type_desc, "ROM/DSP1");
			break;
		case 0x04:
			strcpy(header.type_desc, "ROM/DSP1/RAM");
			break;
		case 0x05:
			strcpy(header.type_desc, "ROM/DSP1/SRAM");
			break;
		case 0x06:
			strcpy(header.type_desc, "FX");
			break;
		case 0x43:
			strcpy(header.type_desc, "ROM/S-DD1");
			break;
		default:
			strcpy(header.type_desc, "ERROR");
			break;
	}

	header.reported_size = rom_contents[ROM_SIZE_ADDR | X];

	switch (header.reported_size) {
		case 0x08:
			header.size = 2;
			break;
		case 0x09:
			header.size = 4;
			break;
		case 0x0a:
			heaer.size = 8;
			break;
		case 0x0b:
			header.size = 16;
			break;
		case 0x0c:
			header.size = 32;
			break;
		default:
			strcpy(header.size_desc, "err");
			break;
	}

	header.reset_vector = rom_contents[0x7ffc | X] | (rom_contents[0x7ffd | X] << 8);

	int i;
	for (i = 0; i < 21; i++) {
		header.game_title[i] = rom_contents[(0x7fc0 + i) | X];
	}

	header.game_title[21] = '\0';

	return;
}

void display_header() {
	printf("Filename: \"%s\"\n", header.filename);	
	printf("Game Title: %s\n", header.game_title);
	printf("ROM Makeup Byte: $%02x\n", header.makeup_byte);
	printf("Speed: %02x %s\n", header.speed, header.speed_desc);
	printf("Map Mode: %s\n", header.map_mode_desc);
	printf("Rom Type: %02x %s\n", header.type, header.type_desc);
	printf("Header ROM Size: %02x (%d %s)\n", header.reported_size, header.size, header.size_desc);
	printf("Calculated ROM Size: %lu\n", header.file_size);
	printf("SRAM size: %02x %s\n", header.sram_size, header.sram_size_desc);
	printf("Header Checksum: %04x\n", header.checksum);
	printf("Header Checksum Complement: %04x\n", header.checksum_comp);
	//printf("Calculated Checksum: %s\n", "placeholder");
	printf("Output: %s\n", header.video_mode);
	//printf("CRC32: %s\n", "placeholder");
	printf("Region: %s\n", header.country_desc);
	printf("Licensed by: %s\n", header.company_desc);
	printf("ROM Version: 1.%d\n", header.version);
	printf("Reset Vector: $%04x\n", header.reset_vector);
	return;
}

/******
 * 
 * Display bytes as snes opcodes
 *
 ******/
void assemble() {
	int addr = header.reset_vector;
	int byte = rom_contents[addr];
	char opcode[12];
	//memset(opcode, '\0', sizeof(opcode));

	switch (byte) {
		case 0x78:
			strcpy(opcode, "SEI");		
			break;
		default:
			strcpy(opcode, "NOP");
			break;
	}
	printf("$%04x\t$%02x\t%s\n", addr, byte, opcode);
	return;
}

uint64_t load_rom(char *filename) {
	
	rom_file = fopen(filename, "rb");

	uint64_t rom_length;
	
	if (rom_file == NULL) {
		fprintf(stderr, "Can't open input file %s.\n", filename);
		exit(EXIT_FAILURE);
	}

	//#ifdef DEBUG
	printf("Opened %s.\n", filename);
	//#endif
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
	
	return rom_length;
}

void dump_memory() {
	#ifdef DEBUG
	printf("dump_memory()\n");
	#endif  
	
	char ascii[16];
	char format_string[120];
	char final_string[120];

	memset(format_string, '\0', sizeof(format_string));
	memset(final_string, '\0', sizeof(final_string));

	strcpy(format_string, "%08x|%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x|%s\n");

	unsigned int i, j, k;
	unsigned int lines = 16;
	unsigned int base = 0xffbc;
	for (i = 0; i < lines * 15; i += 15) {
		for (j = 0; j < 15; j++) {
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
	return;
}

void parse_input(char *input) {

	#ifdef DEBUG
	printf("Input: %s\n", input);
	#endif
	
	char *args[8];

	/*
	int i;
	i = str_length(input);
	printf("%d\n", i);
	*/

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
	return;
}

int main(void) {

	char filename[32];
	uint64_t rom_length;
	//unsigned long rom_length;

	//strcpy(filename, "Street Fighter Alpha 2 (U) [!].smc");
	//strcpy(filename, "Final Fantasy II (USA).sfc");
	strcpy(filename, "Donkey Kong Country 2 - Diddy's Kong Quest (USA) (En,Fr).sfc");
	rom_length = load_rom(filename);
	read_header();

	header.file_size = rom_length;

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
