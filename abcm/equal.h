/*
 * equal.h -- equality comparisons
 */
#ifndef _EQUAL_H_
#define _EQUAL_H_

#include "abcm.h"

BYTE parse_equal(parse_t * x_parse, parse_t * y_parse);
BYTE value_equal(DATA_PTR x, DATA_PTR y);

#endif // _EQUAL_H_
