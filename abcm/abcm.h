/*
 * abcm.h -- Actor Byte-Code Machine
 */
#ifndef _ABCM_H_
#define _ABCM_H_

//#include <stdlib.h>
#include <stdint.h>
//#include <unistd.h>

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

extern type_t prefix_type[1<<8];

extern BYTE s_type_name[][10];

extern BYTE s_[];
extern BYTE s_kind[];
extern BYTE s_message[];
extern BYTE s_actor[];
extern BYTE s_behavior[];
extern BYTE s_name[];
extern BYTE s_value[];
extern BYTE s_error[];
/*
// Actions
    { "kind":"actor_send", "message":<dictionary>, "actor":<address> }
    { "kind":"actor_become", "behavior":<behavior> }
    { "kind":"actor_ignore" }
    { "kind":"actor_assign", "name":<string>, "value":<expression> }
    { "kind":"actor_fail", "error":<expression> }
*/

void memo_clear();  // reset memo table between top-level values

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
BYTE parse_from_data(parse_t * parse, DATA_PTR data);

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
BYTE parse_prefix(parse_t * parse);

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
BYTE parse_value(parse_t * parse);

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
BYTE parse_integer(parse_t * parse);

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
BYTE parse_string(parse_t * parse);

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
BYTE parse_codepoint(parse_t * parse);

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
BYTE parse_array(parse_t * parse);

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
BYTE parse_object(parse_t * parse);

BYTE object_property_count(parse_t * parse, WORD * count_ptr);
BYTE parse_equal(parse_t * x_parse, parse_t * y_parse);
BYTE value_equal(DATA_PTR x, DATA_PTR y);

#endif // _ABCM_H_
