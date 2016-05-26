#ifndef SROM_UTIL_H
#define SROM_UTIL_H

#include <stdint.h>

typedef struct {
	char filename[64];
	char game_title[21];		// $xFC0
	int makeup_byte;		// $xFD5 xxAAxxxB 
	int map_mode;			// $xFD6
	char map_mode_desc[6];
	int reported_size;		// $xFD7	
	char size_desc[16];
	uint64_t file_size;
	int type;
	char type_desc[32];
	int speed;
	char speed_desc[32];
	int country;
	char country_desc[50];
	char video_mode[24];
	int company;
	char company_desc[50];
	int sram_size;			// $xFD8
	char sram_size_desc[6];
	int creator_id;			// $xFD9
	int version;		// $xFDB
	int checksum_comp;	// $xFDC
	int checksum;			// $xFDE	
	int reset_vector;
	int map;
} rom_header;

#endif
