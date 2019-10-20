/*
 * object.c -- Object (Dictionary) operations
 */
//#include <assert.h>

#include "object.h"
#include "bose.h"
#include "equiv.h"

//#define LOG_ALL // enable all logging
#define LOG_INFO
#define LOG_WARN
#include "log.h"

/*
    { "kind":"actor_state", "name":<string> }
    { "kind":"dict_get", "name":<string>, "in":<dictionary> }
    { "kind":"actor_has_state", "name":<string> }
    { "kind":"dict_has", "name":<string>, "in":<dictionary> }
    { "kind":"actor_message" }
    { "kind":"dict_bind", "name":<string>, "value":<expression>, "with":<dictionary> }
*/

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
        return false;  // not found.
    }
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
        if (!parse_string(&parse)) {
            LOG_WARN("object_has: bad property name", parse.start);
            return false;  // bad property name
        }
        if (parse_equiv(&name_parse, &parse)) {
            return true;  // FOUND!
        }
        parse.start = parse.end;
        if (!parse_value(&parse)) {
            LOG_WARN("object_has: bad property value", parse.start);
            return false;  // bad property value
        }
        parse.start = parse.end;
    }
    return false;  // not found.
};

BYTE object_get(DATA_PTR object, DATA_PTR name, DATA_PTR * value) {
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
        return false;  // not found.
    }
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
    BYTE found = false;
    while (parse.start < parse.size) {
        if (!parse_string(&parse)) {
            LOG_WARN("object_has: bad property name", parse.start);
            return false;  // bad property name
        }
        if (parse_equiv(&name_parse, &parse)) {
            found = true;
        }
        parse.start = parse.end;
        if (!parse_value(&parse)) {
            LOG_WARN("object_has: bad property value", parse.start);
            return false;  // bad property value
        }
        if (found) {
            *value = parse.base + parse.start;
            return true;  // FOUND!
        }
        parse.start = parse.end;
    }
    return false;  // not found.
};

/*
static BYTE object_print(parse_t * parse) {
    // print parsed value known to be an Object
    assert(parse->start < (parse->end - parse->value));
    print('{');
    if (parse->value == 0) {  // empty object short-cut
        print('}');
        return true;
    }
    parse_t prop_parse = {
        .base = parse->base + (parse->end - parse->value),  // start of property data
        .size = parse->value,
        .start = 0
    };
    while (prop_parse.start < prop_parse.size) {
        //WORD key_start = prop_parse.start;
        if (!parse_string(&prop_parse)) {
            LOG_WARN("object_print: bad property key", prop_parse.start);
            return false;  // bad property key
        }
        if (!string_print(&prop_parse)) return false;  // print failed!
        print(':');
        prop_parse.start = prop_parse.end;
        //WORD value_start = prop_parse.start;
        if (!parse_value(&prop_parse)) {
            LOG_WARN("object_print: bad property value", prop_parse.start);
            return false;  // bad property value
        }
        if (!parse_print(&prop_parse)) return false;  // print failed!
        if (prop_parse.end < prop_parse.size) {
            print(',');
        }
        prop_parse.start = prop_parse.end;
    }
    print('}');
    return true;  // success!
}
*/
