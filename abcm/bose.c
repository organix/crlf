/*
 * bose.c -- Binary Octet-Stream Encoding
 */
//#include <stdlib.h>
#include <assert.h>

#include "bose.h"
#include "equiv.h"
#include "sponsor.h"
#include "print.h"

//#define LOG_ALL // enable all logging
#define LOG_INFO
#define LOG_WARN
#include "log.h"

type_t prefix_type[1<<8] = {
    T_Boolean, T_Boolean, T_Array, T_Object, T_Array|T_Sized, T_Object|T_Sized, T_Array|T_Sized|T_Counted, T_Object|T_Sized|T_Counted,
    T_String|T_Sized, T_String, T_String|T_Sized, T_String|T_Sized|T_Exact, T_String|T_Sized, T_String|T_Sized|T_Exact, T_String|T_Sized|T_Counted, T_String,
    T_Integer, T_Integer, T_Integer, T_Integer, T_Integer, T_Integer, T_Integer, T_Integer,
    T_Integer|T_Negative, T_Integer|T_Negative, T_Integer|T_Negative, T_Integer|T_Negative, T_Integer|T_Negative, T_Integer|T_Negative, T_Integer|T_Negative, T_Integer|T_Negative,
    T_Float, T_Float, T_Float, T_Float, T_Float, T_Float, T_Float, T_Float,
    T_Float|T_Negative, T_Float|T_Negative, T_Float|T_Negative, T_Float|T_Negative, T_Float|T_Negative, T_Float|T_Negative, T_Float|T_Negative, T_Float|T_Negative,
    T_Range, T_Range, T_Range, T_Range, T_Range, T_Range, T_Range, T_Range,
    T_Range|T_Negative, T_Range|T_Negative, T_Range|T_Negative, T_Range|T_Negative, T_Range|T_Negative, T_Range|T_Negative, T_Range|T_Negative, T_Range|T_Negative,
    T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative,
    T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative,
    T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative,
    T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative,
    T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative,
    T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative,
    T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative,
    T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative, T_Small|T_Negative,
    T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small,
    T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small,
    T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small,
    T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small,
    T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small,
    T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small,
    T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small,
    T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small,
    T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small,
    T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small,
    T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small,
    T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small,
    T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small,
    T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small,
    T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small,
    T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Small, T_Null
};

BYTE s_type_name[][10] = {
    { utf8, n_4, 'N', 'u', 'l', 'l' },
    { utf8, n_7, 'B', 'o', 'o', 'l', 'e', 'a', 'n' },
    { utf8, n_6, 'N', 'u', 'm', 'b', 'e', 'r' },
    { utf8, n_6, 'S', 't', 'r', 'i', 'n', 'g' },
    { utf8, n_5, 'A', 'r', 'r', 'a', 'y' },
    { utf8, n_6, 'O', 'b', 'j', 'e', 'c', 't' },
    { utf8, n_2, '-', '-' }  // unused/invalid
};

BYTE v_null[] = { null };  // null value (encoded)
BYTE b_true[] = { true };  // true value (encoded)
BYTE b_false[] = { false };  // false value (encoded)
BYTE i_0[] = { n_0 };  // integer zero (encoded)
BYTE s_[] = { string_0 };  // empty string (encoded)
BYTE a_[] = { array_0 };  // empty array (encoded)
BYTE o_[] = { object_0 };  // empty object (encoded)

/*
 * BOSE String memoization
 */
static DATA_PTR memo_table[1<<8] = {};
static BYTE memo_index = 0;  // index of next memo slot to use
BYTE memo_freeze = false;  // stop accepting memo table additions

BYTE memo_reset() {  // reset memo table between top-level values
    LOG_DEBUG("memo_reset: index", memo_index);
    memo_freeze = false;
    memo_index = 0;
    do {
        memo_table[memo_index] = s_;  // initialize with safe empty-string
    } while (++memo_index);  // stop when we wrap-around to 0
    return true;  // success!
};

DATA_PTR memo_get(BYTE index) {
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "memo_get: index", index);
    return memo_table[index];
}

