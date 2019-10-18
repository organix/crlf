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

#define HEXDUMP_ANNOTATION 0 /* dump bose-encoded bytes for collection values */

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

static void hex_print(BYTE b) {
    static char * hex = "0123456789ABCDEF";
    print(hex[(b >> 4) & 0xF]);
    print(hex[b & 0xF]);
}

#if HEXDUMP_ANNOTATION
static void hexdump(DATA_PTR data, WORD size) {
    for (WORD i = 0; i < size; ++i) {
        print(' ');
        hex_print(data[i]);
        if (i > 9) {
            print(' ');
            print('~');
            break;  // size-limited dump cut-off
        }
    }
}
#endif

static void space(WORD indent) {  // space between values
    if (indent) {
        newline();
        while (--indent) {
            print(' ');
            print(' ');
        }
    } else {
        print(' ');
    }
}

static BYTE null_print(parse_t * parse) {
    print('n');
    print('u');
    print('l');
    print('l');
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
    return true;  // success!
}

static BYTE number_print(parse_t * parse) {
    // print parsed value known to be a Number
    print('0');
    print('x');
    // FIXME: JSON requires base-10, but we only generate base-16 here...
    if (!(parse->type & T_Sized)) {
        // direct-coded (small) integer
        hex_print(parse->value);
        return true;  // success!
    }
    if (parse->type & T_Counted) {
        LOG_WARN("number_print: no Unum support", parse->type);
        return false;  // not an Integer type
    }
    WORD end = parse->end;  // offset past end of number data
    WORD start = end - parse->value;  // offset to start of number data
    if (start == end) {
        hex_print(0);  // special case for zero bit-count
        return true;
    }
    while (start < end--) {
        hex_print(parse->base[end]);
/* byte separator...
        if (start != end) {
            print('_');
        }
*/
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

static BYTE array_print(parse_t * parse, WORD indent) {
    // print parsed value known to be an Array
    assert(parse->start < (parse->end - parse->value));
    print('[');
    if (parse->value == 0) {  // empty array short-cut
        print(']');
        return true;
    }
    if (indent) {
#if HEXDUMP_ANNOTATION
        print(' '); print(' '); print('/'); print('/');
        hexdump(parse->base + parse->start, (parse->end - parse->start) - parse->value);
        print(' '); print('.'); print('.'); print('.');
#endif
        space(++indent);
    }
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
        if (!parse_print(&item_parse, indent)) return false;  // print failed!
        if (item_parse.end < item_parse.size) {
            print(',');
#if HEXDUMP_ANNOTATION
            if (indent && ((item_parse.prefix < 0x02) || (item_parse.prefix > 0x07))) {  // not Arrays or Objects
                print(' '); print(' '); print('/'); print('/');
                hexdump(item_parse.base + item_parse.start, item_parse.end - item_parse.start);
            }
#endif
            space(indent);
#if HEXDUMP_ANNOTATION
        } else if (indent && ((item_parse.prefix < 0x02) || (item_parse.prefix > 0x07))) {  // not Arrays or Objects
            print(' '); print(' '); print('/'); print('/');
            hexdump(item_parse.base + item_parse.start, item_parse.end - item_parse.start);
#endif
        }
        item_parse.start = item_parse.end;
    }
    if (indent) {
        space(--indent);
    }
    print(']');
    return true;  // success!
}

static BYTE object_print(parse_t * parse, WORD indent) {
    // print parsed value known to be an Object
    assert(parse->start < (parse->end - parse->value));
    print('{');
    if (parse->value == 0) {  // empty object short-cut
        print('}');
        return true;
    }
    if (indent) {
#if HEXDUMP_ANNOTATION
        print(' '); print(' '); print('/'); print('/');
        hexdump(parse->base + parse->start, (parse->end - parse->start) - parse->value);
        print(' '); print('.'); print('.'); print('.');
#endif
        space(++indent);
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
        if (indent) {
            print(' ');
        }
        prop_parse.start = prop_parse.end;
        //WORD value_start = prop_parse.start;
        if (!parse_value(&prop_parse)) {
            LOG_WARN("object_print: bad property value", prop_parse.start);
            return false;  // bad property value
        }
        if (!parse_print(&prop_parse, indent)) return false;  // print failed!
        if (prop_parse.end < prop_parse.size) {
            print(',');
#if HEXDUMP_ANNOTATION
            if (indent && ((prop_parse.prefix < 0x02) || (prop_parse.prefix > 0x07))) {  // not Arrays or Objects
                print(' '); print(' '); print('/'); print('/');
                hexdump(prop_parse.base + prop_parse.start, prop_parse.end - prop_parse.start);
            }
#endif
            space(indent);
#if HEXDUMP_ANNOTATION
        } else if (indent && ((prop_parse.prefix < 0x02) || (prop_parse.prefix > 0x07))) {  // not Arrays or Objects
            print(' '); print(' '); print('/'); print('/');
            hexdump(prop_parse.base + prop_parse.start, prop_parse.end - prop_parse.start);
#endif
        }
        prop_parse.start = prop_parse.end;
    }
    if (indent) {
        space(--indent);
    }
    print('}');
    return true;  // success!
}

BYTE parse_print(parse_t * parse, WORD indent) {
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
            return array_print(parse, indent);
        }
        case T_Object: {
            return object_print(parse, indent);
        }
        default: {
            LOG_WARN("parse_print: bad type", (parse->type & T_Base));
            return false;  // bad type
        }
    }
    return true;  // success!
}

BYTE value_print(DATA_PTR value, WORD indent) {
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
    BYTE ok = parse_print(&parse, indent);
    newline();
    return ok;
};
