/*
 * equal.c -- equality comparisons
 */
#include <assert.h>

#include "equal.h"
#include "abcm.h"
#include "log.h"

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