BYTE memo_add(parse_t * parse) {
    LOG_DEBUG("memo_add: index", memo_index);
    if (memo_freeze) {
        LOG_WARN("parse_value: attempt to memoize after freeze", memo_index);
        return false;  // don't call memo_add if memo_freeze is in effect...
    }
    IF_TRACE(WORD size = parse->end - parse->start);
    LOG_TRACE("memo_add: size", size);
    parse->base[parse->start] &= 0x0E;  // WARNING: THIS CLEARS THE MEMOIZE FLAG IN-PLACE!
    memo_table[memo_index] = parse->base + parse->start;  // point directly to persistent data...
    IF_TRACE(memo_print(memo_table[memo_index], size));
    if (++memo_index == 0) {  // index wrap-around
        memo_freeze = true;
    }
    return true;  // success!
};

/**
Input:
    parse = <parse structure to populate>
    data = <pointer to octet buffer prefix byte>
Output:
    parse->base = <pointer to octet buffer data>
    parse->size = <size of octet buffer in bytes>
    parse->start = 0
    parse->end = 0
    parse->prefix = octets
    parse->type = prefix_type[octets]  // (T_String|T_Sized)
    parse->value = <input `data` value>
**/
BYTE parse_from_data(parse_t * parse, DATA_PTR data) {
    // initialize parse bounds to read from byte buffer
    if (data[0] != octets) {
        LOG_WARN("parse_from_data: octet data required!", data[0]);
        return false;  // data container must be octets
    }
    parse->prefix = data[0];
    parse->type = prefix_type[parse->prefix];
    parse_t size_parse = {
        .base = data + 1,
        .size = sizeof(WORD),
        .start = 0
    };
    // find out how big it is
    if (parse_integer(&size_parse)) {
        parse->base = size_parse.base + size_parse.end;
        parse->size = size_parse.value;
        parse->start = 0;
        parse->end = 0;
        parse->value = (WORD)data;  // ...in case we need the source pointer
        return true;  // success
    }
    LOG_WARN("parse_from_data: bad size", size_parse.value);
    return false;  // bad size
}

/**
Input:
    parse->base = <pointer to data byte(s)>
    parse->size = <size of source buffer in bytes>
    parse->start = <offset from base to prefix byte>
Output:
    parse->end = <offset to data following prefix>
    parse->prefix = <value of prefix byte>
    parse->type = <value type info>
**/
BYTE parse_prefix(parse_t * parse) {
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "parse_prefix: base", (WORD)parse->base);
    if (!parse->base) {
        LOG_WARN("parse_prefix: NULL base pointer!", (WORD)parse);
        return false;  // NULL base pointer!
    }
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "parse_prefix: start", parse->start);
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "parse_prefix: size", parse->size);
/*
    print('^');
    hex_dump(parse->base + parse->start, 8);
    print('\n');
*/
    if (parse->start >= parse->size) {
        LOG_WARN("parse_prefix: out-of-bounds!", parse->size);
        return false;  // out of bounds!
    }
    parse->end = parse->start;
    parse->prefix = parse->base[parse->end++];
    parse->type = prefix_type[parse->prefix];
    LOG_TRACE("parse_prefix: prefix", parse->prefix);
    return true;
}

