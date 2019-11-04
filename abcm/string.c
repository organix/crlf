/*
 * string.h -- String operations
 */
#include <string.h>  // for memcpy, et. al.
//#include <assert.h>

#include "string.h"
#include "bose.h"
#include "sponsor.h"

#define LOG_ALL // enable all logging
//#define LOG_INFO
//#define LOG_WARN
#include "log.h"

BYTE string_count(DATA_PTR string, WORD * count) {
    LOG_TRACE("string_count @", (WORD)string);
    parse_t parse;
    if (!string_parse(string, &parse)) {
        LOG_WARN("string_count: bad string", (WORD)string);
        return false;  // bad string
    }
    if (parse.prefix == octets) {
        *count = parse.size;
        LOG_DEBUG("string_count: binary octets", *count);
        return true;  // success! -- early exit for binary octet data
    }
    WORD n = 0;
    while (parse.start < parse.size) {
        if (!parse_codepoint(&parse)) {
            LOG_WARN("string_count: bad codepoint", parse.start);
            return false;  // bad codepoint
        }
        ++n;  // increment count
        parse.start = parse.end;
    }
    *count = n;
    LOG_DEBUG("string_count: encoded string", *count);
    return true;  // success!
};

BYTE string_get(DATA_PTR string, WORD offset, WORD * codepoint) {
    LOG_TRACE("string_get @", (WORD)string);
    LOG_DEBUG("string_get: offset =", offset);
    parse_t parse;
    if (!string_parse(string, &parse)) {
        LOG_WARN("string_get: bad string", (WORD)string);
        return false;  // bad string
    }
    if ((parse.prefix == octets) && (offset < parse.size)) {
        *codepoint = parse.base[parse.start + offset];  // start should be 0, but...
        LOG_DEBUG("string_get: found octet", *codepoint);
        return true;  // success! -- early exit for binary octet data
    }
    WORD n = 0;
    while (parse.start < parse.size) {
        if (!parse_codepoint(&parse)) {
            LOG_WARN("string_get: bad codepoint", parse.start);
            return false;  // bad codepoint
        }
        if (n == offset) {
            *codepoint = parse.value;
            LOG_DEBUG("string_get: found codepoint", *codepoint);
            return true;  // FOUND!
        }
        ++n;  // increment count
        parse.start = parse.end;
    }
    LOG_WARN("string_get: offset must be less than count", n);
    return false;  // not found.
};

BYTE string_add(sponsor_t * sponsor, DATA_PTR string, WORD codepoint, WORD offset, DATA_PTR * new) {
    LOG_TRACE("string_add @", (WORD)string);
    LOG_DEBUG("string_add: codepoint =", codepoint);
    LOG_DEBUG("string_add: offset =", offset);
#if 0
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
        if (index > parse.count) {
            LOG_WARN("array_add: index exceeds count", parse.count);
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
    WORD offset = 0;
    data[offset++] = array_n;   // 0: counted array
    data[offset++] = p_int_0;   // 1: size field
    data[offset++] = n_2;       // 2:   2 bytes
    data[offset++] = 0;         // 3:   size (LSB)
    data[offset++] = 0;         // 4:   size (MSB)
    data[offset++] = p_int_0;   // 5: count field
    data[offset++] = n_2;       // 6:   2 bytes
    data[offset++] = 0;         // 7:   count (LSB)
    data[offset++] = 0;         // 8:   count (MSB)

    /* copy elements before index */
    parse.size = parse.end;  // limit to array contents
    parse.start = parse.end - parse.value;  // reset to start of element data
    WORD n = 0;
    while ((n < index) && (parse.start < parse.size)) {
        LOG_TRACE("array_add: element start =", parse.start);
        if (!parse_value(&parse)) {
            LOG_WARN("array_add: bad element", parse.start);
            RELEASE(&data);  // free memory on failure
            return false;  // bad element
        }
        //DUMP_PARSE("array element", &parse);
        size = parse.end - parse.start;
        LOG_TRACE("array_add: element size =", size);
        memcpy(data + offset, parse.base + parse.start, size);
        offset += size;
        ++n;  // increment item count
        parse.start = parse.end;
    }

    /* copy item at index */
    size = item_parse.end - item_parse.start;
    LOG_TRACE("array_add: item size =", size);
    memcpy(data + offset, item_parse.base + item_parse.start, size);
    offset += size;

    /* copy elements after index */
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
        memcpy(data + offset, parse.base + parse.start, size);
        offset += size;
        ++n;  // increment item count
        parse.start = parse.end;
    }

    /* fill in size/count and return new array */
    if (index > n) {
        LOG_WARN("array_add: index exceeds count", n);
        RELEASE(&data);  // free memory on failure
        return false;  // not found.
    }
    LOG_TRACE("array_add: final offset =", offset);
    size = offset - 5;  // subtract header byte count
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
#else
    return false;  // NOT IMPLEMENTED!
#endif
};
