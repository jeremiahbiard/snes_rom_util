#include <stdlib.h>
#include <string.h>

int str_length(char *string) {
	int i, length = 0;
	for (i = 0; i < 32; i++) {
		if (string[i] == '\0') {
			break;
		} else {
			length++;
		}
	}
	return length;
}

char * int2bin(int i) {
	char *result;
	memset(result, '\0', 32);


	return result;
}