/**
Input:
    parse->base = <pointer to data byte(s)>
    parse->size = <size of source buffer in bytes>
    parse->start = <offset from base to prefix byte>
Output:
    parse->end = <offset past last byte of extended data>
    parse->prefix = <value of prefix byte>
    parse->type = <value type info>
    parse->value = <size of extended data (if T_Sized), or memo index, or Small Integer value>
    parse->count = <value of count field (if T_Counted)>, or memo_table[index]
**/
BYTE parse_value(parse_t * parse) {
    LOG_TRACE("parse_value @", (WORD)parse);
    parse->value = 0;  // default value/size
    parse->count = 0;  // default count
    if (!parse_prefix(parse)) return false;  // bad prefix!
    if (parse->prefix == mem_ref) {
        if (parse->end >= parse->size) {
            LOG_WARN("parse_value: out-of-bounds", parse->size);
            return false;  // out of bounds
        }
        BYTE index = parse->base[parse->end++];
        parse->value = index;
        parse->count = (WORD)memo_get(index);
        LOG_DEBUG("parse_value: memoized String", parse->value);
    } else if (parse->type & T_Sized) {
        // extended Value
        parse_t ext_parse = {
            .base = parse->base,
            .size = parse->size,
            .start = parse->end
        };
        // parse size field
        if (!parse_integer(&ext_parse)) {
            LOG_WARN("parse_value: bad size", ext_parse.value);
            return false;  // bad size
        }
        WORD start = ext_parse.end;
        LOG_TRACE("parse_value: start =", start);
        WORD size = ext_parse.value;
        parse->end = start + size;  // advance to end of Value
        LOG_TRACE("parse_value: end =", parse->end);
        parse->value = size;  // value is the size
        if (parse->end > parse->size) {
            LOG_WARN("parse_value: not enough data!", size);
            return false;  // not enough data!
        }
        if (parse->type & T_Counted) {
            // parse count field
            ext_parse.start = ext_parse.end;
            if (parse->prefix == s_encoded) {
                // FIXME: parse encoding String
                LOG_WARN("parse_value: unsupported encoding!", (WORD)(ext_parse.base + ext_parse.start));
                return false;  // unsupported encoding!
            }
            if (!parse_integer(&ext_parse)) {
                LOG_WARN("parse_value: bad count", ext_parse.value);
                return false;  // bad count
            }
            parse->count = ext_parse.value;
            LOG_TRACE("parse_value: count =", parse->count);
            WORD skip = ext_parse.end - ext_parse.start;
            parse->value -= skip;  // remove count field from size
        }
        if (parse->prefix == octets) {
            LOG_TRACE("parse_value: octets", parse->value);
            // check for capability-mark
            if (parse->value >= 2) {  // require at least 2 bytes
                if (parse->base[start] == 0x10) {
                    LOG_TRACE("parse_value: skip CAP-mark", start);
                    parse->value -= 1;  // remove CAP-mark from size
                    parse->type |= T_Capability;  // set Capability flag
                }
            }
        } else if ((parse->prefix == utf8) || (parse->prefix == utf8_mem)) {
            LOG_TRACE("parse_value: utf8", parse->value);
            // check for byte-order-mark
            if (parse->value >= 3) {  // require at least 3 bytes
                if ((parse->base[start] == 0xEF)
                &&  (parse->base[start + 1] == 0xBB)
                &&  (parse->base[start + 2] == 0xBF)) {
                    LOG_TRACE("parse_value: skip BOM", start);
                    parse->value -= 3;  // remove BOM from size
                }
            }
            if (parse->type & T_Exact) {
                if (!memo_add(parse)) return false;  // memo failed!
            }
        } else if ((parse->prefix == utf16) || (parse->prefix == utf16_mem)) {
            LOG_TRACE("parse_value: utf16", parse->value);
            // check for byte-order-mark
            if (parse->value >= 2) {  // require at least 2 bytes
                if ((parse->base[start] == 0xFE)
                &&  (parse->base[start + 1] == 0xFF)) {
                    LOG_TRACE("parse_value: skip BOM/BE", start);
                    parse->value -= 2;  // remove BOM/BE from size
                }
                else 
                if ((parse->base[start] == 0xFF)
                &&  (parse->base[start + 1] == 0xFE)) {
                    LOG_TRACE("parse_value: skip BOM/LE", start);
                    parse->type |= T_Negative;  // reverse bytes for LE encoding
                    parse->value -= 2;  // remove BOM/LE from size
                }
            }
            if (parse->type & T_Exact) {
                if (!memo_add(parse)) return false;  // memo failed!
            }
        }
        LOG_DEBUG("parse_value: extended data size", parse->value);
    } else if ((parse->type & ~T_Negative) == T_Small) {
        // direct-coded Small Integer
        parse->value = (int)(parse->prefix - n_0);
        LOG_DEBUG("parse_value: Small Integer", parse->value);
    } else {
        assert(parse->value == 0);
        LOG_DEBUG("parse_value: direct-coded", parse->value);
    }
    return true;  // success
}

