/*
 * object.c -- Object (Dictionary) operations
 */
#include <string.h>  // for memcpy, et. al.
//#include <assert.h>

#include "object.h"
#include "bose.h"
#include "equiv.h"
#include "sponsor.h"

//#define LOG_ALL // enable all logging
#define LOG_INFO
#define LOG_WARN
#include "log.h"

BYTE object_count(DATA_PTR object, WORD * count) {
    LOG_TRACE("object_count @", (WORD)object);
    parse_t parse = {
        .base = object,
        .size = MAX_WORD,  // don't know how big object will be
        .start = 0
    };
    if (!parse_object(&parse)) {
        LOG_WARN("object_count: bad object", (WORD)object);
        return false;  // bad object
    }
    if (parse.value == 0) {  // empty object short-cut
        LOG_DEBUG("object_count: empty object", (WORD)object);
        *count = 0;
        return true;  // success!
    }
    WORD n = parse.count;
    if (!(parse.type & T_Counted)) {
        parse.size = parse.end;  // limit to object contents
        parse.start = (parse.end - parse.value);  // adjust to start of properties
        if (!object_property_count(&parse, &n)) return false;  // property count failed!
    }
    *count = n;
    LOG_TRACE("object_count: count =", *count);
    return true;  // success!
}

BYTE object_has(DATA_PTR object, DATA_PTR name) {
    LOG_TRACE("object_has @", (WORD)object);
    parse_t parse = {
        .base = object,
        .size = MAX_WORD,  // don't know how big object will be
        .start = 0
    };
    if (!parse_object(&parse)) {
        LOG_WARN("object_has: bad object", (WORD)object);
        return false;  // bad object
    }
    if (parse.value == 0) {  // empty object short-cut
        LOG_DEBUG("object_has: empty object", (WORD)object);
        return false;  // not found.
    }
    parse.size = parse.end;  // limit to object contents
    parse.start = parse.end - parse.value;  // reset to start of property data
    parse_t name_parse = {
        .base = name,
        .size = MAX_WORD,  // don't know how big name will be
        .start = 0
    };
    if (!parse_string(&name_parse)) {
        LOG_WARN("object_has: bad name", (WORD)name);
        return false;  // bad name
    }
    while (parse.start < parse.size) {
        LOG_TRACE("object_has: property start =", parse.start);
        if (!parse_string(&parse)) {
            LOG_WARN("object_has: bad property name", parse.start);
            return false;  // bad property name
        }
        if (parse_equiv(&name_parse, &parse)) {
            LOG_DEBUG("object_has: found!", (WORD)name);
            return true;  // FOUND!
        }
        parse.start = parse.end;
        LOG_TRACE("object_has: value start =", parse.start);
        if (!parse_value(&parse)) {
            LOG_WARN("object_has: bad property value", parse.start);
            return false;  // bad property value
        }
        parse.start = parse.end;
    }
    LOG_DEBUG("object_has: not found.", (WORD)name);
    return false;  // not found.
};

BYTE object_get(DATA_PTR object, DATA_PTR name, DATA_PTR * value) {
    LOG_TRACE("object_get @", (WORD)object);
    parse_t parse = {
        .base = object,
        .size = MAX_WORD,  // don't know how big object will be
        .start = 0
    };
    if (!parse_object(&parse)) {
        LOG_WARN("object_get: bad object", (WORD)object);
        return false;  // bad object
    }
    if (parse.value == 0) {  // empty object short-cut
        LOG_DEBUG("object_get: empty object", (WORD)object);
        return false;  // not found.
    }
    parse.size = parse.end;  // limit to object contents
    parse.start = parse.end - parse.value;  // reset to start of property data
    parse_t name_parse = {
        .base = name,
        .size = MAX_WORD,  // don't know how big name will be
        .start = 0
    };
    if (!parse_string(&name_parse)) {
        LOG_WARN("object_get: bad name", (WORD)name);
        return false;  // bad name
    }
    BYTE found = false;
    while (parse.start < parse.size) {
        LOG_TRACE("object_get: property start =", parse.start);
        if (!parse_string(&parse)) {
            LOG_WARN("object_get: bad property name", parse.start);
            return false;  // bad property name
        }
        if (parse_equiv(&name_parse, &parse)) {
            LOG_DEBUG("object_get: found!", (WORD)name);
            found = true;
        }
        parse.start = parse.end;
        LOG_TRACE("object_get: value start =", parse.start);
        if (!parse_value(&parse)) {
            LOG_WARN("object_get: bad property value", parse.start);
            return false;  // bad property value
        }
        if (found) {
            *value = parse.base + parse.start;
            LOG_DEBUG("object_get: value @", (WORD)*value);
            return true;  // FOUND!
        }
        parse.start = parse.end;
    }
    LOG_DEBUG("object_get: not found.", (WORD)name);
    return false;  // not found.
};

