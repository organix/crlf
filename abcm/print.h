/*
 * print.h -- console output formatting
 */
#ifndef _PRINT_H_
#define _PRINT_H_

#include "bose.h"

void print(WORD unicode);
void newline();

BYTE parse_print(parse_t * parse, WORD indent);
BYTE value_print(DATA_PTR value, WORD indent);

#endif // _PRINT_H_
