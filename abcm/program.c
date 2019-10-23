/*
 * program.c -- bootstrap program
 */
#include <assert.h>

#include "program.h"
#include "bose.h"
#include "actor.h"
#include "equiv.h"
#include "print.h"
#include "abcm.h"

#define LOG_ALL // enable all logging
#include "log.h"

#if !USE_PARSE_CURSOR
#include "array.h"
#include "object.h"
#endif

BYTE bootstrap[] = {
    0x06, 0x10, 0x82, 0xc3, 0x01, 0x81, 0x07, 0x10, 0x82, 0xbd, 0x01, 0x84, 0x0a, 0x84, 0x6b, 0x69,  // ···Ã·····½····ki
    0x6e, 0x64, 0x0a, 0x8d, 0x61, 0x63, 0x74, 0x6f, 0x72, 0x5f, 0x73, 0x70, 0x6f, 0x6e, 0x73, 0x6f,  // nd··actor_sponso
    0x72, 0x0a, 0x86, 0x61, 0x63, 0x74, 0x6f, 0x72, 0x73, 0x81, 0x0a, 0x86, 0x65, 0x76, 0x65, 0x6e,  // r··actors···even
    0x74, 0x73, 0x81, 0x0a, 0x86, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x06, 0x10, 0x82, 0x88, 0x01,  // ts···script·····
    0x82, 0x07, 0x10, 0x82, 0x2c, 0x01, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8a, 0x61,  // ····,····kind··a
    0x63, 0x74, 0x6f, 0x72, 0x5f, 0x73, 0x65, 0x6e, 0x64, 0x0a, 0x87, 0x6d, 0x65, 0x73, 0x73, 0x61,  // ctor_send··messa
    0x67, 0x65, 0x07, 0xab, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8c, 0x65, 0x78, 0x70,  // ge·«···kind··exp
    0x72, 0x5f, 0x6c, 0x69, 0x74, 0x65, 0x72, 0x61, 0x6c, 0x0a, 0x84, 0x74, 0x79, 0x70, 0x65, 0x0a,  // r_literal··type·
    0x86, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 0x0a, 0x85, 0x63, 0x6f, 0x6e, 0x73, 0x74, 0x03, 0x0a,  // ·Object··const··
    0x85, 0x61, 0x63, 0x74, 0x6f, 0x72, 0x07, 0x10, 0x82, 0xd7, 0x00, 0x83, 0x0a, 0x84, 0x6b, 0x69,  // ·actor···×····ki
    0x6e, 0x64, 0x0a, 0x8c, 0x61, 0x63, 0x74, 0x6f, 0x72, 0x5f, 0x63, 0x72, 0x65, 0x61, 0x74, 0x65,  // nd··actor_create
    0x0a, 0x85, 0x73, 0x74, 0x61, 0x74, 0x65, 0x07, 0xab, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64,  // ··state·«···kind
    0x0a, 0x8c, 0x65, 0x78, 0x70, 0x72, 0x5f, 0x6c, 0x69, 0x74, 0x65, 0x72, 0x61, 0x6c, 0x0a, 0x84,  // ··expr_literal··
    0x74, 0x79, 0x70, 0x65, 0x0a, 0x86, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 0x0a, 0x85, 0x63, 0x6f,  // type··Object··co
    0x6e, 0x73, 0x74, 0x03, 0x0a, 0x88, 0x62, 0x65, 0x68, 0x61, 0x76, 0x69, 0x6f, 0x72, 0x07, 0x10,  // nst···behavior··
    0x82, 0x7f, 0x00, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8e, 0x61, 0x63, 0x74, 0x6f,  // ······kind··acto
    0x72, 0x5f, 0x62, 0x65, 0x68, 0x61, 0x76, 0x69, 0x6f, 0x72, 0x0a, 0x84, 0x6e, 0x61, 0x6d, 0x65,  // r_behavior··name
    0x0f, 0x0a, 0x86, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x06, 0xd7, 0x81, 0x07, 0xd4, 0x83, 0x0a,  // ···script·×··Ô··
    0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x89, 0x6c, 0x6f, 0x67, 0x5f, 0x70, 0x72, 0x69, 0x6e, 0x74,  // ·kind··log_print
    0x0a, 0x85, 0x6c, 0x65, 0x76, 0x65, 0x6c, 0x81, 0x0a, 0x85, 0x76, 0x61, 0x6c, 0x75, 0x65, 0x07,  // ··level···value·
    0xb1, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8c, 0x65, 0x78, 0x70, 0x72, 0x5f, 0x6c,  // ±···kind··expr_l
    0x69, 0x74, 0x65, 0x72, 0x61, 0x6c, 0x0a, 0x84, 0x74, 0x79, 0x70, 0x65, 0x0a, 0x86, 0x53, 0x74,  // iteral··type··St
    0x72, 0x69, 0x6e, 0x67, 0x0a, 0x85, 0x63, 0x6f, 0x6e, 0x73, 0x74, 0x0a, 0x85, 0x57, 0x6f, 0x72,  // ring··const··Wor
    0x6c, 0x64, 0x07, 0xd4, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x89, 0x6c, 0x6f, 0x67,  // ld·Ô···kind··log
    0x5f, 0x70, 0x72, 0x69, 0x6e, 0x74, 0x0a, 0x85, 0x6c, 0x65, 0x76, 0x65, 0x6c, 0x81, 0x0a, 0x85,  // _print··level···
    0x76, 0x61, 0x6c, 0x75, 0x65, 0x07, 0xb1, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8c,  // value·±···kind··
    0x65, 0x78, 0x70, 0x72, 0x5f, 0x6c, 0x69, 0x74, 0x65, 0x72, 0x61, 0x6c, 0x0a, 0x84, 0x74, 0x79,  // expr_literal··ty
    0x70, 0x65, 0x0a, 0x86, 0x53, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x0a, 0x85, 0x63, 0x6f, 0x6e, 0x73,  // pe··String··cons
    0x74, 0x0a, 0x85, 0x48, 0x65, 0x6c, 0x6c, 0x6f                                                   // t··Hello
};

