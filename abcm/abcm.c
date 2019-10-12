/*
 * abcm.c -- Actor Byte-Code Machine
 */
#include <stdlib.h>
#include <assert.h>

#include "abcm.h"
#include "log.h"
#include "test.h"

static char * _semver = "0.0.1";

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
    { utf8, n_4, 'N', 'u', 'l', 'l', '\0' },
    { utf8, n_7, 'B', 'o', 'o', 'l', 'e', 'a', 'n', '\0' },
    { utf8, n_6, 'N', 'u', 'm', 'b', 'e', 'r', '\0' },
    { utf8, n_6, 'S', 't', 'r', 'i', 'n', 'g', '\0' },
    { utf8, n_5, 'A', 'r', 'r', 'a', 'y', '\0' },
    { utf8, n_6, 'O', 'b', 'j', 'e', 'c', 't', '\0' }
};
// NOTE: the '\0'-terminators are not required, but interoperate better with C
BYTE s_[] = { utf8, n_0, '\0' };
BYTE s_kind[] = { utf8, n_4, 'k', 'i', 'n', 'd', '\0' };
BYTE s_message[] = { utf8, n_7, 'm', 'e', 's', 's', 'a', 'g', 'e', '\0' };
BYTE s_actor[] = { utf8, n_5, 'a', 'c', 't', 'o', 'r', '\0' };
BYTE s_behavior[] = { utf8, n_8, 'b', 'e', 'h', 'a', 'v', 'i', 'o', 'r', '\0' };
BYTE s_name[] = { utf8, n_4, 'n', 'a', 'm', 'e', '\0' };
BYTE s_value[] = { utf8, n_5, 'v', 'a', 'l', 'u', 'e', '\0' };
BYTE s_error[] = { utf8, n_5, 'e', 'r', 'r', 'o', 'r', '\0' };
/*
// Actions
    { "kind":"actor_send", "message":<dictionary>, "actor":<address> }
    { "kind":"actor_become", "behavior":<behavior> }
    { "kind":"actor_ignore" }
    { "kind":"actor_assign", "name":<string>, "value":<expression> }
    { "kind":"actor_fail", "error":<expression> }
*/

static DATA_PTR memo_table[1<<8] = {};
static BYTE memo_index = 0;  // index of next memo slot to use