BYTE object_add(DATA_PTR object, DATA_PTR name, DATA_PTR value, DATA_PTR * new) {
    LOG_TRACE("object_add @", (WORD)object);
    LOG_TRACE("object_add: name @", (WORD)name);
    LOG_TRACE("object_add: value @", (WORD)value);
    parse_t parse = {
        .base = object,
        .size = MAX_WORD,  // don't know how big object will be
        .start = 0
    };
    if (!parse_object(&parse)) {
        LOG_WARN("object_add: bad object", (WORD)object);
        return false;  // bad object
    }
    //DUMP_PARSE("object", &parse);
    parse_t name_parse = {
        .base = name,
        .size = MAX_WORD,  // don't know how big name will be
        .start = 0
    };
    if (!parse_string(&name_parse)) {
        LOG_WARN("object_add: bad name", (WORD)name);
        return false;  // bad name
    }
    //DUMP_PARSE("name", &name_parse);
    parse_t value_parse = {
        .base = value,
        .size = MAX_WORD,  // don't know how big value will be
        .start = 0
    };
    if (!parse_value(&value_parse)) {
        LOG_WARN("object_add: bad value", (WORD)value);
        return false;  // bad value
    }
    //DUMP_PARSE("value", &value_parse);

    /* allocate space for new object */
    DATA_PTR data;
    WORD size = 8;  // margin for size/count growth
    size += (parse.end - parse.start);  // plus size of object
    size += (name_parse.end - name_parse.start);  // plus size of name
    size += (value_parse.end - value_parse.start);  // plus size of value
    LOG_TRACE("object_add: allocation size =", size);
    if (size > 0xFFFF) {  // FIXME: this should be a configurable limit somewhere, but code below depends on it...
        LOG_WARN("object_add: object too large", size);
        return false;  // object too large!
    }
    if (!RESERVE(&data, size)) return false;  // out of memory!
    WORD offset = 0;
    data[offset++] = object_n;  // 0: counted object
    data[offset++] = p_int_0;   // 1: size field
    data[offset++] = n_2;       // 2:   2 bytes
    data[offset++] = 0;         // 3:   size (LSB)
    data[offset++] = 0;         // 4:   size (MSB)
    data[offset++] = p_int_0;   // 5: count field
    data[offset++] = n_2;       // 6:   2 bytes
    data[offset++] = 0;         // 7:   count (LSB)
    data[offset++] = 0;         // 8:   count (MSB)

    /* copy name/value for new property */
    size = name_parse.end - name_parse.start;
    LOG_TRACE("object_add: name size =", size);
    memcpy(data + offset, name_parse.base + name_parse.start, size);
    offset += size;
    size = value_parse.end - value_parse.start;
    LOG_TRACE("object_add: value size =", size);
    memcpy(data + offset, value_parse.base + value_parse.start, size);
    offset += size;
    WORD count = 1;  // count new property first

    /* copy original object properties, filtering duplicate name */
    parse.size = parse.end;  // limit to object contents
    parse.start = parse.end - parse.value;  // reset to start of property data
    while (parse.start < parse.size) {
        BYTE found = false;
        LOG_TRACE("object_add: property start =", parse.start);
        if (!parse_string(&parse)) {
            LOG_WARN("object_add: bad property name", parse.start);
            RELEASE(&data);  // free memory on failure
            return false;  // bad property name
        }
        if (parse_equiv(&name_parse, &parse)) {
            LOG_DEBUG("object_add: duplicate!", (WORD)name);
            found = true;
        } else {
            size = parse.end - parse.start;
            LOG_TRACE("object_add: name size =", size);
            memcpy(data + offset, parse.base + parse.start, size);
            offset += size;
        }
        parse.start = parse.end;
        LOG_TRACE("object_add: value start =", parse.start);
        if (!parse_value(&parse)) {
            LOG_WARN("object_add: bad property value", parse.start);
            RELEASE(&data);  // free memory on failure
            return false;  // bad property value
        }
        if (!found) {
            size = parse.end - parse.start;
            LOG_TRACE("object_add: value size =", size);
            memcpy(data + offset, parse.base + parse.start, size);
            offset += size;
            ++count;  // increment property count
        }
        parse.start = parse.end;
    }

    /* fill in size/count and return new object */
    LOG_TRACE("object_add: final offset =", offset);
    size = offset - 5;  // subtract header byte count
    LOG_TRACE("object_add: final size =", size);
    LOG_TRACE("object_add: final count =", count);
    data[3] = size & 0xFF;  // size (LSB)
    data[4] = size >> 8;    // size (MSB)
    data[7] = count & 0xFF; // count (LSB)
    data[8] = count >> 8;   // count (MSB)
    *new = data;
    LOG_DEBUG("object_add: new object @", (WORD)data);
    return true;  // success!
};

