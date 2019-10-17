/*
 * print.c -- console output formatting
 */
#include <assert.h>

#include "print.h"
#include "bose.h"

//#define LOG_ALL // enable all logging
#define LOG_INFO
#define LOG_WARN
#include "log.h"

#include <stdio.h>
// FIXME: implement non-stdio output primitives
void print(WORD unicode) {
    if ((unicode == '\t')
    ||  (unicode == '\n')
    ||  ((unicode >= 0x20) && (unicode < 0x7F))) {
        fputc(unicode, stdout);
    } else if (unicode >= 0xA0) {
        fputc('~', stdout);  // replacement character
    }
}

void newline() {
    fputc('\n', stdout);
    fflush(stdout);
}

static BYTE null_print(parse_t * parse) {
    print('n');
    print('u');
    print('l');
    print('l');
    //newline();
    return true;  // success!
}

static BYTE boolean_print(parse_t * parse) {
    if (parse->prefix) {
        print('t');
        print('r');
        print('u');
        print('e');
    } else {
        print('f');
        print('a');
        print('l');
        print('s');
        print('e');
    }
    //newline();
    return true;  // success!
}

static void hex_print_byte(BYTE b) {
    static char * hex = "0123456789ABCDEF";
    print(hex[(b >> 4) & 0xF]);
    print(hex[b & 0xF]);
}
static BYTE number_print(parse_t * parse) {
    // print parsed value known to be a Number
    if (!(parse->type & T_Sized)) {
        // direct-coded (small) integer
        hex_print_byte(parse->value);
        return true;  // success!
    }
    if (parse->type & T_Counted) {
        LOG_WARN("number_print: no Unum support", parse->type);
        return false;  // not an Integer type
    }
    WORD end = parse->end;  // offset past end of number data
    WORD start = end - parse->value;  // offset to start of number data
    while (start < end--) {
        hex_print_byte(parse->base[end]);
        if (start != end) {
            print('_');
        }
    }
    return true;  // success!
}

static BYTE string_print(parse_t * parse) {
    // print parsed value known to be a String
    assert(parse->start < (parse->end - parse->value));
    print('"');
    parse_t code_parse = {
        .base = parse->base + (parse->end - parse->value),  // start of codepoint data
        .size = parse->value,
        .prefix = parse->prefix,
        .type = parse->type,
        .start = 0
    };
    while (code_parse.start < code_parse.size) {
        if (!(parse_codepoint(&code_parse))) return false;  // bad codepoint
        if ((code_parse.value == '"') || (code_parse.value == '\\')) {
            print('\\');  // escape special characters
            // FIXME: print() aggressively prunes/replaces characters -- we should implement full JSON escape sequences here...
        }
        print(code_parse.value);
        code_parse.start = code_parse.end;
    }
    print('"');
    return true;  // success!
}

static BYTE array_print(parse_t * parse) {
    // print parsed value known to be an Array
    assert(parse->start < (parse->end - parse->value));
    print('[');
    parse_t item_parse = {
        .base = parse->base + (parse->end - parse->value),  // start of element data
        .size = parse->value,
        .start = 0
    };
    while (item_parse.start < item_parse.size) {
        if (!parse_value(&item_parse)) {
            LOG_WARN("array_print: bad element value", item_parse.start);
            return false;  // bad element value
        }
        if (!parse_print(&item_parse)) return false;  // print failed!
        if (item_parse.end < item_parse.size) {
            print(',');
            print(' ');
        }
        item_parse.start = item_parse.end;
    }
    print(']');
    return true;  // success!
}

static BYTE object_print(parse_t * parse) {
    // print parsed value known to be an Object
    assert(parse->start < (parse->end - parse->value));
    print('{');
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
            print(' ');
        }
        prop_parse.start = prop_parse.end;
    }
    print('}');
    return true;  // success!
}

BYTE parse_print(parse_t * parse) {
    LOG_TRACE("parse_print @", (WORD)parse);
    //DUMP_PARSE("parse_print", parse);
    switch (parse->type & T_Base) {
        case T_Null: {
            return null_print(parse);
        }
        case T_Boolean: {
            return boolean_print(parse);
        }
        case T_Number: {
            return number_print(parse);
        }
        case T_String: {
            return string_print(parse);
        }
        case T_Array: {
            return array_print(parse);
        }
        case T_Object: {
            return object_print(parse);
        }
        default: {
            LOG_WARN("parse_print: bad type", (parse->type & T_Base));
            return false;  // bad type
        }
    }
    return true;  // success!
}

BYTE value_print(DATA_PTR value) {
    LOG_TRACE("value_print @", (WORD)value);
    parse_t parse = {
        .base = value,
        .size = MAX_WORD,  // don't know how big value will be
        .start = 0
    };
    if (!parse_value(&parse)) {
        LOG_WARN("value_print: bad value", (WORD)value);
        return false;  // bad value
    }
    return parse_print(&parse);
};
