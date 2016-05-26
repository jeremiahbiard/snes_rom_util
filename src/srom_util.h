#ifndef SROM_UTIL_H
#define SROM_UTIL_H

typedef struct {
	char filename[32];
	char game_title[21];		// $xFC0
	int makeup_byte;		// $xFD5 xxAAxxxB 
	int map_mode;			// $xFD6
	char map_mode_desc[6];
	int reported_size;		// $xFD7	
	int file_size;
	int sram_size;			// $xFD8
	int creator_id;			// $xFD9
	int version_number;		// $xFDB
	int checksum_complement;	// $xFDC
	int checksum;			// $xFDE	
	int reset_vector;
	int map;
} rom_header;

#endif
