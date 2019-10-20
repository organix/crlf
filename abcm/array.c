/*
 * array.c -- Array (List) operations
 */
//#include <assert.h>

#include "array.h"
#include "bose.h"

//#define LOG_ALL // enable all logging
#define LOG_INFO
#define LOG_WARN
#include "log.h"

/*
    { "kind":"list_get", "at":<number>, "from":<list> }
    { "kind":"list_length", "of":<list> }
    { "kind":"list_add", "value":<expression>, "at":<number>, "to":<list> }
    { "kind":"list_remove", "value":<expression>, "at":<number>, "from":<list> }
*/

BYTE array_length(DATA_PTR array, WORD * length) {
    LOG_TRACE("array_length @", (WORD)array);
    parse_t parse = {
        .base = array,
        .size = MAX_WORD,  // don't know how big array will be
        .start = 0
    };
    if (!parse_array(&parse)) {
        LOG_WARN("array_length: bad array", (WORD)array);
        return false;  // bad array
    }
    if (parse.value == 0) {  // empty array short-cut
        *length = 0;
        return true;  // success!
    }
    if (parse.type | T_Counted) {  // assume count is correct
        *length = parse.count;
        return true;  // success!
    }
    parse.start = parse.end - parse.value;  // reset to start of element data
    WORD n = 0;
    while (parse.start < parse.size) {
        if (!parse_value(&parse)) {
            LOG_WARN("array_length: bad element", parse.start);
            return false;  // bad element
        }
        ++n;  // increment item count
        parse.start = parse.end;
    }
    *length = n;
    return true;  // success!
};

/*
static BYTE array_print(parse_t * parse) {
    // print parsed value known to be an Array
    assert(parse->start < (parse->end - parse->value));
    print('[');
    if (parse->value == 0) {  // empty array short-cut
        print(']');
        return true;
    }
    parse_t item_parse = {
        .base = parse->base + (parse->end - parse->value),  // start of element data
        .size = parse->value,
        .start = 0
    };
    while (item_parse.start < item_parse.size) {
        if (!parse_value(&item_parse)) {
            LOG_WARN("array_print: bad element value", item_parse.start);
            return false;  // bad element value
        }
        if (!parse_print(&item_parse)) return false;  // print failed!
        if (item_parse.end < item_parse.size) {
            print(',');
        }
        item_parse.start = item_parse.end;
    }
    print(']');
    return true;  // success!
}
*/
