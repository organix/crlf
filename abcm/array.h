/*
 * array.h -- Array (List) operations
 */
#ifndef _ARRAY_H_
#define _ARRAY_H_

#include "bose.h"

/*
    { "kind":"list_get", "at":<number>, "from":<list> }
    { "kind":"list_length", "of":<list> }
    { "kind":"list_add", "value":<expression>, "at":<number>, "to":<list> }
    { "kind":"list_remove", "at":<number>, "from":<list> }

    { "kind":"array_length", "array":<array> }
    { "kind":"array_at", "index":<number>, "array":<array> }
    { "kind":"array_insert", "index":<number>, "value":<expression>, "array":<array> }
*/

BYTE array_count(DATA_PTR array, WORD * count);  // elements in Array
BYTE array_get(DATA_PTR array, WORD offset, DATA_PTR * value);
BYTE array_add(DATA_PTR array, DATA_PTR item, WORD offset, DATA_PTR * new);
BYTE array_concat(DATA_PTR left, DATA_PTR right, DATA_PTR * new);

#endif // _ARRAY_H_