/**
Input:
    parse->base = <pointer to data byte(s)>
    parse->size = <size of source buffer in bytes>
    parse->start = <offset from base to prefix byte>
Output:
    parse->end = <offset past end of number data>
    parse->prefix = <value of prefix byte>
    parse->type = <value type info>
    parse->value = <integer value>
**/
BYTE parse_integer(parse_t * parse) {
    // parse Integer value up to WORD size in bits
    LOG_TRACE("parse_integer @", (WORD)parse);
    if (!parse_value(parse)) return false;  // bad Value!
    if ((parse->type & T_Base) != T_Number) {
        LOG_WARN("parse_integer: not a Number", parse->type);
        return false;  // not a Number type
    }
    if (parse->type & T_Counted) {
        LOG_WARN("parse_integer: not an Integer", parse->type);
        return false;  // not an Integer type
    }
    if (parse->type & T_Sized) {
        // extended (variable sized) Integer
        LOG_TRACE("parse_integer: end =", parse->end);
        WORD size = parse->value;  // value is the size, for extended number scanning
        if (size > sizeof(WORD)) {
            LOG_DEBUG("parse_integer: size > sizeof(WORD)", size);
            return false;
        }
        // NOTE: we count on `parse_value` to range-check `size` relative to `base`
        WORD i = parse->end - size;  // start at beginning of extended data
        WORD n = 0;
        int shift = 0;
        while (size-- > 0) {
            // accumulate integer value
            WORD m = parse->base[i++];
            LOG_TRACE("parse_integer: m =", m);
            m <<= shift;
            n |= m;
            shift += 8;
        }
        if ((parse->type & T_Negative) && (shift < (sizeof(WORD) * 8))) {
            // sign extend negative value
            n |= (MAX_WORD << shift);
        }
        parse->value = n;
    }
    LOG_DEBUG("parse_integer: value =", parse->value);
    return true;  // success
}

/**
Input:
    parse->base = <pointer to data byte(s)>
    parse->size = <size of source buffer in bytes>
    parse->start = <offset from base to prefix byte>
Output:
    parse->end = <offset past end of codepoint data>
    parse->prefix = <value of prefix byte>
    parse->type = <value type info>
    parse->value = <size of codepoint data in bytes, or memo index>
    (parse->end - parse->value) = <offset to start of codepoint data>
**/
BYTE parse_string(parse_t * parse) {
    LOG_TRACE("parse_string @", (WORD)parse);
    if (!parse_value(parse)) return false;  // bad Value!
    if ((parse->type & T_Base) != T_String) {
        LOG_WARN("parse_string: not a String", parse->type);
        return false;  // not a String type
    }
    if (parse->type & T_Counted) {
        LOG_WARN("parse_string: encoding not supported", parse->type);
        return false;  // encoding not supported
    }
    if (parse->prefix == string_0) {
        // empty String
        assert(parse->value == 0);
    }
    LOG_DEBUG("parse_string: value/size", parse->value);
    return true;  // success
}

