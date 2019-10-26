/*
 * print.h -- console output formatting
 */
#ifndef _PRINT_H_
#define _PRINT_H_

#include "bose.h"

void print(WORD unicode);
void prints(char * cstring);
void newline();

void hex_print(BYTE b);
void hex_dump(DATA_PTR data, WORD size);
void data_dump(DATA_PTR data, WORD size);

BYTE parse_print(parse_t * parse, WORD indent);
BYTE value_print(DATA_PTR value, WORD indent);

#endif // _PRINT_H_
