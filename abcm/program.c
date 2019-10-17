/*
 * program.c -- bootstrap program
 */
#include <assert.h>

#include "program.h"
#include "bose.h"
#include "equiv.h"
#include "print.h"
#include "abcm.h"

#define LOG_ALL // enable all logging
#include "log.h"

BYTE bootstrap[] = {
    0x06, 0x10, 0x82, 0xeb, 0x00, 0x81, 0x07, 0x10, 0x82, 0xe5, 0x00, 0x84, 0x0a, 0x84, 0x6b, 0x69,  // ···ë·····å····ki
    0x6e, 0x64, 0x0a, 0x8d, 0x61, 0x63, 0x74, 0x6f, 0x72, 0x5f, 0x73, 0x70, 0x6f, 0x6e, 0x73, 0x6f,  // nd··actor_sponso
    0x72, 0x0a, 0x86, 0x61, 0x63, 0x74, 0x6f, 0x72, 0x73, 0xe3, 0x0a, 0x86, 0x65, 0x76, 0x65, 0x6e,  // r··actorsã··even
    0x74, 0x73, 0x10, 0x82, 0xe7, 0x03, 0x0a, 0x86, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x06, 0x10,  // ts··ç···script··
    0x82, 0xad, 0x00, 0x82, 0x07, 0xd4, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x89, 0x6c,  // ·­···Ô···kind··l
    0x6f, 0x67, 0x5f, 0x70, 0x72, 0x69, 0x6e, 0x74, 0x0a, 0x85, 0x6c, 0x65, 0x76, 0x65, 0x6c, 0x80,  // og_print··level·
    0x0a, 0x85, 0x76, 0x61, 0x6c, 0x75, 0x65, 0x07, 0xb1, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64,  // ··value·±···kind
    0x0a, 0x8c, 0x65, 0x78, 0x70, 0x72, 0x5f, 0x6c, 0x69, 0x74, 0x65, 0x72, 0x61, 0x6c, 0x0a, 0x84,  // ··expr_literal··
    0x74, 0x79, 0x70, 0x65, 0x0a, 0x86, 0x53, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x0a, 0x85, 0x63, 0x6f,  // type··String··co
    0x6e, 0x73, 0x74, 0x0a, 0x85, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x07, 0xd4, 0x83, 0x0a, 0x84, 0x6b,  // nst··Hello·Ô···k
    0x69, 0x6e, 0x64, 0x0a, 0x89, 0x6c, 0x6f, 0x67, 0x5f, 0x70, 0x72, 0x69, 0x6e, 0x74, 0x0a, 0x85,  // ind··log_print··
    0x6c, 0x65, 0x76, 0x65, 0x6c, 0x80, 0x0a, 0x85, 0x76, 0x61, 0x6c, 0x75, 0x65, 0x07, 0xb1, 0x83,  // level···value·±·
    0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8c, 0x65, 0x78, 0x70, 0x72, 0x5f, 0x6c, 0x69, 0x74,  // ··kind··expr_lit
    0x65, 0x72, 0x61, 0x6c, 0x0a, 0x84, 0x74, 0x79, 0x70, 0x65, 0x0a, 0x86, 0x53, 0x74, 0x72, 0x69,  // eral··type··Stri
    0x6e, 0x67, 0x0a, 0x85, 0x63, 0x6f, 0x6e, 0x73, 0x74, 0x0a, 0x85, 0x57, 0x6f, 0x72, 0x6c, 0x64   // ng··const··World
};

BYTE object_has_kind(parse_t * parse, DATA_PTR kind) {
    LOG_TRACE("object_has_kind @", (WORD)kind);
    assert((parse->type & T_Base) == T_Object);
    parse_t prop_parse = {
        .base = parse->base + (parse->end - parse->value),  // start of property data
        .size = parse->value,
        .start = 0
    };
    if (!object_get_property(&prop_parse, s_kind)) {
        LOG_DEBUG("object_has_kind: missing 'kind' property @", (WORD)s_kind);
        return false;  // missing property
    }
    LOG_TRACE("object_has_kind: found 'kind' property", prop_parse.start);
    if (!value_equiv(kind, prop_parse.base + prop_parse.start)) {
        LOG_DEBUG("object_has_kind: mismatch", prop_parse.start);
        return false;  // kind mismatch
    }
    LOG_DEBUG("object_has_kind: success!", parse->end);
    return true;  // success!
}

