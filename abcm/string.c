/*
 * string.h -- String operations
 */
#include <string.h>  // for memcpy, et. al.
#include <assert.h>

#include "string.h"
#include "bose.h"
#include "sponsor.h"

#define LOG_ALL // enable all logging
//#define LOG_INFO
//#define LOG_WARN
#include "log.h"

BYTE s_null[] = { utf8, n_4, 'n', 'u', 'l', 'l' };
BYTE s_true[] = { utf8, n_4, 't', 'r', 'u', 'e' };
BYTE s_false[] = { utf8, n_5, 'f', 'a', 'l', 's', 'e' };
BYTE s_0[] = { utf8, n_1, '0' };

static BYTE scratch[48];  // scratch buffer for formatting
static char * hex = "0123456789abcdef";

BYTE string_from(DATA_PTR value, DATA_PTR * result) {
/*        1         2         3         4      .  5         6
0123456789012345678901234567890123456789012345678901234567890123
SN<xx xx xx xx xx xx xx xx xx ...>
*/
    parse_t parse;
    if (!value_parse(value, &parse)) return false;  // parse failed!
    if (parse.prefix == mem_ref) {  // redirect to memo table entry
        value = (DATA_PTR)parse.count;
        if (!value_parse(value, &parse)) return false;  // bad memo!
    }
    switch (parse.type & T_Base) {
        case T_Null: {
            *result = s_null;
            return true;  // hard-coded value
        }
        case T_Boolean: {
            *result = (parse.prefix ? s_true : s_false);
            return true;  // hard-coded value
        }
        case T_Number: {
            WORD n;
            if (value_integer(value, &n)) {
                // value fits in a WORD
                if (n == 0) {
                    *result = s_0;
                    return true;  // hard-coded value
                }
                if (parse.type & T_Negative) {
                    n = -n;  // absolute magnitude
                }
                DATA_PTR p = scratch + sizeof(scratch);  // start past end of scratch buffer
                while (n) {
                    *--p = '0' + (n % 10);  // base-10 digit
                    n /= 10;
                }
                if (parse.type & T_Negative) {
                    *--p = '-';  // sign
                }
                WORD size = (scratch + sizeof(scratch)) - p;
                *--p = n_0 + size;  // string size field
                *--p = utf8;
                if (!COPY(result, p)) return false;  // allocation failure!
                return true;  // base-10 integer
            }
            break;  // fall-thru to hex dump...
        }
        case T_String: {
            *result = value;
            return true;  // no conversion needed
        }
/*
        case T_Array: {
            break;  // fall-thru to hex dump...
        }
        case T_Object: {
            break;  // fall-thru to hex dump...
        }
*/
    }

    /* if all else fails, dump encoded value in hex... */
    DATA_PTR p = scratch;
    *p++ = utf8;
    *p++ = n_0;  // size will be filled in later...
    *p++ = '<';
    WORD i = parse.start;
    while (i < parse.end) {
        if (i > 0) {
            *p++ = ' ';
        }
        if (i > 8) {  // size cut-off
            *p++ = '.';
            *p++ = '.';
            *p++ = '.';
            break;
        }
        BYTE b = parse.base[i++];
        *p++ = hex[(b >> 4) & 0xF];
        *p++ = hex[b & 0xF];
    }
    *p++ = '>';
    scratch[1] = n_m2 + (p - scratch);  // string size field
    if (!COPY(result, scratch)) return false;  // allocation failure!
    return true;  // encoded value dump
}

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

