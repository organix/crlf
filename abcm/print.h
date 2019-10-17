/*
 * print.h -- console output formatting
 */
#ifndef _PRINT_H_
#define _PRINT_H_

#include "bose.h"

void print(WORD unicode);
void newline();

BYTE parse_print(parse_t * parse);
BYTE value_print(DATA_PTR value);

#endif // _PRINT_H_
