/*
 * program.c -- bootstrap program
 */
#include <assert.h>

#include "program.h"
#include "bose.h"
#include "array.h"
#include "object.h"
#include "equiv.h"
#include "print.h"
#include "abcm.h"

#define LOG_ALL // enable all logging
#include "log.h"


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

// evaluate expression found in named object property
static BYTE property_eval(sponsor_t * sponsor, DATA_PTR object, DATA_PTR name, DATA_PTR * value) {
    LOG_TRACE("property_eval: object =", (WORD)object);
    DATA_PTR expression;
    if (!object_get(object, name, &expression)) {
        LOG_DEBUG("property_eval: missing property", (WORD)name);
        return false;  // missing property
    }
    if (!actor_eval(sponsor, expression, value)) {
        LOG_WARN("property_eval: bad expression!", (WORD)expression);
        return false;  // evaluation failed!
    }
    return true;  // success!
}

// evaluate actor expression (expression -> value)
BYTE actor_eval(sponsor_t * sponsor, DATA_PTR expression, DATA_PTR * value) {
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
        if (!property_eval(sponsor, expression, s_state, &state)) return false;  // evaluation failed!
        DATA_PTR behavior;
        if (!property_eval(sponsor, expression, s_behavior, &behavior)) return false;  // evaluation failed!
        DATA_PTR address;
        if (!sponsor_create(sponsor, state, behavior, &address)) {
            LOG_WARN("actor_eval: create failed!", (WORD)expression);
            return false;  // create failed!
        }
        *value = address;
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

// execute actor command (action -> effects)
BYTE actor_exec(sponsor_t * sponsor, DATA_PTR command) {
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
        if (!property_eval(sponsor, command, s_value, &value)) return false;  // evaluation failed!
        if (!value_print(value, 0)) return false;  // print failed
    } else if (value_equiv(kind, k_actor_send)) {
        // { "kind":"actor_send", "message":<dictionary>, "actor":<address> }
        LOG_DEBUG("actor_exec: send action", (WORD)kind);
        DATA_PTR address;
        if (!property_eval(sponsor, command, s_actor, &address)) return false;  // evaluation failed!
/*
        if (value_equiv(address, v_null)) {  // shortcut -- if address is null, don't even evaluate message...
            LOG_INFO("actor_exec: ignoring message to null.", null);
            return true;  // success!
        }
*/
        DATA_PTR message;
        if (!property_eval(sponsor, command, s_message, &message)) return false;  // evaluation failed!
        if (!sponsor_send(sponsor, address, message)) {
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
        if (!property_eval(sponsor, command, s_error, &error)) {
            LOG_INFO("actor_exec: DOUBLE-FAULT!", (WORD)command);
            if (!value_print(command, 0)) return false;  // print failed
            return false;  // evaluation failed!
        }
        // FIXME: probably want this to be a WARN, and not always print the error value...
        LOG_INFO("actor_exec: FAIL!", (WORD)error);
        sponsor_fail(sponsor, error);
        return false;  // force failure!
    } else {
        LOG_WARN("actor_exec: unknown 'kind' of command", (WORD)kind);
        // FIXME: probably want to return `false` here and fail the script execution, but we just ignore it...
    }
    return true;  // success!
}

/*
 * WARNING! All the `run_...` procedures return 0 on success, and 1 on failure. (not true/false)
 */

int run_actor_script(sponsor_t * sponsor, DATA_PTR script) {
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
        if (!actor_exec(sponsor, command)) {
            LOG_WARN("run_actor_script: failed executing command!", (WORD)command);
            return 1;  // failed executing command!
        }
    }
    LOG_INFO("run_actor_script: completed successfully", i);
    return 0;  // success!
}

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
        sponsor_t * sponsor = new_bounded_sponsor(actors, events, heap_pool);
        assert(sponsor);
        LOG_DEBUG("run_actor_config: sponsor =", (WORD)sponsor);
        return run_actor_script(sponsor, script);
    }
    return 1;  // failed!
}

int run_program(DATA_PTR program) {
    LOG_INFO("bootstrap", (WORD)program);
    memo_clear();
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
    return 0;  // success
}