static WORD encode_codepoint(DATA_PTR data, WORD offset, parse_t * parse) {
    WORD u = parse->value;
    assert(u <= MAX_UNICODE);
    switch (parse->prefix) {
        case octets: {
            // each byte is an untranslated codepoint in the range 0x00..0xFF
            assert(u <= 0xFF);
            data[offset++] = u;
            break;
        }
        case utf8: {
            if (u <= 0x7F) {
                data[offset++] = u;
            } else if (u <= 0x07FF) {
                data[offset++] = 0xC0 | ((u >> 6) & 0x1F);
                data[offset++] = 0x80 | (u & 0x3F);
            } else if (u <= 0xFFFF) {
                data[offset++] = 0xE0 | ((u >> 12) & 0x0F);
                data[offset++] = 0x80 | ((u >> 6) & 0x3F);
                data[offset++] = 0x80 | (u & 0x3F);
            } else {
                data[offset++] = 0xF0 | ((u >> 18) & 0x07);
                data[offset++] = 0x80 | ((u >> 12) & 0x3F);
                data[offset++] = 0x80 | ((u >> 6) & 0x3F);
                data[offset++] = 0x80 | (u & 0x3F);
            }
            break;
        }
        case utf16: {
            if (u <= 0xFFFF) {
                if (parse->type & T_Negative) {  // LE
                    data[offset++] = (u & 0xFF);
                    data[offset++] = (u >> 8);
                } else {  // BE
                    data[offset++] = (u >> 8);
                    data[offset++] = (u & 0xFF);
                }
            } else {  // surrogate pairs
                u -= 0x10000;
                if (parse->type & T_Negative) {  // LE
                    data[offset++] = (u & 0xFF);
                    data[offset++] = 0xDC | ((u >> 8) & 0x03);
                    data[offset++] = (u >> 10);
                    data[offset++] = 0xD8 | ((u >> 18) & 0x03);
                } else {  // BE
                    data[offset++] = 0xD8 | ((u >> 18) & 0x03);
                    data[offset++] = (u >> 10);
                    data[offset++] = 0xDC | ((u >> 8) & 0x03);
                    data[offset++] = (u & 0xFF);
                }
            }
            break;
        }
        default: {
            LOG_WARN("encode_codepoint: unsupported string prefix!", parse->prefix);
            break;
        }
    }
    return offset;
}
BYTE string_add(DATA_PTR string, WORD codepoint, WORD offset, DATA_PTR * new) {
    LOG_TRACE("string_add @", (WORD)string);
    LOG_DEBUG("string_add: codepoint =", codepoint);
    LOG_DEBUG("string_add: offset =", offset);
    parse_t parse;
    if (!string_parse(string, &parse)) {
        LOG_WARN("string_add: bad string", (WORD)string);
        return false;  // bad string
    }
    if (parse.type & T_Capability) return false;  // can't inject data into a Capability

    /* allocate space for new string */
    DATA_PTR data;
    WORD size = 8;  // margin for full string header with length and BOM
    size += parse.size;
    LOG_TRACE("string_add: allocation size =", size);
    if (size > 0xFFFF) {  // FIXME: this should be a configurable limit somewhere, but code below depends on it...
        LOG_WARN("string_add: string too large", size);
        return false;  // string too large!
    }
    if (!RESERVE(&data, size)) return false;  // out of memory!
    WORD i = 0;  // data encoding offset
    data[i++] = parse.prefix;   // 0: string prefix
    data[i++] = p_int_0;        // 1: size field
    data[i++] = n_2;            // 2:   2 bytes
    data[i++] = 0;              // 3:   size (LSB)
    data[i++] = 0;              // 4:   size (MSB)
    if (parse.prefix == utf8) {
        data[i++] = 0xEF;  // BOM[0]
        data[i++] = 0xBB;  // BOM[1]
        data[i++] = 0xBF;  // BOM[2]
    } else if (parse.prefix == utf16) {
        if (parse.type & T_Negative) {
            data[i++] = 0xFF;  // BOM-LSB
            data[i++] = 0xFE;  // BOM-MSB
        } else {
            data[i++] = 0xFE;  // BOM-MSB
            data[i++] = 0xFF;  // BOM-LSB
        }
    }

    /* copy codepoints, injecting new value at `offset` */
    WORD n = 0;  // codepoint offset
    while (parse.start < parse.size) {
        if (n == offset) {
            parse.value = codepoint;
            i = encode_codepoint(data, i, &parse);
        }
        if (!parse_codepoint(&parse)) return false;  // bad codepoint
        i = encode_codepoint(data, i, &parse);
        ++n;
        parse.start = parse.end;
    }
    if (n == offset) {
        parse.value = codepoint;
        i = encode_codepoint(data, i, &parse);
    }

    /* fill in size and return new string */
    if (n < offset) {
        LOG_WARN("string_add: index exceeds count", n);
        RELEASE(&data);  // free memory on failure
        return false;  // not found.
    }
    LOG_TRACE("string_add: final offset =", i);
    size = i - 5;  // subtract header byte count
    LOG_TRACE("string_add: final size =", size);
    data[3] = size & 0xFF;  // size (LSB)
    data[4] = size >> 8;    // size (MSB)
    *new = data;
    LOG_DEBUG("string_add: new string @", (WORD)data);
    return true;  // success!
};

