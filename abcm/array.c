/*
 * array.c -- Array (List) operations
 */
#include <string.h>  // for memcpy, et. al.
//#include <assert.h>

#include "array.h"
#include "bose.h"
#include "sponsor.h"

//#define LOG_ALL // enable all logging
#define LOG_INFO
#define LOG_WARN
#include "log.h"

BYTE array_count(DATA_PTR array, WORD * count) {
    LOG_TRACE("array_count @", (WORD)array);
    parse_t parse = {
        .base = array,
        .size = MAX_WORD,  // don't know how big array will be
        .start = 0
    };
    if (!parse_array(&parse)) {
        LOG_WARN("array_count: bad array", (WORD)array);
        return false;  // bad array
    }
    //DUMP_PARSE("array", &parse);
    if (parse.value == 0) {  // empty array short-cut
        *count = 0;
        LOG_DEBUG("array_count: empty array", *count);
        return true;  // success!
    }
    if (parse.type & T_Counted) {  // assume count is correct
        *count = parse.count;
        LOG_DEBUG("array_count: counted array", *count);
        return true;  // success!
    }
    parse.size = parse.end;  // limit to array contents
    parse.start = parse.end - parse.value;  // reset to start of element data
    WORD n = 0;
    while (parse.start < parse.size) {
        LOG_TRACE("array_count: element start =", parse.start);
        if (!parse_value(&parse)) {
            LOG_WARN("array_count: bad element", parse.start);
            return false;  // bad element
        }
        //DUMP_PARSE("array element", &parse);
        ++n;  // increment item count
        parse.start = parse.end;
    }
    *count = n;
    LOG_DEBUG("array_count: variadic array", *count);
    return true;  // success!
};

BYTE array_get(DATA_PTR array, WORD offset, DATA_PTR * value) {
    LOG_TRACE("array_get @", (WORD)array);
    LOG_DEBUG("array_get: offset =", offset);
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
        if (offset >= parse.count) {
            LOG_WARN("array_get: offset must be less than count", parse.count);
            return false;  // not found.
        }
    }
    parse.size = parse.end;  // limit to array contents
    parse.start = parse.end - parse.value;  // reset to start of element data
    WORD n = 0;
    while (parse.start < parse.size) {
        LOG_TRACE("array_get: element start =", parse.start);
#if 1
        if (n == offset) {
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
        if (n == offset) {
            *value = parse.base + parse.start;
            LOG_DEBUG("array_get: found value @", (WORD)*value);
            return true;  // FOUND!
        }
#endif
        ++n;  // increment item count
        parse.start = parse.end;
    }
    LOG_WARN("array_get: offset must be less than count", n);
    return false;  // not found.
};

/*
ADD x AT 0 TO [a, b, c] --> [x, a, b, c]
ADD x AT 1 TO [a, b, c] --> [a, x, b, c]
ADD x AT 2 TO [a, b, c] --> [a, b, x, c]
ADD x AT 3 TO [a, b, c] --> [a, b, c, x]
*/
BYTE array_add(DATA_PTR array, DATA_PTR item, WORD offset, DATA_PTR * new) {
    LOG_TRACE("array_add @", (WORD)array);
    LOG_DEBUG("array_add: item @", (WORD)item);
    LOG_DEBUG("array_add: offset =", offset);
    parse_t parse = {
        .base = array,
        .size = MAX_WORD,  // don't know how big array will be
        .start = 0
    };
    if (!parse_array(&parse)) {
        LOG_WARN("array_add: bad array", (WORD)array);
        return false;  // bad array
    }
    //DUMP_PARSE("array", &parse);
    parse_t item_parse = {
        .base = item,
        .size = MAX_WORD,  // don't know how big item will be
        .start = 0
    };
    if (!parse_value(&item_parse)) {
        LOG_WARN("array_add: bad item value", (WORD)item);
        return false;  // bad item value
    }
    //DUMP_PARSE("item", &item_parse);
    if (parse.type & T_Counted) {  // assume count is correct
        LOG_DEBUG("array_add: counted array", parse.count);
        if (offset > parse.count) {
            LOG_WARN("array_add: offset exceeds count", parse.count);
            return false;  // not found.
        }
    }

    /* allocate space for new array */
    DATA_PTR data;
    WORD size = 8;  // margin for size/count growth
    size += (parse.end - parse.start);  // plus size of array
    size += (item_parse.end - item_parse.start);  // plus size of item
    LOG_TRACE("array_add: allocation size =", size);
    if (size > 0xFFFF) {  // FIXME: this should be a configurable limit somewhere, but code below depends on it...
        LOG_WARN("array_add: array too large", size);
        return false;  // array too large!
    }
    if (!RESERVE(&data, size)) return false;  // out of memory!
    WORD end = 0;
    data[end++] = array_n;   // 0: counted array
    data[end++] = p_int_0;   // 1: size field
    data[end++] = n_2;       // 2:   2 bytes
    data[end++] = 0;         // 3:   size (LSB)
    data[end++] = 0;         // 4:   size (MSB)
    data[end++] = p_int_0;   // 5: count field
    data[end++] = n_2;       // 6:   2 bytes
    data[end++] = 0;         // 7:   count (LSB)
    data[end++] = 0;         // 8:   count (MSB)

    /* copy elements before offset */
    parse.size = parse.end;  // limit to array contents
    parse.start = parse.end - parse.value;  // reset to start of element data
    WORD n = 0;
    while ((n < offset) && (parse.start < parse.size)) {
        LOG_TRACE("array_add: element start =", parse.start);
        if (!parse_value(&parse)) {
            LOG_WARN("array_add: bad element", parse.start);
            RELEASE(&data);  // free memory on failure
            return false;  // bad element
        }
        //DUMP_PARSE("array element", &parse);
        size = parse.end - parse.start;
        LOG_TRACE("array_add: element size =", size);
        memcpy(data + end, parse.base + parse.start, size);
        end += size;
        ++n;  // increment item count
        parse.start = parse.end;
    }

    /* copy item at offset */
    size = item_parse.end - item_parse.start;
    LOG_TRACE("array_add: item size =", size);
    memcpy(data + end, item_parse.base + item_parse.start, size);
    end += size;

    /* copy elements after offset */
    while (parse.start < parse.size) {
        LOG_TRACE("array_add: element start =", parse.start);
        if (!parse_value(&parse)) {
            LOG_WARN("array_add: bad element", parse.start);
            RELEASE(&data);  // free memory on failure
            return false;  // bad element
        }
        //DUMP_PARSE("array element", &parse);
        size = parse.end - parse.start;
        LOG_TRACE("array_add: element size =", size);
        memcpy(data + end, parse.base + parse.start, size);
        end += size;
        ++n;  // increment item count
        parse.start = parse.end;
    }

    /* fill in size/count and return new array */
    if (offset > n) {
        LOG_WARN("array_add: offset exceeds count", n);
        RELEASE(&data);  // free memory on failure
        return false;  // not found.
    }
    LOG_TRACE("array_add: final end =", end);
    size = end - 5;  // subtract header byte count
    LOG_TRACE("array_add: final size =", size);
    ++n;  // count new item
    LOG_TRACE("array_add: final count =", n);
    data[3] = size & 0xFF;  // size (LSB)
    data[4] = size >> 8;    // size (MSB)
    data[7] = n & 0xFF;     // count (LSB)
    data[8] = n >> 8;       // count (MSB)
    *new = data;
    LOG_DEBUG("array_add: new array @", (WORD)data);
    return true;  // success!
};
