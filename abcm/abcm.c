/*
 *  ABCM - Actor Byte-Code Machine
 */
#include <stdlib.h>
#include <stdint.h>
//#include <unistd.h>
#include <assert.h>

#define LOG_ALL
#include "log.c"

static char * _semver = "0.0.1";

typedef uint8_t BYTE;           // 8-bit data (octet)
typedef uintptr_t WORD;         // native machine word

#define MAX_BYTE ((BYTE)-1)
#define MAX_WORD ((WORD)-1)
#define MAX_UNICODE ((WORD)0x10FFFF)

typedef uint8_t NAT8;           // 8-bit natural ring
typedef uint16_t NAT16;         // 16-bit natural ring
typedef uint32_t NAT32;         // 32-bit natural ring
typedef uint64_t NAT64;         // 64-bit natural ring

typedef void * VOID_PTR;        // untyped pointer
typedef BYTE * DATA_PTR;        // pointer to data bytes
typedef WORD (*CODE_PTR)(WORD); // pointer to native code

typedef enum { /*2#_000*/ /*2#_001*/ /*2#_010*/ /*2#_011*/ /*2#_100*/ /*2#_101*/ /*2#_110*/ /*2#_111*/
/*2#00000_*/   false,     true,      array_0,   object_0,  array,     object,    array_n,   object_n,
/*2#00001_*/   octets,    mem_ref,   utf8,      utf8_mem,  utf16,     utf16_mem, s_encoded, string_0,
/*2#00010_*/   p_int_0,   p_int_1,   p_int_2,   p_int_3,   p_int_4,   p_int_5,   p_int_6,   p_int_7,
/*2#00011_*/   m_int_0,   m_int_1,   m_int_2,   m_int_3,   m_int_4,   m_int_5,   m_int_6,   m_int_7,
/*2#00100_*/   p_flt_0,   p_flt_1,   p_flt_2,   p_flt_3,   p_flt_4,   p_flt_5,   p_flt_6,   p_flt_7,
/*2#00101_*/   m_flt_0,   m_flt_1,   m_flt_2,   m_flt_3,   m_flt_4,   m_flt_5,   m_flt_6,   m_flt_7,
/*2#00110_*/   p_rng_0,   p_rng_1,   p_rng_2,   p_rng_3,   p_rng_4,   p_rng_5,   p_rng_6,   p_rng_7,
/*2#00111_*/   m_rng_0,   m_rng_1,   m_rng_2,   m_rng_3,   m_rng_4,   m_rng_5,   m_rng_6,   m_rng_7,
/*2#01000_*/   n_m64,     n_m63,     n_m62,     n_m61,     n_m60,     n_m59,     n_m58,     n_m57,
/*2#01001_*/   n_m56,     n_m55,     n_m54,     n_m53,     n_m52,     n_m51,     n_m50,     n_m49,
/*2#01010_*/   n_m48,     n_m47,     n_m46,     n_m45,     n_m44,     n_m43,     n_m42,     n_m41,
/*2#01011_*/   n_m40,     n_m39,     n_m38,     n_m37,     n_m36,     n_m35,     n_m34,     n_m33,
/*2#01100_*/   n_m32,     n_m31,     n_m30,     n_m29,     n_m28,     n_m27,     n_m26,     n_m25,
/*2#01101_*/   n_m24,     n_m23,     n_m22,     n_m21,     n_m20,     n_m19,     n_m18,     n_m17,
/*2#01110_*/   n_m16,     n_m15,     n_m14,     n_m13,     n_m12,     n_m11,     n_m10,     n_m9,
/*2#01111_*/   n_m8,      n_m7,      n_m6,      n_m5,      n_m4,      n_m3,      n_m2,      n_m1,
/*2#10000_*/   n_0,       n_1,       n_2,       n_3,       n_4,       n_5,       n_6,       n_7,
/*2#10001_*/   n_8,       n_9,       n_10,      n_11,      n_12,      n_13,      n_14,      n_15,
/*2#10010_*/   n_16,      n_17,      n_18,      n_19,      n_20,      n_21,      n_22,      n_23,
/*2#10011_*/   n_24,      n_25,      n_26,      n_27,      n_28,      n_29,      n_30,      n_31,
/*2#10100_*/   n_32,      n_33,      n_34,      n_35,      n_36,      n_37,      n_38,      n_39,
/*2#10101_*/   n_40,      n_41,      n_42,      n_43,      n_44,      n_45,      n_46,      n_47,
/*2#10110_*/   n_48,      n_49,      n_50,      n_51,      n_52,      n_53,      n_54,      n_55,
/*2#10111_*/   n_56,      n_57,      n_58,      n_59,      n_60,      n_61,      n_62,      n_63,
/*2#11000_*/   n_64,      n_65,      n_66,      n_67,      n_68,      n_69,      n_70,      n_71,
/*2#11001_*/   n_72,      n_73,      n_74,      n_75,      n_76,      n_77,      n_78,      n_79,
/*2#11010_*/   n_80,      n_81,      n_82,      n_83,      n_84,      n_85,      n_86,      n_87,
/*2#11011_*/   n_88,      n_89,      n_90,      n_91,      n_92,      n_93,      n_94,      n_95,
/*2#11100_*/   n_96,      n_97,      n_98,      n_99,      n_100,     n_101,     n_102,     n_103,
/*2#11101_*/   n_104,     n_105,     n_106,     n_107,     n_108,     n_109,     n_110,     n_111,
/*2#11110_*/   n_112,     n_113,     n_114,     n_115,     n_116,     n_117,     n_118,     n_119,
/*2#11111_*/   n_120,     n_121,     n_122,     n_123,     n_124,     n_125,     n_126,     null
} prefix_t;