void memo_clear() {  // reset memo table between top-level values
    memo_index = 0;
    do {
        memo_table[memo_index] = s_;  // initialize with safe empty-string
    } while (++memo_index);  // stop when we wrap-around to 0
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
        .size = 16,  // WARNING! we don't really know how big the `size` field is, but this should be plenty...
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
    LOG_TRACE("parse_prefix: base", (WORD)parse->base);
    LOG_TRACE("parse_prefix: start", parse->start);
    LOG_TRACE("parse_prefix: size", parse->size);
    if (parse->start >= parse->size) {
        LOG_WARN("parse_prefix: out-of-bounds", parse->size);
        return false;  // out of bounds
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
    parse->count = <value of count field (if T_Counted)>
**/
BYTE parse_value(parse_t * parse) {
    LOG_TRACE("parse_value @", (WORD)parse);
    parse->value = 0;  // default value/size
    parse->count = 0;  // default count
    if (!parse_prefix(parse)) return false;  // bad prefix!
    if (parse->prefix == mem_ref) {
        // FIXME: handle memo references better...
        if (parse->end >= parse->size) {
            LOG_WARN("parse_value: out-of-bounds", parse->size);
            return false;  // out of bounds
        }
        BYTE index = parse->base[parse->end++];
        parse->value = index;
        parse->count = (WORD)(memo_table[index]);
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
                // FIXME: add String to memo table
                LOG_WARN("parse_string: memo not implemented", parse->value);
                return false;  // memo not implemented
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
                // FIXME: add String to memo table
                LOG_WARN("parse_string: memo not implemented", parse->value);
                return false;  // memo not implemented
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
    if (parse->prefix == mem_ref) {
        LOG_WARN("parse_string: memo not implemented", parse->value);
        return false;  // memo not implemented
    }
    if (parse->prefix == string_0) {
        // empty String
        assert(parse->value == 0);
    }
    if (parse->type & T_Sized) {
        // extended (variable sized) String
        if (parse->type & T_Exact) {
            // FIXME: add String to memo table
            LOG_WARN("parse_string: memo not implemented", parse->value);
            return false;  // memo not implemented
        }
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
    if (parse->start >= parse->size) {
        LOG_WARN("parse_codepoint: out-of-bounds", parse->size);
        return false;  // out of bounds
    }
    LOG_TRACE("parse_codepoint: prefix", parse->prefix);
    if (parse->prefix == mem_ref) return false;  // FIXME: MEMO GET NOT SUPPORTED!
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

static BYTE number_equal(parse_t * x_parse, parse_t * y_parse) {
    // compare parsed values known to be Numbers
    if (x_parse->type & T_Counted) {
        LOG_WARN("number_equal: no Unum support for x", x_parse->type);
        return false;  // not an Integer type
    }
    if (y_parse->type & T_Counted) {
        LOG_WARN("number_equal: no Unum support for y", y_parse->type);
        return false;  // not an Integer type
    }
    DATA_PTR x_data;  // pointer to x number data
    WORD x_n;  // number of data bytes in x
    if (x_parse->type & T_Sized) {
        // extended integer
        x_data = x_parse->base + (x_parse->end - x_parse->value);
        x_n = x_parse->value;
    } else {
        // direct-coded (small) integer
        BYTE xx_data = x_parse->value;
        x_data = &xx_data;
        x_n = sizeof(xx_data);
    }
    DATA_PTR y_data;  // pointer to y number data
    WORD y_n;  // number of data bytes in y
    if (y_parse->type & T_Sized) {
        // extended integer
        y_data = y_parse->base + (y_parse->end - y_parse->value);
        y_n = y_parse->value;
    } else {
        // direct-coded (small) integer
        BYTE yy_data = y_parse->value;
        y_data = &yy_data;
        y_n = sizeof(yy_data);
    }
    BYTE xx_sign = (x_parse->type & T_Negative) ? -1 : 0;
    BYTE yy_sign = (y_parse->type & T_Negative) ? -1 : 0;
    WORD i = 0;
    while ((i < x_n) || (i < y_n)) {
        BYTE xx_byte = (i < x_n) ? x_data[i] : xx_sign;  // sign extended comparison
        BYTE yy_byte = (i < y_n) ? y_data[i] : yy_sign;  // sign extended comparison
        if (xx_byte != yy_byte) {
            LOG_DEBUG("number_equal: mismatch Number xx", xx_byte);
            LOG_DEBUG("number_equal: mismatch Number yy", yy_byte);
            return false;  // mismatched elements
        }
        ++i;
    }
    LOG_DEBUG("number_equal: MATCH Number", true);
    return true;  // Number values match
}

static BYTE string_equal(parse_t * x_parse, parse_t * y_parse) {
    // compare parsed values known to be Strings
    assert(x_parse->start < (x_parse->end - x_parse->value));
    assert(y_parse->start < (y_parse->end - y_parse->value));
    parse_t x_code = {
        .base = x_parse->base + (x_parse->end - x_parse->value),  // start of x codepoint data
        .size = x_parse->value,
        .prefix = x_parse->prefix,
        .type = x_parse->type,
        .start = 0
    };
    parse_t y_code = {
        .base = y_parse->base + (y_parse->end - y_parse->value),  // start of y codepoint data
        .size = y_parse->value,
        .prefix = y_parse->prefix,
        .type = y_parse->type,
        .start = 0
    };
    while ((x_code.start < x_code.size) && (y_code.start < y_code.size)) {
        if (!(parse_codepoint(&x_code) && parse_codepoint(&y_code))) return false;  // bad codepoint
        if (x_code.value != y_code.value) {
            LOG_TRACE("string_equal: mismatch x start", x_code.start);
            LOG_TRACE("string_equal: mismatch y start", y_code.start);
            LOG_DEBUG("string_equal: mismatch String x", x_code.value);
            LOG_DEBUG("string_equal: mismatch String y", y_code.value);
            return false;  // mismatched codepoints
        }
        x_code.start = x_code.end;
        y_code.start = y_code.end;
    }
    if ((x_code.start < x_code.size) || (y_code.start < y_code.size)) {
        if (x_code.start < x_code.size) LOG_DEBUG("string_equal: more String x...", x_code.start);
        if (y_code.start < y_code.size) LOG_DEBUG("string_equal: more String y...", y_code.start);
        return false;  // one String ended before the other
    }
    LOG_DEBUG("string_equal: MATCH String", true);
    return true;  // String values match
}

static BYTE array_equal(parse_t * x_parse, parse_t * y_parse) {
    // compare parsed values known to be Arrays
    assert(x_parse->start < (x_parse->end - x_parse->value));
    assert(y_parse->start < (y_parse->end - y_parse->value));
    parse_t x_item = {
        .base = x_parse->base + (x_parse->end - x_parse->value),  // start of x element data
        .size = x_parse->value,
        .start = 0
    };
    parse_t y_item = {
        .base = y_parse->base + (y_parse->end - y_parse->value),  // start of y element data
        .size = y_parse->value,
        .start = 0
    };
    while ((x_item.start < x_item.size) && (y_item.start < y_item.size)) {
        if (!parse_equal(&x_item, &y_item)) {  // recursively compare corresponding items
            LOG_TRACE("array_equal: mismatch x start", x_item.start);
            LOG_TRACE("array_equal: mismatch y start", y_item.start);
            LOG_DEBUG("array_equal: mismatch Array x", x_item.value);
            LOG_DEBUG("array_equal: mismatch Array y", y_item.value);
            return false;  // mismatched elements
        }
        x_item.start = x_item.end;
        y_item.start = y_item.end;
    }
    if ((x_item.start < x_item.size) || (y_item.start < y_item.size)) {
        if (x_item.start < x_item.size) LOG_DEBUG("array_equal: more Array x...", x_item.start);
        if (y_item.start < y_item.size) LOG_DEBUG("array_equal: more Array y...", y_item.start);
        return false;  // one Array ended before the other
    }
    LOG_DEBUG("array_equal: MATCH Array", true);
    return true;  // Array values match
}

static BYTE object_get_property(parse_t * parse, DATA_PTR key) {
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
        if (value_equal(key, parse->base + key_start)) {
            // keys match
            return true;  // return with value parsed
        }
        parse->start = parse->end;
    }
    LOG_DEBUG("object_get_property: not found", parse->end);
    parse->start = origin;  // restore original starting point
    return false;  // property not found
}

static BYTE object_equal(parse_t * x_parse, parse_t * y_parse) {
    // compare parsed values known to be Objects
    parse_t x_prop = {
        .base = x_parse->base + (x_parse->end - x_parse->value),  // start of x property data
        .size = x_parse->value,
        .start = 0
    };
    parse_t y_prop = {
        .base = y_parse->base + (y_parse->end - y_parse->value),  // start of y property data
        .size = y_parse->value,
        .start = 0
    };
    WORD x_count;
    if (!(object_property_count(&x_prop, &x_count))) {
        return false;  // bad Properties
    }
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "object_equal: Object x type", x_parse->type);
    if (x_parse->type & T_Counted) {
        LOG_TRACE("object_equal: Object x counted", x_parse->count);
        if (x_count != x_parse->count) {
            LOG_WARN("object_equal: Object x expected", x_parse->count);
        }
    }
    WORD y_count;
    if (!(object_property_count(&y_prop, &y_count))) {
        return false;  // bad Properties
    }
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "object_equal: Object y type", y_parse->type);
    if (y_parse->type & T_Counted) {
        LOG_TRACE("object_equal: Object y counted", y_parse->count);
        if (y_count != y_parse->count) {
            LOG_WARN("object_equal: Object y expected", y_parse->count);
        }
    }
    if (x_count != y_count) {
        LOG_DEBUG("object_equal: mismatched property counts", (x_count - y_count));
        return false;  // mismatched elements
    }
    x_prop.start = 0;  // restore original starting point
    y_prop.start = 0;  // restore original starting point
    parse_t * parse = &x_prop;
    while (parse->start < parse->size) {
        WORD key_start = parse->start;
        if (!parse_string(parse)) return false;  // key needed
        parse->start = parse->end;
        WORD value_start = parse->start;
        if (!parse_value(parse)) return false;  // value needed
        if (object_get_property(&y_prop, (parse->base + key_start))) {
            // found matching property
            x_prop.start = value_start;
            if (!parse_equal(&x_prop, &y_prop)) {
                LOG_DEBUG("object_equal: mismatched Object property", value_start);
                return false;  // mismatched property
            }
        } else {
            LOG_DEBUG("object_equal: missing Object property", key_start);
            return false;  // missing property
        }
        y_prop.start = 0;  // restore original starting point
        parse->start = parse->end;
    }
    LOG_DEBUG("object_equal: MATCH Object", true);
    return true;  // Object values match
}

BYTE parse_equal(parse_t * x_parse, parse_t * y_parse) {
    LOG_TRACE("parse_equal: x @", (WORD)x_parse);
    LOG_TRACE("parse_equal: y @", (WORD)y_parse);
    if (x_parse == y_parse) {
        LOG_WARN("parse_equal: MATCH same (aliased)!?", true);
        return parse_value(x_parse);  // advance "both" parsers (aliased)
    }
    if (!(parse_value(x_parse) && parse_value(y_parse))) {
        return false;  // bad Value(s)
    }
    if ((x_parse->type & T_Base) != (y_parse->type & T_Base)) {
        LOG_DEBUG("parse_equal: mismatch x_type", x_parse->type);
        LOG_DEBUG("parse_equal: mismatch y_type", y_parse->type);
        return false;  // mismatched base types
    }
    switch (x_parse->type & T_Base) {
        case T_Number: {
            return number_equal(x_parse, y_parse);
        }
        case T_String: {
            return string_equal(x_parse, y_parse);
        }
        case T_Array: {
            return array_equal(x_parse, y_parse);
        }
        case T_Object: {
            return object_equal(x_parse, y_parse);
        }
        default: {
            BYTE result = (x_parse->prefix == y_parse->prefix);
            LOG_DEBUG("parse_equal: MATCH direct", result);
            return result;
        }
    }
    LOG_WARN("parse_equal: FAIL!", false);
    return false;
}

BYTE value_equal(DATA_PTR x, DATA_PTR y) {
    LOG_TRACE("value_equal: x @", (WORD)x);
    LOG_TRACE("value_equal: y @", (WORD)y);
    if (x == y) {
        LOG_DEBUG("value_equal: MATCH same", true);
        return true;  // identical values are equal
    }
    parse_t x_parse = {
        .base = x,
        .size = MAX_WORD,  // don't know how big value will be
        .start = 0
    };
    parse_t y_parse = {
        .base = y,
        .size = MAX_WORD,  // don't know how big value will be
        .start = 0
    };
    return parse_equal(&x_parse, &y_parse);
};

int main(int argc, char *argv[]) {
    log_config.level = LOG_LEVEL_WARN;
    //log_config.level = LOG_LEVEL_TRACE+1;
    assert(_semver == _semver);  // FIXME: vacuous use of `_semver`, to satisfy compiler...
    LOG_INFO(_semver, (WORD)_semver);
    memo_clear();
    int result = run_test_suite();  // pass == 0, fail != 0
    return (exit(result), result);
}
