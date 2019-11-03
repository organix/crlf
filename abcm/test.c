/*
 * test.c -- unit test suite
 */
#include <stddef.h>  // for NULL, size_t, et. al.
#include <assert.h>

#include "test.h"
#include "bose.h"
#include "equiv.h"
#include "abcm.h"
#include "print.h"
#include "sponsor.h"
#include "array.h"
#include "object.h"

#define LOG_ALL // enable all logging
//#define LOG_INFO
//#define LOG_WARN
#include "log.h"

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

    BYTE data[] = { null };
    parse_t parse = {
        .base = data,
        .size = sizeof(data),
        .start = 0
    };
    assert(parse_value(&parse));
    LOG_TRACE("test_bytecode_types: null payload", parse.value);
    assert((parse.end - parse.start) == 1);
    assert(parse.value == 0);
    parse_t cursor = parse;  // structure assignment makes a field-by-field copy
    assert(cursor.base == parse.base);
    assert(cursor.type == parse.type);
    assert(cursor.end == parse.end);
    cursor.start = (cursor.end - cursor.value);  // set cursor to start of variadic data (none, in this case)
    assert(cursor.start > parse.start);

    return 0;
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

static int test_parse_codepoint() {
    parse_t string_parse;
    parse_t code_parse;
    int i;
    WORD w;

    WORD code[] = { 0x0061, 0x0063, 0x0074, 0x006F, 0x0072 };
    BYTE data[] = { utf8, n_5, 'a', 'c', 't', 'o', 'r' };
    string_parse.base = data;
    string_parse.size = sizeof(data);
    string_parse.start = 0;
    if (!parse_string(&string_parse)) return 1;  // bad String
    if (string_parse.prefix == mem_ref) {  // redirect to memo table entry
        /*
         * NOTE: There are no memo entries during testing because there is no sponsor,
         *       but this is a good example of how to redirect to the memo table for String values.
         */
        LOG_DEBUG("test_parse_codepoint: memo index =", string_parse.value);
        if (!value_parse((DATA_PTR)string_parse.count, &string_parse)) return 1;  // bad memo!
    }
    code_parse.base = string_parse.base + (string_parse.end - string_parse.value);  // start of codepoint data
    code_parse.size = string_parse.value;
    code_parse.prefix = string_parse.prefix;
    code_parse.type = string_parse.type;
    code_parse.start = 0;
    i = 0;
    while (code_parse.start < code_parse.size) {
        if (!parse_codepoint(&code_parse)) return 1;  // bad codepoint
        LOG_DEBUG("test_parse_codepoint: codepoint =", code_parse.value);
        w = code[i++];
        assert(code_parse.value == w);
        code_parse.start = code_parse.end;
    }
    assert(i == (sizeof(code) / sizeof(WORD)));  // make sure we used all the results

    WORD code16[] = { 0xDF62, 0xD852, 0x20AC, 0x24B62 };
    BYTE data16[] = { utf16, n_10, 0xDF, 0x62, 0xD8, 0x52, 0x20, 0xAC, 0xD8, 0x52, 0xDF, 0x62 };
    string_parse.base = data16;
    string_parse.size = sizeof(data16);
    string_parse.start = 0;
    if (!parse_string(&string_parse)) return 1;  // bad String
    if (string_parse.prefix == mem_ref) {  // redirect to memo table entry
        /*
         * NOTE: There are no memo entries during testing because there is no sponsor,
         *       but this is a good example of how to redirect to the memo table for String values.
         */
        LOG_DEBUG("test_parse_codepoint: memo index =", string_parse.value);
        if (!value_parse((DATA_PTR)string_parse.count, &string_parse)) return 1;  // bad memo!
    }
    code_parse.base = string_parse.base + (string_parse.end - string_parse.value);  // start of codepoint data
    code_parse.size = string_parse.value;
    code_parse.prefix = string_parse.prefix;
    code_parse.type = string_parse.type;
    code_parse.start = 0;
    i = 0;
    while (code_parse.start < code_parse.size) {
        if (!parse_codepoint(&code_parse)) return 1;  // bad codepoint
        LOG_DEBUG("test_parse_codepoint: codepoint =", code_parse.value);
        w = code16[i++];
        assert(code_parse.value == w);
        code_parse.start = code_parse.end;
    }
    assert(i == (sizeof(code16) / sizeof(WORD)));  // make sure we used all the results

    return 0;
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

static int test_parse_object() {
    BYTE data[] = {
        object_0,
        object, n_8, utf8, n_5, 'v', 'a', 'l', 'u', 'e', n_42,
        object_n, n_9, n_1, utf8, n_5, 'v', 'a', 'l', 'u', 'e', n_42,
        object_n, n_1, n_0,
        object, n_0
    };
    parse_t parse = {
        .base = data,
        .size = sizeof(data),
        .start = 0
    };

    assert(parse_object(&parse) == true);
    assert((parse.end - parse.start) == 1);
    assert(parse.value == 0);

    parse.start = parse.end;
    assert(parse_object(&parse) == true);
    assert((parse.end - parse.start) == 10);
    assert(parse.value == 8);
    assert(parse.count == 0);

    parse.start = parse.end;
    assert(parse_object(&parse) == true);
    assert((parse.end - parse.start) == 11);
    assert(parse.value == 8);
    assert(parse.count == 1);

    parse.start = parse.end;
    assert(parse_object(&parse) == true);
    assert((parse.end - parse.start) == 3);
    assert(parse.value == 0);

    parse.start = parse.end;
    assert(parse_object(&parse) == true);
    assert((parse.end - parse.start) == 2);
    assert(parse.value == 0);

    assert(parse.end == parse.size);  // used up all the data
    return 0;
}

static int test_object_property_count() {
    BYTE data_0[] = { object_0 };
    BYTE data_1[] = { object, n_8, utf8, n_5, 'v', 'a', 'l', 'u', 'e', n_42 };
    BYTE data_2[] = { object_n, n_12, n_2, utf8, n_1, 'x', m_int_5, n_1, 0xFE, utf16, n_2, '\0', 'y', n_3 };
    BYTE data_3[] = { object_n, n_1, n_0 };
    BYTE data_4[] = { object, n_0 };
    BYTE data_5[] = { object, n_40,
        utf8, n_7, 'B', 'o', 'o', 'l', 'e', 'a', 'n', object, n_15,
            utf8, n_4, 't', 'r', 'u', 'e', true,
            utf8, n_5, 'f', 'a', 'l', 's', 'e', false,
        utf8, n_4, 'z', 'e', 'r', 'o', n_0,
        utf8, n_4, 'N', 'u', 'l', 'l', null
    };
    parse_t parse;
    WORD count;

    parse.base = data_0;
    parse.size = sizeof(data_0);
    parse.start = 0;
    assert(parse_value(&parse));
    LOG_TRACE("test_object_property_count: Object data_0 payload", parse.value);
    parse.start = (parse.end - parse.value);
    assert(object_property_count(&parse, &count));
    assert(count == 0);

    parse.base = data_1;
    parse.size = sizeof(data_1);
    parse.start = 0;
    assert(parse_value(&parse));
    LOG_TRACE("test_object_property_count: Object data_1 payload", parse.value);
    parse.start = (parse.end - parse.value);
    assert(object_property_count(&parse, &count));
    assert(count == 1);

    parse.base = data_2;
    parse.size = sizeof(data_2);
    parse.start = 0;
    assert(parse_value(&parse));
    LOG_TRACE("test_object_property_count: Object data_2 payload", parse.value);
    parse.start = (parse.end - parse.value);
    assert(object_property_count(&parse, &count));
    assert(count == 2);

    parse.base = data_3;
    parse.size = sizeof(data_3);
    parse.start = 0;
    assert(parse_value(&parse));
    LOG_TRACE("test_object_property_count: Object data_3 payload", parse.value);
    parse.start = (parse.end - parse.value);
    assert(object_property_count(&parse, &count));
    assert(count == 0);

    parse.base = data_4;
    parse.size = sizeof(data_4);
    parse.start = 0;
    assert(parse_value(&parse));
    LOG_TRACE("test_object_property_count: Object data_4 payload", parse.value);
    parse.start = (parse.end - parse.value);
    assert(object_property_count(&parse, &count));
    assert(count == 0);

    parse.base = data_5;
    parse.size = sizeof(data_5);
    parse.start = 0;
    assert(parse_value(&parse));
    LOG_TRACE("test_object_property_count: Object data_5 payload", parse.value);
    parse.start = (parse.end - parse.value);
    assert(object_property_count(&parse, &count));
    assert(count == 3);

    return 0;
}

static int test_value_equiv() {
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

    assert(value_equiv(data_0, data_0));
    assert(value_equiv(data_0, data_1));
    assert(value_equiv(data_0, data_2));
    assert(value_equiv(data_0, data_3));
    assert(value_equiv(data_2, data_3));
    assert(value_equiv(data_4, data_5));
    assert(value_equiv(data_6, data_7));
    assert(value_equiv(data_8, data_9));

    BYTE data_10[] = { string_0 };
    BYTE data_11[] = { octets, n_0 };
    BYTE data_12[] = { utf8, n_0 };
    BYTE data_13[] = { utf8, n_3, 0xEF, 0xBB, 0xBF };
    BYTE data_14[] = { utf16, n_2, 0xFE, 0xFF };
    BYTE data_15[] = { utf16, n_2, 0xFF, 0xFE };

    assert(value_equiv(s_, s_));
    assert(value_equiv(s_, data_10));
    assert(value_equiv(s_, data_11));
    assert(value_equiv(s_, data_12));
    assert(value_equiv(s_, data_13));
    assert(value_equiv(s_, data_14));
    assert(value_equiv(s_, data_15));
    assert(value_equiv(data_13, data_14));
    assert(value_equiv(data_13, data_15));
    assert(value_equiv(data_14, data_15));

    assert(!value_equiv(s_kind, s_));
    assert(!value_equiv(s_kind, s_actor));

    BYTE data_20[] = { octets, n_4, 'k', 'i', 'n', 'd' };
    BYTE data_21[] = { utf8, n_4, 'k', 'i', 'n', 'd' };
    BYTE data_22[] = { utf8, n_7, 0xEF, 0xBB, 0xBF, 'k', 'i', 'n', 'd' };
    BYTE data_23[] = { utf16, n_8, '\0', 'k', '\0', 'i', '\0', 'n', '\0', 'd' };
    BYTE data_24[] = { utf16, n_10, 0xFE, 0xFF, '\0', 'k', '\0', 'i', '\0', 'n', '\0', 'd' };
    BYTE data_25[] = { utf16, n_10, 0xFF, 0xFE, 'k', '\0', 'i', '\0', 'n', '\0', 'd', '\0' };

    assert(value_equiv(s_kind, data_20));
    assert(value_equiv(s_kind, data_21));
    assert(value_equiv(s_kind, data_22));
    assert(value_equiv(s_kind, data_23));
    assert(value_equiv(s_kind, data_24));
    assert(value_equiv(s_kind, data_25));
    assert(value_equiv(data_22, data_23));
    assert(value_equiv(data_22, data_24));
    assert(value_equiv(data_22, data_25));
    assert(value_equiv(data_23, data_24));
    assert(value_equiv(data_23, data_25));
    assert(value_equiv(data_24, data_25));

    BYTE data_30[] = { array_0 };
    BYTE data_31[] = { array_n, n_1, n_0 };
    BYTE data_32[] = { array, n_0 };
    BYTE data_33[] = { array, n_19,
        utf8, n_4, 'k', 'i', 'n', 'd',
        utf8, n_8, 0xEF, 0xBB, 0xBF, 'a', 'c', 't', 'o', 'r',
        true, false, null };
    BYTE data_34[] = { array, n_29,
        utf16, n_10, 0xFE, 0xFF, '\0', 'k', '\0', 'i', '\0', 'n', '\0', 'd',
        utf16, n_12, 0xFF, 0xFE, 'a', '\0', 'c', '\0', 't', '\0', 'o', '\0', 'r', '\0',
        true, false, null };

    assert(value_equiv(data_30, data_30));
    assert(value_equiv(data_30, data_31));
    assert(value_equiv(data_30, data_32));
    assert(value_equiv(data_31, data_32));
    assert(value_equiv(data_32, data_32));
    assert(value_equiv(data_33, data_34));

    BYTE data_40[] = { object_0 };
    BYTE data_41[] = { object, n_8, utf8, n_5, 'v', 'a', 'l', 'u', 'e', n_42 };
    BYTE data_42[] = { object_n, n_9, n_1, utf8, n_5, 'v', 'a', 'l', 'u', 'e', n_42 };
    BYTE data_43[] = { object, n_10, utf8, n_1, 'x', n_m2, utf8, n_1, 'y', p_int_3, n_1, 0x03 };
    BYTE data_44[] = { object_n, n_12, n_2, utf8, n_1, 'x', m_int_5, n_1, 0xFE, utf16, n_2, '\0', 'y', n_3 };
    BYTE data_45[] = { object_n, n_1, n_0 };
    BYTE data_46[] = { object, n_0 };

    assert(value_equiv(data_40, data_40));
    assert(value_equiv(data_41, data_42));
    assert(value_equiv(data_43, data_44));
    assert(value_equiv(data_40, data_45));
    assert(value_equiv(data_40, data_46));
    assert(value_equiv(data_45, data_46));

    return 0;
}

static int test_sponsor() {
    assert(sponsor);
    LOG_DEBUG("test_sponsor: sponsor =", (WORD)sponsor);

    DATA_PTR orig = s_actor;
    DATA_PTR copy;
    assert(COPY(&copy, orig));
    //assert(value_print(copy, 1));
    assert(orig != copy);
    assert(value_equiv(orig, copy));
    //assert(sponsor_share(sponsor, &copy));
    //assert(sponsor_release(sponsor, &copy));
    assert(RELEASE(&copy));
    assert(!copy);

    config_t * config = SPONSOR_CONFIG(sponsor);
    LOG_DEBUG("test_sponsor: config =", (WORD)config);
    DATA_PTR actor;
    assert(!config_create(config, NULL, o_, v_null, &actor));  // no budget for actors
    assert(!config_send(config, actor, o_));  // no budget for message-events
    assert(!config_dispatch(config));  // no message-events to deliver

    return 0;
}

static int test_array() {
    BYTE data_0[] = { array_0 };
    BYTE data_1[] = { array_n, n_1, n_0 };
    BYTE data_2[] = { array, n_0 };
    BYTE data_3[] = { array, n_4, true, false, n_0, null };
    BYTE data_4[] = { array_n, n_3, n_2, n_0, n_m1 };
    BYTE data_5[] = { array, n_29,
        utf16, n_10, 0xFE, 0xFF, '\0', 'k', '\0', 'i', '\0', 'n', '\0', 'd',
        utf16, n_12, 0xFF, 0xFE, 'a', '\0', 'c', '\0', 't', '\0', 'o', '\0', 'r', '\0',
        true, false, null };
    WORD length;
    DATA_PTR value;

    IF_TRACE(value_print(data_0, 0));
    assert(array_length(data_0, &length));
    assert(length == 0);

    IF_TRACE(value_print(data_1, 0));
    assert(array_length(data_1, &length));
    assert(length == 0);

    IF_TRACE(value_print(data_2, 0));
    assert(array_length(data_2, &length));
    assert(length == 0);

    IF_TRACE(value_print(data_3, 1));
    assert(array_length(data_3, &length));
    LOG_DEBUG("test_array: data_3.length =", length);
    assert(length == 4);
    assert(array_get(data_3, 0, &value));
    assert(value_equiv(value, b_true));
    assert(array_get(data_3, 1, &value));
    assert(value_equiv(value, b_false));
    assert(array_get(data_3, 2, &value));
    assert(value_equiv(value, i_0));
    assert(array_get(data_3, 3, &value));
    assert(value_equiv(value, v_null));
    assert(!array_get(data_3, 4, &value));

    IF_TRACE(value_print(data_4, 1));
    assert(array_length(data_4, &length));
    LOG_DEBUG("test_array: data_4.length =", length);
    assert(length == 2);
    assert(array_get(data_4, 0, &value));
    assert(value_equiv(value, i_0));
    assert(array_get(data_4, 1, &value));
    assert(!array_get(data_4, 2, &value));

    IF_TRACE(value_print(data_5, 1));
    assert(array_length(data_5, &length));
    LOG_DEBUG("test_array: data_5.length =", length);
    assert(length == 5);

/*
ADD x AT 0 TO [a, b, c] --> [x, a, b, c]
ADD x AT 1 TO [a, b, c] --> [a, x, b, c]
ADD x AT 2 TO [a, b, c] --> [a, b, x, c]
ADD x AT 3 TO [a, b, c] --> [a, b, c, x]
*/
    BYTE data_10[] = { array, n_3, n_1, n_2, n_3 };
    BYTE data_11[] = { array, n_4, n_0, n_1, n_2, n_3 };
    BYTE data_12[] = { array, n_4, n_1, n_0, n_2, n_3 };
    BYTE data_13[] = { array, n_4, n_1, n_2, n_0, n_3 };
    BYTE data_14[] = { array, n_4, n_1, n_2, n_3, n_0 };

    assert(array_add(data_10, i_0, 0, &value));
    assert(value_equiv(value, data_11));
    assert(RELEASE(&value));

    assert(array_add(data_10, i_0, 1, &value));
    assert(value_equiv(value, data_12));
    assert(RELEASE(&value));

    assert(array_add(data_10, i_0, 2, &value));
    assert(value_equiv(value, data_13));
    assert(RELEASE(&value));

    assert(array_add(data_10, i_0, 3, &value));
    assert(value_equiv(value, data_14));
    assert(array_length(value, &length));
    LOG_DEBUG("test_array: value.length =", length);
    assert(length == 4);
    assert(RELEASE(&value));

    assert(!array_add(data_10, i_0, 4, &value));

    // start with nothing
    LOG_DEBUG("test_array: sponsor =", (WORD)sponsor);
    DATA_PTR array;
    assert(COPY(&array, a_));
    // append 1
    assert(array_length(array, &length));
    assert(array_get(data_10, 0, &value));
    assert(array_add(array, value, length, &value));
    assert(RELEASE(&array));
    array = TRACK(value);
    // append 2
    assert(array_length(array, &length));
    assert(array_get(data_10, 1, &value));
    assert(array_add(array, value, length, &value));
    assert(RELEASE(&array));
    array = TRACK(value);
    // append 3
    assert(array_length(array, &length));
    assert(array_get(data_10, 2, &value));
    assert(array_add(array, value, length, &value));
    assert(RELEASE(&array));
    array = TRACK(value);
    // verify result
    IF_TRACE(value_print(array, 1));
    assert(value_equiv(array, data_10));
    assert(array_length(array, &length));
    LOG_DEBUG("test_array: array.length =", length);
    assert(length == 3);
    assert(RELEASE(&array));

    return 0;
}

static int test_object() {
    BYTE data_0[] = { object_0 };
    BYTE data_1[] = { object, n_8, utf8, n_5, 'v', 'a', 'l', 'u', 'e', n_42 };
    BYTE data_2[] = { object_n, n_12, n_2, utf8, n_1, 'x', m_int_5, n_1, 0xFE, utf16, n_2, '\0', 'y', n_3 };
    BYTE data_3[] = { object_n, n_1, n_0 };
    BYTE data_4[] = { object, n_0 };
    BYTE data_5[] = { object, n_31,
        utf8, n_7, 'B', 'o', 'o', 'l', 'e', 'a', 'n', array, n_2,
            true,
            false,
        utf16, n_8, '\0', 'z', '\0', 'e', '\0', 'r', '\0', 'o', n_0,
        utf8, n_4, 'N', 'u', 'l', 'l', null
    };
    DATA_PTR value;
    DATA_PTR result;
    BYTE i_42[] = { n_42 };  // integer 42 (encoded)
    BYTE s_zero[] = { utf8, n_4, 'z', 'e', 'r', 'o' };  // string "zero" (encoded)

/*
BYTE object_has(DATA_PTR object, DATA_PTR name);
BYTE object_get(DATA_PTR object, DATA_PTR name, DATA_PTR * value);
*/
    IF_TRACE(value_print(data_0, 0));
    assert(!object_has(data_0, s_kind));
    assert(!object_get(data_0, s_kind, &value));

    IF_TRACE(value_print(data_1, 0));
    assert(!object_has(data_1, s_kind));
    assert(!object_get(data_1, s_kind, &value));
    assert(object_has(data_1, s_value));
    assert(object_get(data_1, s_value, &value));
    assert(value_equiv(value, i_42));

    IF_TRACE(value_print(data_2, 0));
    assert(!object_has(data_2, s_kind));
    assert(!object_get(data_0, s_kind, &value));

    IF_TRACE(value_print(data_3, 0));
    assert(!object_has(data_3, s_kind));
    assert(!object_get(data_0, s_kind, &value));

    IF_TRACE(value_print(data_4, 0));
    assert(!object_has(data_4, s_kind));
    assert(!object_get(data_0, s_kind, &value));

    IF_TRACE(value_print(data_5, 1));
    assert(!object_has(data_5, s_kind));
    assert(object_get(data_5, s_zero, &value));
    assert(value_equiv(value, i_0));

/*
BIND "kind" TO "actor_assign" WITH {}
    --> {"kind":"actor_assign"}
BIND "name" TO "actor" WITH {"kind":"actor_assign"}
    --> {"name":"actor", "kind":"actor_assign"}
BIND "name" TO null WITH {"name":"actor", "kind":"actor_assign"}
    --> {"name":null, "kind":"actor_assign"}
BIND "kind" TO 0 WITH {"name":null, "kind":"actor_assign"}
    --> {"kind":0, "name":null}
*/
    BYTE data_10[] = { object, n_18,
        utf8, n_4, 'n', 'a', 'm', 'e', null,
        utf16, n_8, '\0', 'k', '\0', 'i', '\0', 'n', '\0', 'd', n_0 };

    // start with nothing
    LOG_DEBUG("test_object: sponsor =", (WORD)sponsor);
    DATA_PTR object;
    assert(COPY(&object, o_));
    IF_TRACE(value_print(object, 1));
    // bind "kind"
    assert(object_add(object, s_kind, k_actor_assign, &result));
    IF_TRACE(value_print(result, 1));
    assert(RELEASE(&object));
    object = result;
    // bind "name"
    assert(object_add(object, s_name, s_actor, &result));
    IF_TRACE(value_print(result, 1));
    assert(RELEASE(&object));
    object = result;
    // re-bind "name"
    assert(object_add(object, s_name, v_null, &result));
    IF_TRACE(value_print(result, 1));
    assert(RELEASE(&object));
    object = result;
    // re-bind "kind"
    assert(object_add(object, s_kind, i_0, &result));
    IF_TRACE(value_print(result, 1));
    assert(RELEASE(&object));
    object = result;
    // verify result
    assert(COPY(&result, object));
    IF_TRACE(value_print(result, 1));
    assert(object_get(object, s_name, &value));
    assert(value_equiv(value, v_null));
    assert(RELEASE(&object));
    IF_TRACE(value_print(data_10, 1));
    assert(value_equiv(result, data_10));
    assert(RELEASE(&result));

    return 0;
}

static int test_C_platform() {
    BYTE b;

    LOG_INFO("sizeof(WORD)", sizeof(WORD));
    assert(sizeof(WORD) >= 4);  // require at least 32-bit machine words
    LOG_DEBUG("sizeof(int)", sizeof(int));
    LOG_DEBUG("sizeof(long)", sizeof(long));
    b = 0;
    assert((BYTE)(b + 1) == 0x01);
    assert((BYTE)(b - 1) == 0xFF);
    b = -1;
    assert((BYTE)(b + 1) == 0x00);
    assert((++b) == 0x00);
    assert((--b) == 0xFF);
    assert((MAX_WORD + 1) == 0);
    assert((MAX_BYTE + 1) == 256);

    // report big-endian or small-endian integer representation
    BYTE utf16_BOM[] = { 0xFE, 0xFF };
    assert(sizeof(utf16_BOM) == sizeof(uint16_t));
    LOG_INFO("BOM", *((uint16_t *)utf16_BOM));
    BYTE byte_order[] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
    LOG_INFO("endianness", *((WORD *)byte_order));
    return 0;
}

int run_test_suite() {
    if (!memo_reset()) return 1;  // memo reset failed!
    return test_C_platform()
        || test_bytecode_types()
        || test_parse_integer()
        || test_parse_string()
        || test_parse_codepoint()
        || test_parse_array()
        || test_parse_object()
        || test_object_property_count()
        || test_value_equiv()
        || test_sponsor()
        || test_array()
        || test_object()
        ;
}