typedef enum {
    T_Null,
    T_Boolean,
    T_Number,
    T_String,
    T_Array,
    T_Object,
/*  T_6_unused, */
    T_Base = ((1 << 3) - 1),  // mask for base type
    T_Negative = (1 << 3),  // also "UTF-16LE" for Strings
    T_Exact = (1 << 4),  // also "memoize" for Strings
    T_Sized = (1 << 5),
    T_Counted = (1 << 6),  // also "encoded" for Strings, "Unum" for Numbers (has exponent)
    T_Capability = (1 << 7)
} type_t;

#define T_Small (T_Number|T_Exact)
#define T_Integer (T_Number|T_Exact|T_Sized)
#define T_Float (T_Number|T_Exact|T_Sized|T_Counted)
#define T_Range (T_Number|T_Sized|T_Counted)

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
    { utf8, n_4, 'N', 'u', 'l', 'l', '\0', '\0', '\0', '\0' },
    { utf8, n_7, 'B', 'o', 'o', 'l', 'e', 'a', 'n', '\0' },
    { utf8, n_6, 'N', 'u', 'm', 'b', 'e', 'r', '\0', '\0' },
    { utf8, n_6, 'S', 't', 'r', 'i', 'n', 'g', '\0', '\0' },
    { utf8, n_5, 'A', 'r', 'r', 'a', 'y', '\0', '\0', '\0' },
    { utf8, n_6, 'O', 'b', 'j', 'e', 'c', 't', '\0', '\0' }
};

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

