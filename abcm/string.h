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

BYTE string_length(DATA_PTR string, WORD * length);
BYTE string_get(DATA_PTR string, WORD offset, WORD * codepoint);
BYTE string_add(sponsor_t * sponsor, DATA_PTR string, WORD codepoint, WORD offset, DATA_PTR * new);

#endif // _STRING_H_
