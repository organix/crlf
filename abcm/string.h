/*
 * string.h -- String operations
 */
#ifndef _STRING_H_
#define _STRING_H_

#include "bose.h"
#include "sponsor.h"

/*
    { "kind":"expr_operation", "name":"length[1]", "args":[ <string> ] }
    { "kind":"expr_operation", "name":"charAt_FROM_START[2]", "args":[ <string>, <number> ] }
*/

BYTE string_from(DATA_PTR value, DATA_PTR * result);  // convert arbitrary value to String
BYTE string_count(DATA_PTR string, WORD * count);  // codepoints in String
BYTE string_get(DATA_PTR string, WORD offset, WORD * codepoint);
BYTE string_add(DATA_PTR string, WORD codepoint, WORD offset, DATA_PTR * new);
BYTE string_concat(DATA_PTR left, DATA_PTR right, DATA_PTR * new);

#endif // _STRING_H_