static int test_bytecode_types() {
    assert(null != 0);
    assert(null == MAX_BYTE);
    assert(true);
    assert(!false);
    assert(!!true);
    assert(true == (0 == 0));
    assert(false == (0 != 0));
    assert(prefix_type[false] == T_Boolean);
    assert(prefix_type[true] == T_Boolean);
    assert((prefix_type[array_0] & T_Base) == T_Array);
    assert((prefix_type[array] & T_Base) == T_Array);
    assert((prefix_type[array_n] & T_Base) == T_Array);
    assert((prefix_type[array] & ~T_Base) == T_Sized);
    assert((prefix_type[array_n] & ~T_Base) == (T_Sized | T_Counted));
    assert((prefix_type[object_0] & T_Base) == T_Object);
    assert((prefix_type[object] & T_Base) == T_Object);
    assert((prefix_type[object_n] & T_Base) == T_Object);
    assert((prefix_type[array] & ~T_Base) == T_Sized);
    assert((prefix_type[array_n] & ~T_Base) == (T_Sized | T_Counted));
    assert((prefix_type[octets]) == (T_String | T_Sized));
    assert((prefix_type[utf8]) == (T_String | T_Sized));
    assert((prefix_type[string_0]) == T_String);
    assert(prefix_type[p_int_0] == (T_Number | T_Exact | T_Sized));
    assert((prefix_type[p_int_0] & T_Negative) == (p_int_0 & T_Negative));
    assert(prefix_type[m_int_0] == (T_Number | T_Exact | T_Negative | T_Sized));
    assert((prefix_type[m_int_0] & T_Negative) == (m_int_0 & T_Negative));
    assert((prefix_type[p_flt_0] & T_Exact) == (~p_flt_0 & T_Exact));
    assert((prefix_type[p_flt_0] & T_Negative) == (p_flt_0 & T_Negative));
    assert((prefix_type[m_flt_0] & T_Exact) == (~m_flt_0 & T_Exact));
    assert((prefix_type[m_flt_0] & T_Negative) == (m_flt_0 & T_Negative));
    assert((prefix_type[p_rng_0] & T_Exact) == (~p_rng_0 & T_Exact));
    assert((prefix_type[p_rng_0] & T_Negative) == (p_rng_0 & T_Negative));
    assert((prefix_type[m_rng_0] & T_Exact) == (~m_rng_0 & T_Exact));
    assert((prefix_type[m_rng_0] & T_Negative) == (m_rng_0 & T_Negative));
    assert((prefix_type[n_1] & T_Base) == T_Number);
    assert((prefix_type[n_0] & T_Base) == T_Number);
    assert((prefix_type[n_m1] & T_Base) == T_Number);
    assert(prefix_type[n_1] == T_Small);
    assert(prefix_type[n_0] == T_Small);
    assert(prefix_type[n_m1] == (T_Small | T_Negative));
    assert(prefix_type[null] == T_Null);
    assert((n_0 - n_0) == 0);
    assert((n_1 - n_0) == 1);
    assert((n_m1 - n_0) == -1);
    assert((n_64 - n_0) == 64);
    assert((n_m64 - n_0) == -64);
    assert((n_126 - n_0) == 126);
    assert(sizeof(s_) == 3);
    return 0;
}

static DATA_PTR memo_table[1<<8] = {};
static BYTE memo_index = 0;  // index of next memo slot to use

void memo_clear() {  // reset memo table between top-level values
    memo_index = 0;
    do {
        memo_table[memo_index] = s_;  // initialize with safe empty-string
    } while (++memo_index);  // stop when we wrap-around to 0
};

typedef struct {
    DATA_PTR    base;           // base address of source buffer
    WORD        size;           // size (in bytes) of valid source data [0, size-1]
    WORD        start;          // offset to begin parsing
    WORD        end;            // offset at which parsing ended (next start)
    prefix_t    prefix;         // parsed data prefix
    type_t      type;           // parsed data type info
    WORD        value;          // parsed data value/size (accumulator or pointer)
    WORD        count;          // parsed data count (for T_Counted)
} parse_t;

