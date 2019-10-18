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
    0x06, 0x10, 0x82, 0x96, 0x01, 0x81, 0x07, 0x10, 0x82, 0x90, 0x01, 0x84, 0x0a, 0x84, 0x6b, 0x69,  // ··············ki
    0x6e, 0x64, 0x0a, 0x8d, 0x61, 0x63, 0x74, 0x6f, 0x72, 0x5f, 0x73, 0x70, 0x6f, 0x6e, 0x73, 0x6f,  // nd··actor_sponso
    0x72, 0x0a, 0x86, 0x61, 0x63, 0x74, 0x6f, 0x72, 0x73, 0xe3, 0x0a, 0x86, 0x65, 0x76, 0x65, 0x6e,  // r··actorsã··even
    0x74, 0x73, 0x10, 0x82, 0xe7, 0x03, 0x0a, 0x86, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x06, 0x10,  // ts··ç···script··
    0x82, 0x58, 0x01, 0x82, 0x07, 0x10, 0x82, 0xfc, 0x00, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64,  // ·X·····ü····kind
    0x0a, 0x8a, 0x61, 0x63, 0x74, 0x6f, 0x72, 0x5f, 0x73, 0x65, 0x6e, 0x64, 0x0a, 0x87, 0x6d, 0x65,  // ··actor_send··me
    0x73, 0x73, 0x61, 0x67, 0x65, 0x07, 0x93, 0x81, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8a,  // ssage·····kind··
    0x64, 0x69, 0x63, 0x74, 0x5f, 0x65, 0x6d, 0x70, 0x74, 0x79, 0x0a, 0x85, 0x61, 0x63, 0x74, 0x6f,  // dict_empty··acto
    0x72, 0x07, 0x10, 0x82, 0xbf, 0x00, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8c, 0x61,  // r···¿····kind··a
    0x63, 0x74, 0x6f, 0x72, 0x5f, 0x63, 0x72, 0x65, 0x61, 0x74, 0x65, 0x0a, 0x85, 0x73, 0x74, 0x61,  // ctor_create··sta
    0x74, 0x65, 0x07, 0x93, 0x81, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8a, 0x64, 0x69, 0x63,  // te·····kind··dic
    0x74, 0x5f, 0x65, 0x6d, 0x70, 0x74, 0x79, 0x0a, 0x88, 0x62, 0x65, 0x68, 0x61, 0x76, 0x69, 0x6f,  // t_empty··behavio
    0x72, 0x07, 0x10, 0x82, 0x7f, 0x00, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8e, 0x61,  // r········kind··a
    0x63, 0x74, 0x6f, 0x72, 0x5f, 0x62, 0x65, 0x68, 0x61, 0x76, 0x69, 0x6f, 0x72, 0x0a, 0x84, 0x6e,  // ctor_behavior··n
    0x61, 0x6d, 0x65, 0x0f, 0x0a, 0x86, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x06, 0xd7, 0x81, 0x07,  // ame···script·×··
    0xd4, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x89, 0x6c, 0x6f, 0x67, 0x5f, 0x70, 0x72,  // Ô···kind··log_pr
    0x69, 0x6e, 0x74, 0x0a, 0x85, 0x6c, 0x65, 0x76, 0x65, 0x6c, 0x81, 0x0a, 0x85, 0x76, 0x61, 0x6c,  // int··level···val
    0x75, 0x65, 0x07, 0xb1, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8c, 0x65, 0x78, 0x70,  // ue·±···kind··exp
    0x72, 0x5f, 0x6c, 0x69, 0x74, 0x65, 0x72, 0x61, 0x6c, 0x0a, 0x84, 0x74, 0x79, 0x70, 0x65, 0x0a,  // r_literal··type·
    0x86, 0x53, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x0a, 0x85, 0x63, 0x6f, 0x6e, 0x73, 0x74, 0x0a, 0x85,  // ·String··const··
    0x57, 0x6f, 0x72, 0x6c, 0x64, 0x07, 0xd4, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x89,  // World·Ô···kind··
    0x6c, 0x6f, 0x67, 0x5f, 0x70, 0x72, 0x69, 0x6e, 0x74, 0x0a, 0x85, 0x6c, 0x65, 0x76, 0x65, 0x6c,  // log_print··level
    0x81, 0x0a, 0x85, 0x76, 0x61, 0x6c, 0x75, 0x65, 0x07, 0xb1, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e,  // ···value·±···kin
    0x64, 0x0a, 0x8c, 0x65, 0x78, 0x70, 0x72, 0x5f, 0x6c, 0x69, 0x74, 0x65, 0x72, 0x61, 0x6c, 0x0a,  // d··expr_literal·
    0x84, 0x74, 0x79, 0x70, 0x65, 0x0a, 0x86, 0x53, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x0a, 0x85, 0x63,  // ·type··String··c
    0x6f, 0x6e, 0x73, 0x74, 0x0a, 0x85, 0x48, 0x65, 0x6c, 0x6c, 0x6f                                 // onst··Hello
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