#if USE_PARSE_CURSOR
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
    } else {
        LOG_WARN("actor_eval: unknown 'kind' of expression", prop_parse.start);
        //if (!value_print(kind, 0)) return false;  // print failed
        if (!value_print(parse->base + parse->start, 0)) return false;  // print failed
        // FIXME: probably want to return `false` here and fail the script execution, but we just ignore it...
        *value_out = v_null;
    }
    return true;  // success!
}
#else
static BYTE property_eval(DATA_PTR object, DATA_PTR name, DATA_PTR * value) {  // evaluate a expression found in the named object property
    LOG_TRACE("property_eval: object =", (WORD)object);
    DATA_PTR expression;
    if (!object_get(object, name, &expression)) {
        LOG_DEBUG("property_eval: missing property", (WORD)name);
        return false;  // missing property
    }
    if (!actor_eval(expression, value)) {
        LOG_WARN("property_eval: bad expression!", (WORD)expression);
        return false;  // evaluation failed!
    }
    return true;  // success!
}
BYTE actor_eval(DATA_PTR expression, DATA_PTR * value) {  // evaluate actor expressions (expression -> value)
    LOG_TRACE("actor_eval: expression", (WORD)expression);
    prints("  ");
    if (!value_print(expression, 0)) return false;  // print failed!
    DATA_PTR kind;
    if (!object_get(expression, s_kind, &kind)) {
        LOG_WARN("actor_eval: missing 'kind' property", (WORD)expression);
        return false;  // missing property
    }
    if (value_equiv(kind, k_expr_literal)) {
        // { "kind":"expr_literal", "const":<value> }
        LOG_DEBUG("actor_eval: literal expression", (WORD)kind);
        DATA_PTR constant;
        if (!object_get(expression, s_const, &constant)) {
            LOG_DEBUG("actor_eval: missing 'const' property", (WORD)expression);
            return false;  // missing property
        }
        *value = constant;
    } else if (value_equiv(kind, k_actor_create)) {
        // { "kind":"actor_create", "state":<dictionary>, "behavior":<behavior> }
        LOG_DEBUG("actor_eval: create expression", (WORD)kind);
        DATA_PTR state;
        if (!property_eval(expression, s_state, &state)) return false;  // evaluation failed!
        DATA_PTR behavior;
        if (!property_eval(expression, s_behavior, &behavior)) return false;  // evaluation failed!
        DATA_PTR actor;
        if (!actor_create(state, behavior, &actor)) {
            LOG_WARN("actor_eval: create failed!", (WORD)expression);
            return false;  // create failed!
        }
        *value = actor;
    } else if (value_equiv(kind, k_actor_behavior)) {
        // { "kind":"actor_behavior", "name":<string>, "script":[<action>, ...] }
        LOG_DEBUG("actor_eval: behavior expression", (WORD)kind);
        *value = expression;  // self-evaluating expression
    } else {
        LOG_WARN("actor_eval: unknown 'kind' of expression", (WORD)kind);
        // FIXME: probably want to return `false` here and fail the script execution, but we just ignore it...
        *value = v_null;
    }
    LOG_TRACE("actor_eval: value", (WORD)(*value));
    prints("  -> ");
    if (!value_print(*value, 0)) return false;  // print failed!
    return true;  // success!
}
#endif