static WORD encode_size(DATA_PTR data, WORD offset, WORD size) {
    if (size < 127) {
        data[offset++] = n_0 + size;    // directly-coded small integer
    } else {
        assert(size <= 0xFFFF);
        data[offset++] = p_int_0;       // positive integer
        data[offset++] = n_2;           //   2 bytes
        data[offset++] = size & 0xFF;   //   size (LSB)
        data[offset++] = size >> 8;     //   size (MSB)
    }
    return offset;
}
BYTE string_concat(DATA_PTR left, DATA_PTR right, DATA_PTR * new) {
    LOG_TRACE("string_concat: left @", (WORD)left);
    LOG_TRACE("string_concat: right @", (WORD)right);
    parse_t left_parse;
    if (!string_parse(left, &left_parse)) {
        LOG_WARN("string_concat: bad left string", (WORD)left);
        return false;  // bad left string
    }
    IF_TRACE({
        DUMP_PARSE("left", &left_parse);
    });
    parse_t right_parse;
    if (!string_parse(right, &right_parse)) {
        LOG_WARN("string_concat: bad right string", (WORD)right);
        return false;  // bad right string
    }
    IF_TRACE({
        DUMP_PARSE("right", &right_parse);
    });

    /* handle special cases (e.g.: empty strings) */
    if (left_parse.prefix == string_0) {
        return COPY(new, right);
    }
    if (right_parse.prefix == string_0) {
        return COPY(new, left);
    }
    if (left_parse.type != right_parse.type) {
        // FIXME: we could get fancier with transcoding, but it would be significantly more complicated...
        LOG_WARN("string_concat: type/encoding mismatch!", (WORD)(left_parse.type ^ right_parse.type));
        return false;  // type/encoding mismatch!
    }
    WORD size = left_parse.size + right_parse.size;  // total content size

    /* allocate space for new string */
    DATA_PTR data;
    size += 8;  // margin for full string header with length and BOM
    LOG_TRACE("string_concat: allocation size =", size);
    if (size > 0xFFFF) {  // FIXME: this should be a configurable limit somewhere, but code below depends on it...
        LOG_WARN("string_concat: string too large", size);
        return false;  // string too large!
    }
    if (!RESERVE(&data, size)) return false;  // out of memory!
    size -= 8;  // revert to content size
    WORD offset = 0;
    data[offset++] = left_parse.prefix;  // string prefix

    /* encode size & insert markers */
    if (left_parse.prefix == octets) {
        if (left_parse.type & T_Capability) {
            size += 1;  // adjust for CAP-mark
            offset = encode_size(data, offset, size);
            data[offset++] = 0x10;  // CAP-mark
        } else {
            offset = encode_size(data, offset, size);
        }
    } else if (left_parse.prefix == utf8) {
        size += 3;  // adjust for BOM
        offset = encode_size(data, offset, size);
        data[offset++] = 0xEF;  // BOM[0]
        data[offset++] = 0xBB;  // BOM[1]
        data[offset++] = 0xBF;  // BOM[2]
    } else if (left_parse.prefix == utf16) {
        size += 2;  // adjust for BOM
        offset = encode_size(data, offset, size);
        if (left_parse.type & T_Negative) {
            data[offset++] = 0xFF;  // BOM-LSB
            data[offset++] = 0xFE;  // BOM-MSB
        } else {
            data[offset++] = 0xFE;  // BOM-MSB
            data[offset++] = 0xFF;  // BOM-LSB
        }
    } else {
        LOG_WARN("string_concat: unsupported string prefix!", left_parse.prefix);
        return false;  // unsupported string prefix!
    }

    /* copy codepoints from left */
    LOG_TRACE("string_concat: left size =", left_parse.size);
    memcpy(data + offset, left_parse.base, left_parse.size);
    offset += left_parse.size;

    /* copy codepoints from right */
    LOG_TRACE("string_concat: right size =", right_parse.size);
    memcpy(data + offset, right_parse.base, right_parse.size);
    offset += right_parse.size;

    /* return new string */
    LOG_TRACE("string_concat: final offset =", offset);
    LOG_TRACE("string_concat: final size =", size);
    *new = data;
    LOG_DEBUG("string_concat: new string @", (WORD)data);
    return true;  // success!
};
