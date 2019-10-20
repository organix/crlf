/*
 * object.h -- Object (Dictionary) operations
 */
#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "bose.h"

BYTE object_has(DATA_PTR object, DATA_PTR name);
BYTE object_get(DATA_PTR object, DATA_PTR name, DATA_PTR * value);

#endif // _OBJECT_H_