/**
Usage:
    BYTE data[] = { utf8, n_5, 'v', 'a', 'l', 'u', 'e' };
    parse_t string_parse = {
        .base = data,
        .size = sizeof(data),
        .start = 0
    };
    if (parse_string(&string_parse)) {
        if (string_parse.prefix == mem_ref) {  // redirect to memo table entry
            if (!value_parse((DATA_PTR)string_parse.count, &string_parse)) return false;  // bad memo!
        }
        WORD code_start = (string_parse.end - string_parse.value);  // start of codepoint data
        parse_t code_parse = {
            .base = string_parse.base + code_start,
            .size = string_parse.value,
            .prefix = string_parse.prefix,
            .type = string_parse.type,
            .start = 0
        };
        while (code_parse.start < code_parse.size) {
            if (!parse_codepoint(&code_parse)) {
                break;  // or `return false;`
            }
            output(code_parse.value);
            code_parse.start = code_parse.end;
        }
    }
**/
BYTE parse_codepoint(parse_t * parse) {
    // parse a single codepoint value from a String parsed by `parse_string`
    LOG_TRACE("parse_codepoint @", (WORD)parse);
    if (parse->prefix == mem_ref)  {
        LOG_WARN("parse_codepoint: must parse table entry for memo!", parse->value);
        return false;  // resolve memo reference first...
    }
    if (parse->start >= parse->size) {
        LOG_WARN("parse_codepoint: out-of-bounds", parse->size);
        return false;  // out of bounds
    }
    LOG_TRACE("parse_codepoint: prefix", parse->prefix);
    parse->end = parse->start;
    parse->value = parse->base[parse->end++];
    LOG_TRACE("parse_codepoint: value", parse->value);
    BYTE b;
    switch (parse->prefix) {
        case octets: {
            // each byte is an untranslated codepoint in the range 0x00..0xFF
            LOG_DEBUG("parse_codepoint: octet", parse->value);
            return true;
        }
        case utf8_mem:
        case utf8: {
            LOG_LEVEL(LOG_LEVEL_TRACE+1, "parse_codepoint: utf8", parse->prefix);
            if ((parse->value & 0x80) == 0x00) {
                // 7-bit ASCII values in the range 0x00..0x7F are encoded directly
                LOG_DEBUG("parse_codepoint: utf8 ASCII", parse->value);
                return true;
            }
            // FIXME: should handle invalid and overlong sequences better...
            if ((parse->value & 0xE0) == 0xC0) {
                // 2-byte UTF-8 sequences encode the range 0x0080..0x07FF
                if (parse->end >= parse->size) return false;  // require at least 1 more byte
                parse->value &= 0x1F;  // first 5 bits of the codepoint
                b = parse->base[parse->end++];
                if ((b & 0xC0) != 0x80) return false;  // invalid continuation byte
                parse->value = ((parse->value << 6) | (b & 0x3F));  // shift in next 6 bits of codepoint
                LOG_DEBUG("parse_codepoint: utf8 2-byte", parse->value);
                return true;
            }
            if ((parse->value & 0xF0) == 0xE0) {
                // 3-byte UTF-8 sequences encode the range 0x0800..0xFFFF
                if ((parse->end + 1) >= parse->size) return false;  // require at least 2 more bytes
                parse->value &= 0x0F;  // first 4 bits of the codepoint
                b = parse->base[parse->end++];
                if ((b & 0xC0) != 0x80) return false;  // invalid continuation byte
                parse->value = ((parse->value << 6) | (b & 0x3F));  // shift in next 6 bits of codepoint
                b = parse->base[parse->end++];
                if ((b & 0xC0) != 0x80) return false;  // invalid continuation byte
                parse->value = ((parse->value << 6) | (b & 0x3F));  // shift in next 6 bits of codepoint
                LOG_DEBUG("parse_codepoint: utf8 3-byte", parse->value);
                return true;
            }
            if ((parse->value & 0xF8) == 0xF0) {
                // 4-byte UTF-8 sequences encode the range 0x010000..0x10FFFF
                if ((parse->end + 2) >= parse->size) return false;  // require at least 3 more bytes
                parse->value &= 0x07;  // first 3 bits of the codepoint
                b = parse->base[parse->end++];
                if ((b & 0xC0) != 0x80) return false;  // invalid continuation byte
                parse->value = ((parse->value << 6) | (b & 0x3F));  // shift in next 6 bits of codepoint
                b = parse->base[parse->end++];
                if ((b & 0xC0) != 0x80) return false;  // invalid continuation byte
                parse->value = ((parse->value << 6) | (b & 0x3F));  // shift in next 6 bits of codepoint
                b = parse->base[parse->end++];
                if ((b & 0xC0) != 0x80) return false;  // invalid continuation byte
                parse->value = ((parse->value << 6) | (b & 0x3F));  // shift in next 6 bits of codepoint
                if (parse->value <= MAX_UNICODE) {
                    LOG_DEBUG("parse_codepoint: utf8 4-byte", parse->value);
                    return true;
                }
            }
            LOG_WARN("parse_codepoint: bad utf8", parse->value);
            return false;  // bad UTF-8
        }
        case utf16_mem:
        case utf16: {
            LOG_LEVEL(LOG_LEVEL_TRACE+1, "parse_codepoint: utf16", parse->prefix);
            if (parse->end >= parse->size) return false;  // require at least 1 more byte
            b = parse->base[parse->end++];
            LOG_TRACE("parse_codepoint: b16 =", b);
            if (parse->type & T_Negative) {
                parse->value = ((b << 8) | parse->value);  // combine bytes (LE) to form codepoint
            } else {
                parse->value = ((parse->value << 8) | b);  // combine bytes (BE) to form codepoint
            }
            if ((parse->value & 0xF800) == 0xD800) {
                // surrogates
                WORD w = parse->value;
                if ((w & 0xFC00) == 0xD800) {
                    // high surrogate
                    LOG_TRACE("parse_codepoint: high surrogate", w);
                    w &= 0x03FF;  // mask off bottom 10 bits
                    if ((parse->end + 1) < parse->size) {  // ensure at least 2 more bytes
                        WORD v = parse->base[parse->end];
                        LOG_TRACE("parse_codepoint: v =", v);
                        b = parse->base[parse->end + 1];
                        LOG_TRACE("parse_codepoint: b =", b);
                        if (parse->type & T_Negative) {
                            v = ((b << 8) | v);  // combine bytes (LE) to form codepoint
                        } else {
                            v = ((v << 8) | b);  // combine bytes (BE) to form codepoint
                        }
                        if ((v & 0xFC00) == 0xDC00) {
                            // surrogate pair
                            LOG_TRACE("parse_codepoint: low surrogate", v);
                            v &= 0x03FF;  // mask off bottom 10 bits
                            parse->value = ((w << 10) | v) + 0x10000;
                            parse->end += 2;
                        }
                    }
                } else {
                    LOG_TRACE("parse_codepoint: low surrogate", w);
                }
                // unpaired surrogates pass through unchanged
            }
            if (parse->value <= MAX_UNICODE) {
                LOG_DEBUG("parse_codepoint: utf16", parse->value);
                return true;
            }
            LOG_WARN("parse_codepoint: bad utf16", parse->value);
            return false;  // bad UTF-16
        }
        default: {
            LOG_WARN("parse_codepoint: bad prefix", parse->prefix);
            return false;  // bad prefix
        }
    }
    //return false;  // failure
}

