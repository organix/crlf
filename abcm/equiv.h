/*
 * equiv.h -- abstract value equivalence
 */
#ifndef _EQUIV_H_
#define _EQUIV_H_

#include "bose.h"

BYTE parse_equiv(parse_t * x_parse, parse_t * y_parse);
BYTE value_equiv(DATA_PTR x, DATA_PTR y);

#endif // _EQUIV_H_