BYTE object_concat(DATA_PTR left, DATA_PTR right, DATA_PTR * new) {
    LOG_TRACE("object_concat: left @", (WORD)left);
    LOG_TRACE("object_concat: right @", (WORD)right);
    parse_t left_parse = {
        .base = left,
        .size = MAX_WORD,  // don't know how big left will be
        .start = 0
    };
    if (!parse_object(&left_parse)) {
        LOG_WARN("object_concat: bad left object", (WORD)left);
        return false;  // bad left object
    }
    IF_TRACE({
        DUMP_PARSE("left", &left_parse);
    });
    parse_t right_parse = {
        .base = right,
        .size = MAX_WORD,  // don't know how big right will be
        .start = 0
    };
    if (!parse_object(&right_parse)) {
        LOG_WARN("object_concat: bad right object", (WORD)right);
        return false;  // bad right object
    }
    IF_TRACE({
        DUMP_PARSE("right", &right_parse);
    });
    WORD count = right_parse.count;  // count all properties from right
    if (!(right_parse.type & T_Counted)) {
        parse_t prop_parse = right_parse;
        prop_parse.size = prop_parse.end;  // limit to object contents
        prop_parse.start = (prop_parse.end - prop_parse.value);  // adjust to start of properties
        if (!object_property_count(&prop_parse, &count)) return false;  // property count failed!
    }
    LOG_TRACE("object_concat: right count =", count);

    /* allocate space for new object */
    DATA_PTR data;
    WORD size = 8;  // margin for size/count growth
    size += right_parse.value;  // plus size of right content
    size += left_parse.value;  // plus size of left content
    LOG_TRACE("object_concat: allocation size =", size);
    if (size > 0xFFFF) {  // FIXME: this should be a configurable limit somewhere, but code below depends on it...
        LOG_WARN("object_concat: object too large", size);
        return false;  // object too large!
    }
    if (!RESERVE(&data, size)) return false;  // out of memory!
    WORD offset = 0;
    data[offset++] = object_n;  // 0: counted object
    data[offset++] = p_int_0;   // 1: size field
    data[offset++] = n_2;       // 2:   2 bytes
    data[offset++] = 0;         // 3:   size (LSB)
    data[offset++] = 0;         // 4:   size (MSB)
    data[offset++] = p_int_0;   // 5: count field
    data[offset++] = n_2;       // 6:   2 bytes
    data[offset++] = 0;         // 7:   count (LSB)
    data[offset++] = 0;         // 8:   count (MSB)

    /* copy all properties from right */
    size = right_parse.value;
    LOG_TRACE("object_concat: right size =", size);
    memcpy(data + offset, right_parse.base + right_parse.end - size, size);
    offset += size;

    /* copy properties from left, filtering duplicates (right overrides left) */
    parse_t parse = left_parse;
    parse.size = parse.end;  // limit to object contents
    parse.start = parse.end - parse.value;  // reset to start of property data
    while (parse.start < parse.size) {
        BYTE found = false;
        LOG_TRACE("object_concat: property start =", parse.start);
        if (!parse_string(&parse)) {
            LOG_WARN("object_concat: bad property name", parse.start);
            RELEASE(&data);  // free memory on failure
            return false;  // bad property name
        }
        if (object_has(right, parse.base + parse.start)) {
            LOG_DEBUG("object_concat: duplicate!", (WORD)parse.base + parse.start);
            found = true;
        } else {
            size = parse.end - parse.start;
            LOG_TRACE("object_concat: name size =", size);
            memcpy(data + offset, parse.base + parse.start, size);
            offset += size;
        }
        parse.start = parse.end;
        LOG_TRACE("object_concat: value start =", parse.start);
        if (!parse_value(&parse)) {
            LOG_WARN("object_concat: bad property value", parse.start);
            RELEASE(&data);  // free memory on failure
            return false;  // bad property value
        }
        if (!found) {
            size = parse.end - parse.start;
            LOG_TRACE("object_concat: value size =", size);
            memcpy(data + offset, parse.base + parse.start, size);
            offset += size;
            ++count;  // increment property count
        }
        parse.start = parse.end;
    }

    /* fill in size/count and return new object */
    LOG_TRACE("object_concat: final offset =", offset);
    size = offset - 5;  // subtract header byte count
    LOG_TRACE("object_concat: final size =", size);
    LOG_TRACE("object_concat: final count =", count);
    data[3] = size & 0xFF;  // size (LSB)
    data[4] = size >> 8;    // size (MSB)
    data[7] = count & 0xFF; // count (LSB)
    data[8] = count >> 8;   // count (MSB)
    *new = data;
    LOG_DEBUG("object_concat: new object @", (WORD)data);
    return true;  // success!
};
