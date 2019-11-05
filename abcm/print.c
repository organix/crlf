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

#define HEXDUMP_ANNOTATION 1 /* dump bose-encoded bytes for collection values */
#define ANSI_COLOR_OUTPUT 1 /* use ansi terminal escape sequences to set output colors */

#include <stdio.h>
// FIXME: implement non-stdio output primitives
#define OUTPUT stdout

void print(WORD unicode) {
    if ((unicode == '\t')
    ||  (unicode == '\n')
    ||  ((unicode >= 0x20) && (unicode < 0x7F))) {
        fputc(unicode, OUTPUT);
    } else if (unicode >= 0xA0) {
        fputc('~', OUTPUT);  // replacement character
    }
}

void prints(char * cstring) {
    WORD c;
    while (cstring && (c = (WORD)*cstring++)) {
        print(c);
    }
}

void newline() {
    fputc('\n', OUTPUT);
    fflush(OUTPUT);
}

typedef enum {
    Black = '0',    // ^[[30m
    Red,            // ^[[31m
    Green,          // ^[[32m
    Yellow,         // ^[[33m
    Blue,           // ^[[34m
    Magenta,        // ^[[35m
    Cyan,           // ^[[36m
    White           // ^[[37m
} color_t;

#if ANSI_COLOR_OUTPUT

#define NUM_COLOR   Green       // number
#define TEXT_COLOR  Yellow      // string
#define MEMO_COLOR  Red         // memo marker
#define PRIM_COLOR  Magenta     // primitive/capability
#define PUNCT_COLOR Cyan        // punctuation
#define DUMP_COLOR  Blue        // hexdump

#define ESC 0x1B

void set_color(color_t color) {
    fputc(ESC, OUTPUT);
    fputc('[', OUTPUT);
    fputc('3', OUTPUT);
    fputc(color, OUTPUT);
    fputc('m', OUTPUT);
}
void clear_color() {
    fputc(ESC, OUTPUT);
    fputc('[', OUTPUT);
    fputc('m', OUTPUT);
}

#else

#define set_color(c) /* set_color disabled */
#define clear_color() /* clear_color disabled */

#endif

static BYTE int_print(parse_t * parse) {
    long n = 0;
    if (parse->type & T_Counted) {
        LOG_WARN("number_print: no Unum support", parse->type);
        return false;  // not an Integer type
    }
    if (parse->type & T_Sized) {
        WORD end = parse->end;  // offset past end of number data
        WORD start = end - parse->value;  // offset to start of number data
        if (start == end) {
            print('0');  // special case for zero bit-count
            return true;
        }
        if ((end - start) > sizeof(long)) {
            return false;  // number too big for `long` representation
        }
        BYTE sign = (parse->type & T_Negative) ? 0xFF : 0x00;
        int shift = 0;
        while (shift < (8 * sizeof(long))) {
            BYTE b = (start < end) ? parse->base[start++] : sign;
            n |= (b << shift);
            shift += 8;
        }
        if ((sign == 0) && (n < 0)) {
            return false;  // positive number exceeds signed range
        }
    } else {
        // direct-coded (small) integer
        n = (long)(parse->value);
    }
    set_color(NUM_COLOR);
    fprintf(OUTPUT, "%ld", n);  // FIXME!
    clear_color();
    return true;  // success!
}

void hex_byte(BYTE b) {
    static char * hex = "0123456789ABCDEF";
    print(hex[(b >> 4) & 0xF]);
    print(hex[b & 0xF]);
}

void hex_word(WORD w) {
    WORD shift = sizeof(WORD) * 8;
    for (WORD i = 0; i < sizeof(WORD); ++i) {
        shift -= 8;
        hex_byte(w >> shift);
    }
}

void hex_dump(DATA_PTR data, WORD size) {
    for (WORD i = 0; i < size; ++i) {
        print(' ');
        hex_byte(data[i]);
        if (i > 9) {
            prints(" ~");
            break;  // size-limited dump cut-off
        }
    }
}

void data_dump(DATA_PTR data, WORD size) {
    for (WORD i = 0; i < size; ++i) {
        print(' ');
        BYTE b = data[i];
        if ((b >= 0x20) && (b < 0x7F)) {
            print(b);
        } else {
            hex_byte(b);
        }
        if (i > 12) {
            prints(" ~~");
            break;  // size-limited dump cut-off
        }
    }
}

void memo_print(DATA_PTR data, WORD size) {
    set_color(MEMO_COLOR);
    print('<');
    data_dump(data, size);
    prints(" >\n");
    clear_color();
}

static void space(WORD indent) {  // space between values
    if (indent) {
        newline();
        while (--indent) {
            prints("  ");
        }
    } else {
        print(' ');
    }
}

static BYTE null_print(parse_t * parse) {
    set_color(PRIM_COLOR);
    prints("null");
    clear_color();
    return true;  // success!
}

static BYTE boolean_print(parse_t * parse) {
    set_color(PRIM_COLOR);
    if (parse->prefix) {
        prints("true");
    } else {
        prints("false");
    }
    clear_color();
    return true;  // success!
}

static BYTE number_print(parse_t * parse) {
    // print parsed value known to be a Number
    if (int_print(parse)) {
        return true;  // base-10 works for int-sized values...
    }
    // FIXME: JSON requires base-10, but we generate base-16 for numbers that are bigger than `int`...
    set_color(NUM_COLOR);
    prints("0x");
    WORD end = parse->end;  // offset past end of number data
    WORD start = end - parse->value;  // offset to start of number data
    while (start < end--) {
        hex_byte(parse->base[end]);
/* byte separator...
        if (start != end) {
            print('_');
        }
*/
    }
    clear_color();
    return true;  // success!
}

