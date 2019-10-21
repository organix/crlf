/*
 * object.c -- Object (Dictionary) operations
 */
//#include <assert.h>

#include "object.h"
#include "bose.h"
#include "equiv.h"

#define LOG_ALL // enable all logging
//#define LOG_INFO
//#define LOG_WARN
#include "log.h"

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
        LOG_WARN("object_get: empty object", (WORD)object);
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