BYTE parse_integer(parse_t * parse);  // FORWARD DECLARATION

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
            WORD skip = ext_parse.end - ext_parse.start;
            parse->value -= skip;  // remove count field from size
        }
        if ((parse->prefix == utf8) || (parse->prefix == utf8_mem)) {
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

static int test_parse_integer() {
    BYTE data[] = {
        n_0,
        n_1,
        n_m1,
        p_int_0, n_1, 42,
        m_int_0, n_3, 0xFF, 0xFF, 0xFF,
        p_int_4, n_2, 0x00, 0x08,
        m_int_4, n_2, 0x00, 0xF8,
        m_int_0, n_4, 0xFE, 0xFF, 0xFF, 0xFF,
        p_int_0, n_2, 0xFE, 0xFF,
        m_int_3, n_9, 0x00, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        p_int_0, n_0
    };
    parse_t parse = {
        .base = data,
        .size = sizeof(data),
        .start = 0
    };

    assert(parse_integer(&parse) == true);
    assert((parse.end - parse.start) == 1);
    assert(parse.value == 0);

    parse.start = parse.end;
    assert(parse_integer(&parse) == true);
    assert((parse.end - parse.start) == 1);
    assert(parse.value == 1);

    parse.start = parse.end;
    assert(parse_integer(&parse) == true);
    assert((parse.end - parse.start) == 1);
    assert(parse.value == -1);

    parse.start = parse.end;
    assert(parse_integer(&parse) == true);
    assert((parse.end - parse.start) == 3);
    assert(parse.value == 42);

    parse.start = parse.end;
    assert(parse_integer(&parse) == true);
    assert((parse.end - parse.start) == 5);
    assert(parse.value == -1);

    parse.start = parse.end;
    assert(parse_integer(&parse) == true);
    assert((parse.end - parse.start) == 4);
    assert(parse.value == 2048);

    parse.start = parse.end;
    assert(parse_integer(&parse) == true);
    assert((parse.end - parse.start) == 4);
    assert(parse.value == -2048);

    parse.start = parse.end;
    assert(parse_integer(&parse) == true);
    assert((parse.end - parse.start) == 6);
    assert(parse.value == -2);

    parse.start = parse.end;
    assert(parse_integer(&parse) == true);
    assert((parse.end - parse.start) == 4);
    assert(parse.value == 0xFFFE);
    assert(parse.value == 65534);

    parse.start = parse.end;
    assert(parse_integer(&parse) == false);
    assert((parse.end - parse.start) == 11);
    assert(parse.value == 9);

    parse.start = parse.end;
    assert(parse_integer(&parse) == true);
    assert((parse.end - parse.start) == 2);
    assert(parse.value == 0);

    assert(parse.end == parse.size);  // used up all the data
    return 0;
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

static int test_parse_string() {
    BYTE data[] = {
        string_0,
        octets, n_4, 'k', 'i', 'n', 'd',
        utf8, n_4, 'k', 'i', 'n', 'd',
        utf8, n_7, 0xEF, 0xBB, 0xBF, 'k', 'i', 'n', 'd',
        utf16, n_8, '\0', 'k', '\0', 'i', '\0', 'n', '\0', 'd',
        utf16, n_10, 0xFE, 0xFF, '\0', 'k', '\0', 'i', '\0', 'n', '\0', 'd',
        utf16, n_10, 0xFF, 0xFE, 'k', '\0', 'i', '\0', 'n', '\0', 'd', '\0',
        utf8, n_3, 0xEF, 0xBB, 0xBF,
        utf16, n_2, 0xFF, 0xFE,
        utf8, n_0
    };
    parse_t parse = {
        .base = data,
        .size = sizeof(data),
        .start = 0
    };

    assert(parse_string(&parse) == true);
    assert((parse.end - parse.start) == 1);
    assert(parse.value == 0);

    parse.start = parse.end;
    assert(parse_string(&parse) == true);
    assert((parse.end - parse.start) == 6);
    assert(parse.value == 4);

    parse.start = parse.end;
    assert(parse_string(&parse) == true);
    assert((parse.end - parse.start) == 6);
    assert(parse.value == 4);

    parse.start = parse.end;
    assert(parse_string(&parse) == true);
    assert((parse.end - parse.start) == 9);
    assert(parse.value == 4);

    parse.start = parse.end;
    assert(parse_string(&parse) == true);
    assert((parse.end - parse.start) == 10);
    assert(parse.value == 8);

    parse.start = parse.end;
    assert(parse_string(&parse) == true);
    assert((parse.end - parse.start) == 12);
    assert(parse.value == 8);

    parse.start = parse.end;
    assert(parse_string(&parse) == true);
    assert((parse.end - parse.start) == 12);
    assert(parse.value == 8);

    parse.start = parse.end;
    assert(parse_string(&parse) == true);
    assert((parse.end - parse.start) == 5);
    assert(parse.value == 0);

    parse.start = parse.end;
    assert(parse_string(&parse) == true);
    assert((parse.end - parse.start) == 4);
    assert(parse.value == 0);

    parse.start = parse.end;
    assert(parse_string(&parse) == true);
    assert((parse.end - parse.start) == 2);
    assert(parse.value == 0);

    assert(parse.end == parse.size);  // used up all the data
    return 0;
}

/**
Usage:
    BYTE data[] = { utf8, n_5, 'v', 'a', 'l', 'u', 'e' };
    parse_t parse = {
        .base = data,
        .size = sizeof(data),
        .start = 0
    };
    if (parse_string(&parse)) {
        parse.size = parse.end;
        parse.start = (parse.end - parse.value);  // start of codepoint data
        while (parse.start < parse.size) {
            if (!parse_codepoint(&parse)) {
                break;  // or `return false;`
            }
            output(parse.value);
            parse.start = parse.end;
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
            LOG_LEVEL(LOG_LEVEL_TRACE + 1, "parse_codepoint: utf8", parse->prefix);
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
            LOG_LEVEL(LOG_LEVEL_TRACE + 1, "parse_codepoint: utf16", parse->prefix);
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

static int test_parse_codepoint() {
    int i;
    WORD w;

    WORD code[] = { 0x0061, 0x0063, 0x0074, 0x006F, 0x0072 };
    BYTE data[] = { utf8, n_5, 'a', 'c', 't', 'o', 'r' };
    parse_t parse = {
        .base = data,
        .size = sizeof(data),
        .start = 0
    };
    LOG_TRACE("test_parse_codepoint", (WORD)&parse);

    if (!parse_string(&parse)) return 1;  // bad String
    parse.base += (parse.end - parse.value);  // move base to start of codepoint data
    parse.size = parse.value;
    parse.start = 0;
    i = 0;
    while (parse.start < parse.size) {
        if (!parse_codepoint(&parse)) return 1;  // bad codepoint
        LOG_DEBUG("codepoint =", parse.value);
        w = code[i++];
        assert(parse.value == w);
        parse.start = parse.end;
    }
    assert(i == (sizeof(code) / sizeof(WORD)));

    WORD code16[] = { 0xDF62, 0xD852, 0x20AC, 0x24B62 };
    BYTE data16[] = { utf16, n_10, 0xDF, 0x62, 0xD8, 0x52, 0x20, 0xAC, 0xD8, 0x52, 0xDF, 0x62 };
    parse.base = data16;
    parse.size = sizeof(data16);
    parse.start = 0;
    if (!parse_string(&parse)) return 1;  // bad String
    parse.base += (parse.end - parse.value);  // move base to start of codepoint data
    parse.size = parse.value;
    parse.start = 0;
    i = 0;
    while (parse.start < parse.size) {
        if (!parse_codepoint(&parse)) return 1;  // bad codepoint
        LOG_DEBUG("codepoint16 =", parse.value);
        w = code16[i++];
        assert(parse.value == w);
        parse.start = parse.end;
    }
    assert(i == (sizeof(code16) / sizeof(WORD)));

    return 0;
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
        LOG_WARN("parse_string: not an Array", parse->type);
        return false;  // not an Array type
    }
    // NOTE: we count on `parse_value` to range-check `size` relative to `base`
    if (parse->prefix == array_0) {
        // empty Array
        assert(parse->value == 0);
/*
        parse->value = 0;  // value is the size, for element scanning
        LOG_DEBUG("parse_array: array_0", parse->value);
        return true;  // success
*/
    }
    LOG_DEBUG("parse_array: value/size", parse->value);
    return true;  // success
}

