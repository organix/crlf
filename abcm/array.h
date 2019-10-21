/*
 * array.h -- Array (List) operations
 */
#ifndef _ARRAY_H_
#define _ARRAY_H_

#include "bose.h"
#include "sponsor.h"

/*
    { "kind":"list_get", "at":<number>, "from":<list> }
    { "kind":"list_length", "of":<list> }
    { "kind":"list_add", "value":<expression>, "at":<number>, "to":<list> }
    { "kind":"list_remove", "at":<number>, "from":<list> }
*/

BYTE array_length(DATA_PTR array, WORD * length);
BYTE array_get(DATA_PTR array, WORD index, DATA_PTR * value);
BYTE array_add(sponsor_t * sponsor, DATA_PTR array, DATA_PTR item, WORD index, DATA_PTR * new);

#endif // _ARRAY_H_
