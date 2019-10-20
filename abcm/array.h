/*
 * array.h -- Array (List) operations
 */
#ifndef _ARRAY_H_
#define _ARRAY_H_

#include "bose.h"

BYTE array_length(DATA_PTR array, WORD * length);
BYTE array_get(DATA_PTR object, WORD index, DATA_PTR * value);

#endif // _ARRAY_H_