/**
Input:
    parse->base = <pointer to data byte(s)>
    parse->size = <size of source buffer in bytes>
    parse->start = <offset from base to prefix byte>
Output:
    parse->end = <offset past last byte of extended data>
    parse->prefix = <value of prefix byte>
    parse->type = <value type info>
    parse->value = <size of element data in bytes>
    parse->count = <value of count field (if T_Counted)>
    (parse->end - parse->value) = <offset to start of element data>
**/
BYTE parse_array(parse_t * parse) {
    LOG_TRACE("parse_array @", (WORD)parse);
    if (!parse_value(parse)) return false;  // bad Value!
    if ((parse->type & T_Base) != T_Array) {
        LOG_WARN("parse_array: not an Array", parse->type);
        return false;  // not an Array type
    }
    // NOTE: we count on `parse_value` to range-check `size` relative to `base`
    if (parse->prefix == array_0) {
        // empty Array
        assert(parse->value == 0);
    }
    LOG_DEBUG("parse_array: value/size", parse->value);
    return true;  // success
}

/**
Input:
    parse->base = <pointer to data byte(s)>
    parse->size = <size of source buffer in bytes>
    parse->start = <offset from base to prefix byte>
Output:
    parse->end = <offset past last byte of extended data>
    parse->prefix = <value of prefix byte>
    parse->type = <value type info>
    parse->value = <size of element data in bytes>
    parse->count = <value of count field (if T_Counted)>
    (parse->end - parse->value) = <offset to start of element data>
**/
BYTE parse_object(parse_t * parse) {
    LOG_TRACE("parse_object @", (WORD)parse);
    if (!parse_value(parse)) return false;  // bad Value!
    if ((parse->type & T_Base) != T_Object) {
        LOG_WARN("parse_object: not an Object", parse->type);
        return false;  // not an Object type
    }
    // NOTE: we count on `parse_value` to range-check `size` relative to `base`
    if (parse->prefix == object_0) {
        // empty Object
        assert(parse->value == 0);
    }
    LOG_DEBUG("parse_object: value/size", parse->value);
    return true;  // success
}