static int test_parse_array() {
    BYTE data[] = {
        array_0,
        array, n_4, true, false, n_0, null,
        array_n, n_1, n_0,
        array, n_0
    };
    parse_t parse = {
        .base = data,
        .size = sizeof(data),
        .start = 0
    };

    assert(parse_array(&parse) == true);
    assert((parse.end - parse.start) == 1);
    assert(parse.value == 0);

    parse.start = parse.end;
    assert(parse_array(&parse) == true);
    assert((parse.end - parse.start) == 6);
    assert(parse.value == 4);

    parse.start = parse.end;
    assert(parse_array(&parse) == true);
    assert((parse.end - parse.start) == 3);
    assert(parse.value == 0);

    parse.start = parse.end;
    assert(parse_array(&parse) == true);
    assert((parse.end - parse.start) == 2);
    assert(parse.value == 0);

    assert(parse.end == parse.size);  // used up all the data
    return 0;
}

BYTE parse_equal(parse_t * x_parse, parse_t * y_parse) {
    LOG_TRACE("parse_equal: x @", (WORD)x_parse);
    LOG_TRACE("parse_equal: y @", (WORD)y_parse);
    if (x_parse == y_parse) {
        LOG_DEBUG("parse_equal: MATCH same", true);
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
            if (x_parse->type & T_Counted) {
                LOG_WARN("parse_equal: no Unum support for x", x_parse->type);
                return false;  // not an Integer type
            }
            if (y_parse->type & T_Counted) {
                LOG_WARN("parse_equal: no Unum support for y", y_parse->type);
                return false;  // not an Integer type
            }
            DATA_PTR x_p;  // pointer to x number data
            WORD x_n;  // number of data bytes in x
            if (x_parse->type & T_Sized) {
                // extended integer
                x_p = x_parse->base + (x_parse->end - x_parse->value);
                x_n = x_parse->value;
            } else {
                // direct-coded (small) integer
                BYTE xx_data = x_parse->value;
                x_p = &xx_data;
                x_n = sizeof(xx_data);
            }
            DATA_PTR y_p;  // pointer to y number data
            WORD y_n;  // number of data bytes in y
            if (y_parse->type & T_Sized) {
                // extended integer
                y_p = y_parse->base + (y_parse->end - y_parse->value);
                y_n = y_parse->value;
            } else {
                // direct-coded (small) integer
                BYTE yy_data = y_parse->value;
                y_p = &yy_data;
                y_n = sizeof(yy_data);
            }
            BYTE xx_sign = (x_parse->type & T_Negative) ? -1 : 0;
            BYTE yy_sign = (y_parse->type & T_Negative) ? -1 : 0;
            WORD i = 0;
            while ((i < x_n) || (i < y_n)) {
                BYTE xx_byte = (i < x_n) ? x_p[i] : xx_sign;  // sign extended comparison
                BYTE yy_byte = (i < y_n) ? y_p[i] : yy_sign;  // sign extended comparison
                if (xx_byte != yy_byte) {
                    LOG_DEBUG("parse_equal: mismatch Number xx", xx_byte);
                    LOG_DEBUG("parse_equal: mismatch Number yy", yy_byte);
                    return false;  // mismatched elements
                }
                ++i;
            }
            LOG_DEBUG("parse_equal: MATCH Number", true);
            return true;  // Number values match
        }
        case T_String: {
            x_parse->start = (x_parse->end - x_parse->value);  // start of x codepoint data
            y_parse->start = (y_parse->end - y_parse->value);  // start of y codepoint data
            WORD x_end = x_parse->end;
            WORD y_end = y_parse->end;
            while ((x_parse->start < x_end) && (y_parse->start < y_end)) {
                if (!(parse_codepoint(x_parse) && parse_codepoint(y_parse))) return false;  // bad codepoint
                if (x_parse->value != y_parse->value) {
                    LOG_TRACE("parse_equal: mismatch x_parse->end", x_parse->end);
                    LOG_TRACE("parse_equal: mismatch y_parse->end", y_parse->end);
                    LOG_DEBUG("parse_equal: mismatch String x", x_parse->value);
                    LOG_DEBUG("parse_equal: mismatch String y", y_parse->value);
                    return false;  // mismatched codepoints
                }
                x_parse->start = x_parse->end;
                y_parse->start = y_parse->end;
            }
            if ((x_parse->start < x_end) || (y_parse->start < y_end)) {
                if (x_parse->start < x_end) LOG_DEBUG("parse_equal: more String x...", x_parse->start);
                if (y_parse->start < y_end) LOG_DEBUG("parse_equal: more String y...", y_parse->start);
                return false;  // one string ended before the other
            }
            LOG_DEBUG("parse_equal: MATCH String", true);
            return true;  // String values match
        }
        case T_Array: {
            x_parse->start = (x_parse->end - x_parse->value);  // start of x element data
            y_parse->start = (y_parse->end - y_parse->value);  // start of y element data
            WORD x_end = x_parse->end;
            WORD y_end = y_parse->end;
            while ((x_parse->start < x_end) && (y_parse->start < y_end)) {
                if (!parse_equal(x_parse, y_parse)) {
                    LOG_TRACE("parse_equal: mismatch x_parse->end", x_parse->end);
                    LOG_TRACE("parse_equal: mismatch y_parse->end", y_parse->end);
                    LOG_DEBUG("parse_equal: mismatch Array x", x_parse->value);
                    LOG_DEBUG("parse_equal: mismatch Array y", y_parse->value);
                    return false;  // mismatched elements
                }
                x_parse->start = x_parse->end;
                y_parse->start = y_parse->end;
            }
            if ((x_parse->start < x_end) || (y_parse->start < y_end)) {
                if (x_parse->start < x_end) LOG_DEBUG("parse_equal: more Array x...", x_parse->start);
                if (y_parse->start < y_end) LOG_DEBUG("parse_equal: more Array y...", y_parse->start);
                return false;  // one array ended before the other
            }
            LOG_DEBUG("parse_equal: MATCH Array", true);
            return true;  // Array values match
        }
        case T_Object: {
            break;
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

static int test_value_equal() {
    BYTE data_0[] = { n_0 };
    BYTE data_1[] = { p_int_0, n_0 };
    BYTE data_2[] = { p_int_0, n_1, 0x00 };
    BYTE data_3[] = { p_int_0, n_4, 0x00, 0x00, 0x00, 0x00 };
    BYTE data_4[] = { n_m2 };
    BYTE data_5[] = { m_int_0, n_3, 0xFE, 0xFF, 0xFF };
    BYTE data_6[] = { n_126 };
    BYTE data_7[] = { p_int_0, n_2, 0x7E, 0x00 };
    BYTE data_8[] = { m_int_3, n_9, 0x00, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
    BYTE data_9[] = { m_int_6, n_10, 0x00, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFF };

    assert(value_equal(data_0, data_0));
    assert(value_equal(data_0, data_1));
    assert(value_equal(data_0, data_2));
    assert(value_equal(data_0, data_3));
    assert(value_equal(data_2, data_3));
    assert(value_equal(data_4, data_5));
    assert(value_equal(data_6, data_7));
    assert(value_equal(data_8, data_9));

    BYTE data_10[] = { string_0 };
    BYTE data_11[] = { octets, n_0 };
    BYTE data_12[] = { utf8, n_0 };
    BYTE data_13[] = { utf8, n_3, 0xEF, 0xBB, 0xBF };
    BYTE data_14[] = { utf16, n_2, 0xFE, 0xFF };
    BYTE data_15[] = { utf16, n_2, 0xFF, 0xFE };

    assert(value_equal(s_, s_));
    assert(value_equal(s_, data_10));
    assert(value_equal(s_, data_11));
    assert(value_equal(s_, data_12));
    assert(value_equal(s_, data_13));
    assert(value_equal(s_, data_14));
    assert(value_equal(s_, data_15));
    assert(value_equal(data_13, data_14));
    assert(value_equal(data_13, data_15));
    assert(value_equal(data_14, data_15));

    assert(!value_equal(s_kind, s_));
    assert(!value_equal(s_kind, s_actor));

    BYTE data_20[] = { octets, n_4, 'k', 'i', 'n', 'd' };
    BYTE data_21[] = { utf8, n_4, 'k', 'i', 'n', 'd' };
    BYTE data_22[] = { utf8, n_7, 0xEF, 0xBB, 0xBF, 'k', 'i', 'n', 'd' };
    BYTE data_23[] = { utf16, n_8, '\0', 'k', '\0', 'i', '\0', 'n', '\0', 'd' };
    BYTE data_24[] = { utf16, n_10, 0xFE, 0xFF, '\0', 'k', '\0', 'i', '\0', 'n', '\0', 'd' };
    BYTE data_25[] = { utf16, n_10, 0xFF, 0xFE, 'k', '\0', 'i', '\0', 'n', '\0', 'd', '\0' };

    assert(value_equal(s_kind, data_20));
    assert(value_equal(s_kind, data_21));
    assert(value_equal(s_kind, data_22));
    assert(value_equal(s_kind, data_23));
    assert(value_equal(s_kind, data_24));
    assert(value_equal(s_kind, data_25));
    assert(value_equal(data_22, data_23));
    assert(value_equal(data_22, data_24));
    assert(value_equal(data_22, data_25));
    assert(value_equal(data_23, data_24));
    assert(value_equal(data_23, data_25));
    assert(value_equal(data_24, data_25));

    BYTE data_30[] = { array_0 };
    BYTE data_31[] = { array_n, n_1, n_0 };
    BYTE data_32[] = { array, n_0 };
    BYTE data_33[] = { array, n_16,
        utf8, n_4, 'k', 'i', 'n', 'd',
        utf8, n_8, 0xEF, 0xBB, 0xBF, 'a', 'c', 't', 'o', 'r' };
    BYTE data_34[] = { array, n_16,
        utf16, n_10, 0xFE, 0xFF, '\0', 'k', '\0', 'i', '\0', 'n', '\0', 'd',
        utf16, n_12, 0xFF, 0xFE, 'a', '\0', 'c', '\0', 't', '\0', 'o', '\0', 'r', '\0' };

    assert(value_equal(data_30, data_30));
    assert(value_equal(data_30, data_31));
    assert(value_equal(data_30, data_32));
    assert(value_equal(data_31, data_32));
    assert(value_equal(data_32, data_32));
    assert(value_equal(data_33, data_34));

    return 0;
}

static int test_C_language() {
    assert(sizeof(WORD) >= 4);  // require at least 32-bit machine words
    BYTE b = 0;
    assert((BYTE)(b + 1) == 0x01);
    assert((BYTE)(b - 1) == 0xFF);
    b = -1;
    assert((BYTE)(b + 1) == 0x00);
    assert((++b) == 0x00);
    assert((--b) == 0xFF);
    assert((MAX_WORD + 1) == 0);
    assert((MAX_BYTE + 1) == 256);
    return 0;
}

static int run_test_suite() {
    return test_C_language()
        || test_bytecode_types()
        || test_parse_integer()
        || test_parse_string()
        || test_parse_codepoint()
        || test_parse_array()
        || test_value_equal()
        ;
}

int main(int argc, char *argv[]) {
    log_config.level = LOG_LEVEL_WARN;
    memo_clear();
    int result = run_test_suite();  // pass == 0, fail != 0
    return (exit(result), result);
}
