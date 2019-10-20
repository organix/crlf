/*
 * array.c -- Array (List) operations
 */
//#include <assert.h>

#include "array.h"
#include "bose.h"

#define LOG_ALL // enable all logging
//#define LOG_INFO
//#define LOG_WARN
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
    //DUMP_PARSE("array", &parse);
    if (parse.value == 0) {  // empty array short-cut
        *length = 0;
        LOG_DEBUG("array_length: empty array", *length);
        return true;  // success!
    }
    if (parse.type & T_Counted) {  // assume count is correct
        *length = parse.count;
        LOG_DEBUG("array_length: counted array", *length);
        return true;  // success!
    }
    parse.size = parse.end;  // limit to array contents
    parse.start = parse.end - parse.value;  // reset to start of element data
    WORD n = 0;
    while (parse.start < parse.size) {
        LOG_TRACE("array_length: element start =", parse.start);
        if (!parse_value(&parse)) {
            LOG_WARN("array_length: bad element", parse.start);
            return false;  // bad element
        }
        //DUMP_PARSE("array element", &parse);
        ++n;  // increment item count
        parse.start = parse.end;
    }
    *length = n;
    LOG_DEBUG("array_length: variadic array", *length);
    return true;  // success!
};

BYTE array_get(DATA_PTR array, WORD index, DATA_PTR * value) {
    LOG_TRACE("array_get @", (WORD)array);
    LOG_DEBUG("array_get: index =", index);
    parse_t parse = {
        .base = array,
        .size = MAX_WORD,  // don't know how big array will be
        .start = 0
    };
    if (!parse_array(&parse)) {
        LOG_WARN("array_get: bad array", (WORD)array);
        return false;  // bad array
    }
    //DUMP_PARSE("array", &parse);
    if (parse.value == 0) {  // empty array short-cut
        LOG_WARN("array_get: empty array", (WORD)array);
        return false;  // not found.
    }
    if (parse.type & T_Counted) {  // assume count is correct
        LOG_DEBUG("array_get: counted array", parse.count);
        if (index >= parse.count) {
            LOG_WARN("array_get: index must be less than count", parse.count);
            return false;  // not found.
        }
    }
    parse.size = parse.end;  // limit to array contents
    parse.start = parse.end - parse.value;  // reset to start of element data
    WORD n = 0;
    while (parse.start < parse.size) {
        LOG_TRACE("array_get: element start =", parse.start);
#if 1
        if (n == index) {
            *value = parse.base + parse.start;
            LOG_DEBUG("array_get: found value @", (WORD)*value);
            return true;  // FOUND!
        }
        if (!parse_value(&parse)) {
            LOG_WARN("array_get: bad element", parse.start);
            return false;  // bad element
        }
        //DUMP_PARSE("array element", &parse);
#else
        if (!parse_value(&parse)) {
            LOG_WARN("array_get: bad element", parse.start);
            return false;  // bad element
        }
        DUMP_PARSE("array element", &parse);
        if (n == index) {
            *value = parse.base + parse.start;
            LOG_DEBUG("array_get: found value @", (WORD)*value);
            return true;  // FOUND!
        }
#endif
        ++n;  // increment item count
        parse.start = parse.end;
    }
    LOG_WARN("array_get: index must be less than length", n);
    return false;  // not found.
};