BYTE object_property_count(parse_t * parse, WORD * count_ptr) {
    LOG_TRACE("object_property_count @", (WORD)parse);
    WORD count = 0;
    WORD origin = parse->start;
    while (parse->start < parse->size) {
        //WORD key_start = parse->start;
        LOG_TRACE("object_property_count: key_start =", parse->start);
        if (!parse_string(parse)) return false;  // key needed
        parse->start = parse->end;
        //WORD value_start = parse->start;
        LOG_TRACE("object_property_count: value_start =", parse->start);
        if (!parse_value(parse)) return false;  // value needed
        parse->start = parse->end;
        ++count;
        LOG_TRACE("object_property_count: count =", count);
    }
    LOG_DEBUG("object_property_count: final", count);
    parse->start = origin;  // restore original starting point
    *count_ptr = count;  // "return" count value
    return true;
}

BYTE object_get_property(parse_t * parse, DATA_PTR key) {
    LOG_TRACE("object_get_property @", (WORD)parse);
    WORD origin = parse->start;
    while (parse->start < parse->size) {
        WORD key_start = parse->start;
        LOG_TRACE("object_get_property: key_start =", parse->start);
        if (!parse_string(parse)) return false;  // key needed
        parse->start = parse->end;
        //WORD value_start = parse->start;
        LOG_TRACE("object_get_property: value_start =", parse->start);
        if (!parse_value(parse)) return false;  // value needed
        if (value_equiv(key, parse->base + key_start)) {
            // keys match
            return true;  // return with value parsed
        }
        parse->start = parse->end;
    }
    LOG_DEBUG("object_get_property: not found", parse->end);
    parse->start = origin;  // restore original starting point
    return false;  // property not found
}

BYTE value_parse(DATA_PTR value, parse_t * parse) {
    LOG_TRACE("value_parse @", (WORD)value);
    parse->base = value;
    parse->size = MAX_WORD;  // don't know how big value will be
    parse->start = 0;  // start at the beginning
    if (!parse_value(parse)) {
        LOG_WARN("value_parse: bad value", (WORD)value);
        return false;  // bad value
    }
    // warning: data already written through `parse` pointer, even on failure!
    return true;
}

/**
Usage:
    BYTE data[] = { utf8, n_5, 'v', 'a', 'l', 'u', 'e' };
    parse_t parse;
    if (!string_parse(data, &parse)) return false;  // bad string
    while (parse.start < parse.size) {
        if (!parse_codepoint(&parse)) return false;  // bad codepoint
        output(parse.value);
        parse.start = parse.end;
    }
**/
BYTE string_parse(DATA_PTR value, parse_t * parse) {  // prepare parse structure for iteration by `parse_codepoint`
    LOG_TRACE("string_parse @", (WORD)value);
    parse->base = value;
    parse->size = MAX_WORD;  // don't know how big value will be
    parse->start = 0;  // start at the beginning
    if (!parse_string(parse)) {
        LOG_WARN("value_parse: bad value", (WORD)value);
        return false;  // bad value
    }
    if (parse->prefix == mem_ref) {  // redirect to memo table entry
        if (!value_parse((DATA_PTR)parse->count, parse)) return false;  // bad memo!
    }
    WORD code_start = (parse->end - parse->value);  // start of codepoint data
    // note: prefix and type are used by `parse_codepoint`, but are already set by `parse_value`...
    parse->base += code_start;  // reposition to start of codepoint data
    parse->size = parse->value;  // limit to codepoint data
    parse->start = 0;  // start at the beginning
    // warning: data already written through `parse` pointer, even on failure!
    return true;
}

BYTE value_type(DATA_PTR value, BYTE * type) {
    LOG_TRACE("value_type @", (WORD)value);
    parse_t parse;
    if (!value_parse(value, &parse)) {
        LOG_WARN("value_type: bad value", (WORD)value);
        return false;  // bad value
    }
    *type = parse.type;  // "return" type field
    return true;
}

BYTE value_integer(DATA_PTR value, WORD * number) {
    LOG_TRACE("value_integer @", (WORD)value);
    parse_t parse = {
        .base = value,
        .size = MAX_WORD,  // don't know how big value will be
        .start = 0
    };
    if (!parse_integer(&parse)) {
        LOG_WARN("value_integer: bad value", (WORD)value);
        return false;  // bad value
    }
    *number = parse.value;  // "return" integer value
    return true;
};