static BYTE string_print(parse_t * parse) {
    // print parsed value known to be a String
    if (parse->prefix == mem_ref) {
#if HEXDUMP_ANNOTATION
        set_color(MEMO_COLOR);
        print('(');
        hex_byte(parse->value);
        print(')');
        clear_color();
#endif
        parse_t mem_parse;
        if (!value_parse((DATA_PTR)parse->count, &mem_parse)) return false;
        parse_print(&mem_parse, 0);
        return true;  // success!
    }
    assert(parse->start < (parse->end - parse->value));
    if (parse->prefix == octets) {  // special case: raw octets or capability data
        set_color(TEXT_COLOR);
        print('<');
        if (parse->type & T_Capability) {
            set_color(PRIM_COLOR);
            print('@');
            set_color(TEXT_COLOR);
        }
        DATA_PTR data = parse->base + (parse->end - parse->value);
        for (WORD i = 0; i < parse->value; ++i) {
            print(' ');
            hex_byte(data[i]);
        }
        prints(" >");
        clear_color();
        return true;  // success!
    }
    set_color(TEXT_COLOR);
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
    clear_color();
    return true;  // success!
}

static BYTE array_print(parse_t * parse, WORD indent) {
    // print parsed value known to be an Array
    assert(parse->start < (parse->end - parse->value));
    set_color(PUNCT_COLOR);
    print('[');
    if (parse->value == 0) {  // empty array short-cut
        print(']');
        clear_color();
#if HEXDUMP_ANNOTATION
        if (indent) {
            set_color(DUMP_COLOR);
            prints("  //");
            hex_dump(parse->base + parse->start, (parse->end - parse->start) - parse->value);
            clear_color();
        }
#endif
        return true;
    }
    if (indent) {
#if HEXDUMP_ANNOTATION
        set_color(DUMP_COLOR);
        prints("  //");
        hex_dump(parse->base + parse->start, (parse->end - parse->start) - parse->value);
        prints(" ...");
        clear_color();
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
            set_color(PUNCT_COLOR);
            print(',');
            clear_color();
#if HEXDUMP_ANNOTATION
            if (indent && ((item_parse.prefix < 0x02) || (item_parse.prefix > 0x07))) {  // not Arrays or Objects
                set_color(DUMP_COLOR);
                prints("  //");
                hex_dump(item_parse.base + item_parse.start, item_parse.end - item_parse.start);
                clear_color();
            }
#endif
            space(indent);
#if HEXDUMP_ANNOTATION
        } else if (indent && ((item_parse.prefix < 0x02) || (item_parse.prefix > 0x07))) {  // not Arrays or Objects
            set_color(DUMP_COLOR);
            prints("  //");
            hex_dump(item_parse.base + item_parse.start, item_parse.end - item_parse.start);
            clear_color();
#endif
        }
        item_parse.start = item_parse.end;
    }
    if (indent) {
        space(--indent);
    }
    set_color(PUNCT_COLOR);
    print(']');
    clear_color();
    return true;  // success!
}

static BYTE object_print(parse_t * parse, WORD indent) {
    // print parsed value known to be an Object
    assert(parse->start < (parse->end - parse->value));
    set_color(PUNCT_COLOR);
    print('{');
    if (parse->value == 0) {  // empty object short-cut
        print('}');
        clear_color();
#if HEXDUMP_ANNOTATION
        if (indent) {
            set_color(DUMP_COLOR);
            prints("  //");
            hex_dump(parse->base + parse->start, (parse->end - parse->start) - parse->value);
            clear_color();
        }
#endif
        return true;
    }
    if (indent) {
#if HEXDUMP_ANNOTATION
        set_color(DUMP_COLOR);
        prints("  //");
        hex_dump(parse->base + parse->start, (parse->end - parse->start) - parse->value);
        prints(" ...");
        clear_color();
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
        set_color(PUNCT_COLOR);
        print(':');
        clear_color();
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
            set_color(PUNCT_COLOR);
            print(',');
            clear_color();
#if HEXDUMP_ANNOTATION
            if (indent && ((prop_parse.prefix < 0x02) || (prop_parse.prefix > 0x07))) {  // not Arrays or Objects
                set_color(DUMP_COLOR);
                prints("  //");
                hex_dump(prop_parse.base + prop_parse.start, prop_parse.end - prop_parse.start);
                clear_color();
            }
#endif
            space(indent);
#if HEXDUMP_ANNOTATION
        } else if (indent && ((prop_parse.prefix < 0x02) || (prop_parse.prefix > 0x07))) {  // not Arrays or Objects
            set_color(DUMP_COLOR);
            prints("  //");
            hex_dump(prop_parse.base + prop_parse.start, prop_parse.end - prop_parse.start);
            clear_color();
#endif
        }
        prop_parse.start = prop_parse.end;
    }
    if (indent) {
        space(--indent);
    }
    set_color(PUNCT_COLOR);
    print('}');
    clear_color();
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
    parse_t parse;
    if (!value_parse(value, &parse)) {
        LOG_WARN("value_print: bad value", (WORD)value);
        return false;  // bad value
    }
    BYTE ok = parse_print(&parse, indent);
#if HEXDUMP_ANNOTATION
    if (indent && ((parse.prefix < 0x02) || (parse.prefix > 0x07))) {  // not Arrays or Objects
        set_color(DUMP_COLOR);
        prints("  //");
        hex_dump(parse.base + parse.start, parse.end - parse.start);
        clear_color();
    }
#endif
    newline();
    return ok;
};