#if USE_PARSE_CURSOR
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
        LOG_DEBUG("actor_exec: print action", (WORD)kind);
        prop_parse.start = 0;  // search from beginning of properties
        if (!object_get_property(&prop_parse, s_value)) {
            LOG_DEBUG("actor_exec: missing 'value' property @", (WORD)s_value);
            return false;  // missing property
        }
        DATA_PTR value;
        if (!actor_eval(&prop_parse, &value)) return false;  // evaluation failed!
        if (!value_print(value, 0)) return false;  // print failed
    } else if (value_equiv(kind, k_actor_ignore)) {
        LOG_DEBUG("actor_exec: ignore action", (WORD)kind);
    } else if (value_equiv(kind, k_actor_fail)) {
        LOG_DEBUG("actor_exec: fail action", (WORD)kind);
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
#else
BYTE actor_exec(DATA_PTR command) {  // execute actor commands (action -> effects)
    LOG_TRACE("actor_exec: command", (WORD)command);
    if (!value_print(command, 1)) return false;  // print failed!
    DATA_PTR kind;
    if (!object_get(command, s_kind, &kind)) {
        LOG_WARN("actor_exec: missing 'kind' property", (WORD)command);
        return false;  // missing property
    }
    if (value_equiv(kind, k_log_print)) {
        // { "kind":"log_print", "level":<number>, "value":<expression> }  // --DEPRECATED--
        LOG_DEBUG("actor_exec: print action", (WORD)kind);
        DATA_PTR value;
        if (!property_eval(command, s_value, &value)) return false;  // evaluation failed!
        if (!value_print(value, 0)) return false;  // print failed
    } else if (value_equiv(kind, k_actor_send)) {
        // { "kind":"actor_send", "message":<dictionary>, "actor":<address> }
        LOG_DEBUG("actor_exec: send action", (WORD)kind);
        DATA_PTR actor;
        if (!property_eval(command, s_actor, &actor)) return false;  // evaluation failed!
/*
        if (value_equiv(actor, v_null)) {  // shortcut -- if actor is null, don't even evaluate message...
            LOG_INFO("actor_exec: ignoring message to null.", null);
            return true;  // success!
        }
*/
        DATA_PTR message;
        if (!property_eval(command, s_message, &message)) return false;  // evaluation failed!
        if (!actor_send(actor, message)) {
            LOG_WARN("actor_exec: send failed!", (WORD)command);
            return false;  // send failed!
        }
    } else if (value_equiv(kind, k_actor_ignore)) {
        // { "kind":"actor_ignore" }
        LOG_DEBUG("actor_exec: ignore action", (WORD)kind);
    } else if (value_equiv(kind, k_actor_fail)) {
        // { "kind":"actor_fail", "error":<expression> }
        LOG_DEBUG("actor_exec: fail action", (WORD)kind);
        DATA_PTR error;
        if (!property_eval(command, s_error, &error)) {
            LOG_INFO("actor_exec: DOUBLE-FAULT!", (WORD)command);
            if (!value_print(command, 0)) return false;  // print failed
            return false;  // evaluation failed!
        }
        // FIXME: probably want this to be a WARN, and not always print the error value...
        LOG_INFO("actor_exec: FAIL!", (WORD)error);
        if (!value_print(error, 0)) return false;  // print failed
        return false;  // force failure!
    } else {
        LOG_WARN("actor_exec: unknown 'kind' of command", (WORD)kind);
        // FIXME: probably want to return `false` here and fail the script execution, but we just ignore it...
    }
    return true;  // success!
}
#endif

/*
 * WARNING! All the `run_...` procedures return 0 on success, and 1 on failure. (not true/false)
 */

#if USE_PARSE_CURSOR
int run_actor_script(parse_t * parse) {
    LOG_TRACE("run_actor_script @", (WORD)parse);
    //if (!parse_print(parse, 1)) return 1;  // print failed!
    //newline();
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
#else
int run_actor_script(DATA_PTR script) {
    LOG_TRACE("run_actor_script @", (WORD)script);
    //if (!value_print(script, 1)) return 1;  // print failed!
    WORD length;
    if (!array_length(script, &length)) {
        LOG_WARN("run_actor_script: script array required!", (WORD)script);
        return 1;  // top-level array required!
    }
    LOG_INFO("run_actor_script: script array length", length);
    WORD i;
    for (i = 0; i < length; ++i) {
        DATA_PTR command;
        if (!array_get(script, i, &command)
        ||  !object_has(command, s_kind)) {
            LOG_WARN("run_actor_script: command object required!", (WORD)command);
            return 1;  // command object required!
        }
        if (!actor_exec(command)) {
            LOG_WARN("run_actor_script: failed executing command!", (WORD)command);
            return 1;  // failed executing command!
        }
    }
    LOG_INFO("run_actor_script: completed successfully", i);
    return 0;  // success!
}
#endif

#if USE_PARSE_CURSOR
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
    return 1;  // failed!
}
#else
int run_actor_config(DATA_PTR item) {
    LOG_TRACE("run_actor_config @", (WORD)item);
    //if (!value_print(item, 1)) return 1;  // print failed!
    DATA_PTR value;
    WORD actors = 0;
    if (object_get(item, s_actors, &value)
    &&  value_integer(value, &actors)) {
        LOG_INFO("run_actor_config: actors =", actors);
    }
    WORD events = 0;
    if (object_get(item, s_events, &value)
    &&  value_integer(value, &events)) {
        LOG_INFO("run_actor_config: events =", events);
    }
    DATA_PTR script;
    if (object_get(item, s_script, &script)) {
        LOG_INFO("run_actor_config: script =", (WORD)script);
        return run_actor_script(script);
    }
    return 1;  // failed!
}
#endif

#if USE_PARSE_CURSOR
static BYTE object_has_kind(parse_t * parse, DATA_PTR kind) {
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
#endif

int run_program(DATA_PTR program) {
    LOG_INFO("bootstrap", (WORD)program);
    memo_clear();
#if USE_PARSE_CURSOR
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
#else
    WORD length;
    if (!array_length(program, &length)) {
        LOG_WARN("run_program: top-level array required!", (WORD)program);
        return 1;  // top-level array required!
    }
    LOG_INFO("run_program: top-level array length", length);
    for (WORD i = 0; i < length; ++i) {
        DATA_PTR item;
        DATA_PTR kind;
        if (!array_get(program, i, &item)
        ||  !object_get(item, s_kind, &kind)
        ||  !value_equiv(kind, k_actor_sponsor)) {
            LOG_WARN("run_program: sponsor object required!", (WORD)item);
            return 1;  // sponsor object required!
        }
        if (run_actor_config(item) != 0) {
            LOG_WARN("run_program: failed to start actor configuration!", (WORD)item);
            return 1;  // failed to start!
        }
    }
#endif
    return 0;  // success
}