BYTE actor_eval(parse_t * parse, DATA_PTR * value_out) {  // evaluate actor expressions (expression -> value)
    LOG_TRACE("actor_eval @", (WORD)parse);
    assert((parse->type & T_Base) == T_Object);
    parse_t prop_parse = {
        .base = parse->base + (parse->end - parse->value),  // start of command properties
        .size = parse->value,
        .start = 0
    };
    if (!object_get_property(&prop_parse, s_kind)) {
        LOG_DEBUG("actor_eval: missing 'kind' property @", (WORD)s_kind);
        return false;  // missing property
    }
    LOG_TRACE("actor_eval: found 'kind' property", prop_parse.start);
    DATA_PTR kind = prop_parse.base + prop_parse.start;
    if (value_equiv(kind, k_expr_literal)) {
        prop_parse.start = 0;  // search from beginning of properties
        if (!object_get_property(&prop_parse, s_const)) {
            LOG_DEBUG("actor_eval: missing 'const' property @", (WORD)s_const);
            return false;  // missing property
        }
        *value_out = prop_parse.base + prop_parse.start;  // return constant value
    } else if (value_equiv(kind, k_dict_empty)) {
        *value_out = o_;  // return empty (dictionary) object
    } else {
        LOG_WARN("actor_eval: unknown 'kind' of expression", prop_parse.start);
        //if (!value_print(kind, 0)) return false;  // print failed
        if (!value_print(parse->base + parse->start, 0)) return false;  // print failed
        // FIXME: probably want to return `false` here and fail the script execution, but we just ignore it...
        *value_out = v_null;
    }
    return true;  // success!
}

BYTE actor_exec(parse_t * parse) {  // execute actor commands (action -> effects)
    LOG_TRACE("actor_exec @", (WORD)parse);
    assert((parse->type & T_Base) == T_Object);
/*
    DUMP_PARSE("command", parse);
    if (!parse_print(parse, 0)) return false;  // print failed!
    newline();
*/
    parse_t prop_parse = {
        .base = parse->base + (parse->end - parse->value),  // start of command properties
        .size = parse->value,
        .start = 0
    };
    if (!object_get_property(&prop_parse, s_kind)) {
        LOG_DEBUG("actor_exec: missing 'kind' property @", (WORD)s_kind);
        return false;  // missing property
    }
    LOG_TRACE("actor_exec: found 'kind' property", prop_parse.start);
    DATA_PTR kind = prop_parse.base + prop_parse.start;
    if (value_equiv(kind, k_log_print)) {
        prop_parse.start = 0;  // search from beginning of properties
        if (!object_get_property(&prop_parse, s_value)) {
            LOG_DEBUG("actor_exec: missing 'value' property @", (WORD)s_value);
            return false;  // missing property
        }
        DATA_PTR value;
        if (!actor_eval(&prop_parse, &value)) return false;  // evaluation failed!
        if (!value_print(value, 0)) return false;  // print failed
    } else if (value_equiv(kind, k_actor_ignore)) {
        LOG_DEBUG("actor_exec: ignore action @", (WORD)k_actor_ignore);
    } else if (value_equiv(kind, k_actor_fail)) {
        prop_parse.start = 0;  // search from beginning of properties
        if (!object_get_property(&prop_parse, s_value)) {
            LOG_DEBUG("actor_exec: missing 'value' property @", (WORD)s_value);
            return false;  // missing property
        }
        DATA_PTR value;
        if (!actor_eval(&prop_parse, &value)) {
            LOG_INFO("actor_exec: DOUBLE-FAULT! @", (WORD)&prop_parse);
            if (!value_print(prop_parse.base + prop_parse.start, 0)) return false;  // print failed
            return false;  // evaluation failed!
        }
        // FIXME: probably want this to be a WARN, and not always print the error value...
        LOG_INFO("actor_exec: FAIL!", (WORD)value);
        if (!value_print(value, 0)) return false;  // print failed
        return false;  // force failure!
    } else {
        LOG_WARN("actor_exec: unknown 'kind' of command", prop_parse.start);
        //if (!value_print(kind, 0)) return false;  // print failed
        if (!value_print(parse->base + parse->start, 0)) return false;  // print failed
        // FIXME: probably want to return `false` here and fail the script execution, but we just ignore it...
    }
    return true;  // success!
}

/*
 * WARNING! All the `run_...` procedures return 0 on success, and 1 on failure. (not true/false)
 */

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
    LOG_INFO("run_actor_script: completed successfully", cmd_parse.end);
    return 0;  // success!
}

int run_actor_config(parse_t * parse) {
    LOG_TRACE("run_actor_config @", (WORD)parse);
    if (!parse_print(parse, 1)) return 1;  // print failed!
    newline();
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
    LOG_INFO("run_program: total number of bytes", parse.value);
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
