/*
 * program.c -- bootstrap program
 */
#include <assert.h>

#include "program.h"
#include "bose.h"
#include "equiv.h"
#include "abcm.h"

#define LOG_ALL // enable all logging
#include "log.h"

BYTE bootstrap[] = {
    0x06, 0x10, 0x82, 0xf2, 0x00, 0x81, 0x07, 0x10, 0x82, 0xec, 0x00, 0x84, 0x0a, 0x84, 0x6b, 0x69,  // ···ò·····ì····ki
    0x6e, 0x64, 0x0a, 0x8d, 0x61, 0x63, 0x74, 0x6f, 0x72, 0x5f, 0x73, 0x70, 0x6f, 0x6e, 0x73, 0x6f,  // nd··actor_sponso
    0x72, 0x0a, 0x86, 0x61, 0x63, 0x74, 0x6f, 0x72, 0x73, 0x80, 0x0a, 0x86, 0x65, 0x76, 0x65, 0x6e,  // r··actors···even
    0x74, 0x73, 0x81, 0x0a, 0x86, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x06, 0x10, 0x82, 0xb7, 0x00,  // ts···script·····
    0x82, 0x07, 0xd9, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x89, 0x6c, 0x6f, 0x67, 0x5f,  // ··Ù···kind··log_
    0x70, 0x72, 0x69, 0x6e, 0x74, 0x0a, 0x85, 0x6c, 0x65, 0x76, 0x65, 0x6c, 0x0a, 0x84, 0x49, 0x4e,  // print··level··IN
    0x46, 0x4f, 0x0a, 0x85, 0x76, 0x61, 0x6c, 0x75, 0x65, 0x07, 0xb1, 0x83, 0x0a, 0x84, 0x6b, 0x69,  // FO··value·±···ki
    0x6e, 0x64, 0x0a, 0x8c, 0x65, 0x78, 0x70, 0x72, 0x5f, 0x6c, 0x69, 0x74, 0x65, 0x72, 0x61, 0x6c,  // nd··expr_literal
    0x0a, 0x84, 0x74, 0x79, 0x70, 0x65, 0x0a, 0x86, 0x53, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x0a, 0x85,  // ··type··String··
    0x63, 0x6f, 0x6e, 0x73, 0x74, 0x0a, 0x85, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x07, 0xd9, 0x83, 0x0a,  // const··Hello·Ù··
    0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x89, 0x6c, 0x6f, 0x67, 0x5f, 0x70, 0x72, 0x69, 0x6e, 0x74,  // ·kind··log_print
    0x0a, 0x85, 0x6c, 0x65, 0x76, 0x65, 0x6c, 0x0a, 0x84, 0x49, 0x4e, 0x46, 0x4f, 0x0a, 0x85, 0x76,  // ··level··INFO··v
    0x61, 0x6c, 0x75, 0x65, 0x07, 0xb1, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8c, 0x65,  // alue·±···kind··e
    0x78, 0x70, 0x72, 0x5f, 0x6c, 0x69, 0x74, 0x65, 0x72, 0x61, 0x6c, 0x0a, 0x84, 0x74, 0x79, 0x70,  // xpr_literal··typ
    0x65, 0x0a, 0x86, 0x53, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x0a, 0x85, 0x63, 0x6f, 0x6e, 0x73, 0x74,  // e··String··const
    0x0a, 0x85, 0x57, 0x6f, 0x72, 0x6c, 0x64                                                         // ··World
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
        //DUMP_PARSE("cmd_parse", &cmd_parse);
        parse_t prop_parse = {
            .base = cmd_parse.base + (cmd_parse.end - cmd_parse.value),  // start of property data
            .size = cmd_parse.value,
            .start = 0
        };
        if (!object_get_property(&prop_parse, s_kind)) {
            LOG_DEBUG("run_actor_script: missing 'kind' property @", (WORD)s_kind);
            return false;  // missing property
        }
        LOG_TRACE("run_actor_script: found 'kind' property", prop_parse.start);
        DUMP_PARSE("kind_parse", &prop_parse);
        // get next command
        cmd_parse.start = cmd_parse.end;
    }
    return 0;  // success!
}

int run_actor_config(parse_t * parse) {
    LOG_TRACE("run_actor_config @", (WORD)parse);
    assert((parse->type & T_Base) == T_Object);
    parse_t prop_parse = {
        .base = parse->base + (parse->end - parse->value),  // start of property data
        .size = parse->value,
        .start = 0
    };
    if (object_get_property(&prop_parse, s_actors)) {
        LOG_INFO("run_actor_config: actors =", prop_parse.value);
    }
    prop_parse.start = 0;  // always search from the beginning...
    if (object_get_property(&prop_parse, s_events)) {
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