BYTE actor_exec(parse_t * parse) {  // execute actor commands (actions -> effects)
    LOG_TRACE("actor_exec @", (WORD)parse);
    assert((parse->type & T_Base) == T_Object);
    DUMP_PARSE("command", parse);
    if (!parse_print(parse)) return false;  // print failed!
    newline();
    parse_t kind_parse = {
        .base = parse->base + (parse->end - parse->value),  // start of command properties
        .size = parse->value,
        .start = 0
    };
    if (!object_get_property(&kind_parse, s_kind)) {
        LOG_DEBUG("actor_exec: missing 'kind' property @", (WORD)s_kind);
        return false;  // missing property
    }
    LOG_TRACE("actor_exec: found 'kind' property", kind_parse.start);
    DUMP_PARSE("kind_parse", &kind_parse);
    return true;  // success!
}

int run_actor_script(parse_t * parse) {
    LOG_TRACE("run_actor_script @", (WORD)parse);
    assert((parse->type & T_Base) == T_Array);
    parse_t cmd_parse = {
        .base = parse->base + (parse->end - parse->value),  // start of script actions
        .size = parse->value,
        .start = 0
    };
    while (cmd_parse.start < cmd_parse.size) {
        if (!parse_object(&cmd_parse)) {
            LOG_WARN("run_actor_script: command object required!", cmd_parse.start);
            return 1;  // command object required!
        }
        if (!actor_exec(&cmd_parse)) {
            LOG_WARN("run_actor_script: failed executing command!", cmd_parse.start);
            return 1;  // failed executing command!
        }
        // get next command
        cmd_parse.start = cmd_parse.end;
    }
    return 0;  // success!
}

int run_actor_config(parse_t * parse) {
    LOG_TRACE("run_actor_config @", (WORD)parse);
    DUMP_PARSE("configuration", parse);
    if (!parse_print(parse)) return 1;  // print failed!
    assert((parse->type & T_Base) == T_Object);
    parse_t prop_parse = {
        .base = parse->base + (parse->end - parse->value),  // start of property data
        .size = parse->value,
        .start = 0
    };
    if (object_get_property(&prop_parse, s_actors) && parse_integer(&prop_parse)) {
        LOG_INFO("run_actor_config: actors =", prop_parse.value);
    }
    prop_parse.start = 0;  // always search from the beginning...
    if (object_get_property(&prop_parse, s_events) && parse_integer(&prop_parse)) {
        LOG_INFO("run_actor_config: events =", prop_parse.value);
    }
    prop_parse.start = 0;  // always search from the beginning...
    if (object_get_property(&prop_parse, s_script)) {
        LOG_INFO("run_actor_config: script =", prop_parse.value);
        return run_actor_script(&prop_parse);
    }
    return 0;  // success!
}

int run_program(DATA_PTR program) {
    LOG_INFO("bootstrap", (WORD)program);
    memo_clear();
    parse_t parse = {
        .base = program,
        .size = MAX_WORD,  // don't know how big program will be
        .start = 0
    };
    if (!parse_array(&parse)) {
        LOG_WARN("run_program: top-level array required!", parse.start);
        return 1;  // top-level array required!
    }
    // reset parsing cursor to iterate over array elements
    parse.base += (parse.end - parse.value);
    parse.size = parse.value;
    parse.start = 0;
    while (parse.start < parse.size) {
        if (!parse_object(&parse) || !object_has_kind(&parse, k_actor_sponsor)) {
            LOG_WARN("run_program: sponsor object required!", parse.start);
            return 1;  // sponsor object required!
        }
        if (run_actor_config(&parse) != 0) {
            LOG_WARN("run_program: failed to start actor configuration!", parse.start);
            return 1;  // failed to start!
        }
        parse.start = parse.end;
    }
    return 0;  // success
}
