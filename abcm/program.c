/*
 * program.c -- bootstrap program
 */
#include <assert.h>

#include "program.h"
#include "bose.h"
#include "sponsor.h"
#include "array.h"
#include "object.h"
#include "equiv.h"
#include "print.h"
#include "abcm.h"

#define LOG_ALL // enable all logging
#include "log.h"


BYTE bootstrap[] = {
    0x06, 0x10, 0x82, 0x0d, 0x02, 0x81, 0x07, 0x10, 0x82, 0x07, 0x02, 0x84, 0x0a, 0x84, 0x6b, 0x69,  // ··············ki
    0x6e, 0x64, 0x0a, 0x8d, 0x61, 0x63, 0x74, 0x6f, 0x72, 0x5f, 0x73, 0x70, 0x6f, 0x6e, 0x73, 0x6f,  // nd··actor_sponso
    0x72, 0x0a, 0x86, 0x61, 0x63, 0x74, 0x6f, 0x72, 0x73, 0x81, 0x0a, 0x86, 0x65, 0x76, 0x65, 0x6e,  // r··actors···even
    0x74, 0x73, 0x81, 0x0a, 0x86, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x06, 0x10, 0x82, 0xd2, 0x01,  // ts···script···Ò·
    0x83, 0x07, 0x10, 0x82, 0x03, 0x01, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8c, 0x61,  // ·········kind··a
    0x63, 0x74, 0x6f, 0x72, 0x5f, 0x61, 0x73, 0x73, 0x69, 0x67, 0x6e, 0x0a, 0x84, 0x6e, 0x61, 0x6d,  // ctor_assign··nam
    0x65, 0x0a, 0x83, 0x77, 0x68, 0x6f, 0x0a, 0x85, 0x76, 0x61, 0x6c, 0x75, 0x65, 0x07, 0x10, 0x82,  // e··who··value···
    0xd7, 0x00, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8c, 0x61, 0x63, 0x74, 0x6f, 0x72,  // ×····kind··actor
    0x5f, 0x63, 0x72, 0x65, 0x61, 0x74, 0x65, 0x0a, 0x85, 0x73, 0x74, 0x61, 0x74, 0x65, 0x07, 0xab,  // _create··state·«
    0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8c, 0x65, 0x78, 0x70, 0x72, 0x5f, 0x6c, 0x69,  // ···kind··expr_li
    0x74, 0x65, 0x72, 0x61, 0x6c, 0x0a, 0x84, 0x74, 0x79, 0x70, 0x65, 0x0a, 0x86, 0x4f, 0x62, 0x6a,  // teral··type··Obj
    0x65, 0x63, 0x74, 0x0a, 0x85, 0x63, 0x6f, 0x6e, 0x73, 0x74, 0x03, 0x0a, 0x88, 0x62, 0x65, 0x68,  // ect··const···beh
    0x61, 0x76, 0x69, 0x6f, 0x72, 0x07, 0x10, 0x82, 0x7f, 0x00, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e,  // avior········kin
    0x64, 0x0a, 0x8e, 0x61, 0x63, 0x74, 0x6f, 0x72, 0x5f, 0x62, 0x65, 0x68, 0x61, 0x76, 0x69, 0x6f,  // d··actor_behavio
    0x72, 0x0a, 0x84, 0x6e, 0x61, 0x6d, 0x65, 0x0f, 0x0a, 0x86, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74,  // r··name···script
    0x06, 0xd7, 0x81, 0x07, 0xd4, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x89, 0x6c, 0x6f,  // ·×··Ô···kind··lo
    0x67, 0x5f, 0x70, 0x72, 0x69, 0x6e, 0x74, 0x0a, 0x85, 0x6c, 0x65, 0x76, 0x65, 0x6c, 0x81, 0x0a,  // g_print··level··
    0x85, 0x76, 0x61, 0x6c, 0x75, 0x65, 0x07, 0xb1, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a,  // ·value·±···kind·
    0x8c, 0x65, 0x78, 0x70, 0x72, 0x5f, 0x6c, 0x69, 0x74, 0x65, 0x72, 0x61, 0x6c, 0x0a, 0x84, 0x74,  // ·expr_literal··t
    0x79, 0x70, 0x65, 0x0a, 0x86, 0x53, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x0a, 0x85, 0x63, 0x6f, 0x6e,  // ype··String··con
    0x73, 0x74, 0x0a, 0x85, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x07, 0xf1, 0x83, 0x0a, 0x84, 0x6b, 0x69,  // st··World·ñ···ki
    0x6e, 0x64, 0x0a, 0x8a, 0x61, 0x63, 0x74, 0x6f, 0x72, 0x5f, 0x73, 0x65, 0x6e, 0x64, 0x0a, 0x87,  // nd··actor_send··
    0x6d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x07, 0xab, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64,  // message·«···kind
    0x0a, 0x8c, 0x65, 0x78, 0x70, 0x72, 0x5f, 0x6c, 0x69, 0x74, 0x65, 0x72, 0x61, 0x6c, 0x0a, 0x84,  // ··expr_literal··
    0x74, 0x79, 0x70, 0x65, 0x0a, 0x86, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 0x0a, 0x85, 0x63, 0x6f,  // type··Object··co
    0x6e, 0x73, 0x74, 0x03, 0x0a, 0x85, 0x61, 0x63, 0x74, 0x6f, 0x72, 0x07, 0x9f, 0x82, 0x0a, 0x84,  // nst···actor·····
    0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8b, 0x61, 0x63, 0x74, 0x6f, 0x72, 0x5f, 0x73, 0x74, 0x61, 0x74,  // kind··actor_stat
    0x65, 0x0a, 0x84, 0x6e, 0x61, 0x6d, 0x65, 0x0a, 0x83, 0x77, 0x68, 0x6f, 0x07, 0xd4, 0x83, 0x0a,  // e··name··who·Ô··
    0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x89, 0x6c, 0x6f, 0x67, 0x5f, 0x70, 0x72, 0x69, 0x6e, 0x74,  // ·kind··log_print
    0x0a, 0x85, 0x6c, 0x65, 0x76, 0x65, 0x6c, 0x81, 0x0a, 0x85, 0x76, 0x61, 0x6c, 0x75, 0x65, 0x07,  // ··level···value·
    0xb1, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8c, 0x65, 0x78, 0x70, 0x72, 0x5f, 0x6c,  // ±···kind··expr_l
    0x69, 0x74, 0x65, 0x72, 0x61, 0x6c, 0x0a, 0x84, 0x74, 0x79, 0x70, 0x65, 0x0a, 0x86, 0x53, 0x74,  // iteral··type··St
    0x72, 0x69, 0x6e, 0x67, 0x0a, 0x85, 0x63, 0x6f, 0x6e, 0x73, 0x74, 0x0a, 0x85, 0x48, 0x65, 0x6c,  // ring··const··Hel
    0x6c, 0x6f                                                                                       // lo
};

// evaluate expression found in named object property
static BYTE property_eval(sponsor_t * sponsor, actor_t * actor, DATA_PTR object, DATA_PTR name, DATA_PTR * value) {
    LOG_TRACE("property_eval: object =", (WORD)object);
    DATA_PTR expression;
    if (!object_get(object, name, &expression)) {
        LOG_DEBUG("property_eval: missing property", (WORD)name);
        return false;  // missing property
    }
    if (!actor_eval(sponsor, actor, expression, value)) {
        LOG_WARN("property_eval: bad expression!", (WORD)expression);
        return false;  // evaluation failed!
    }
    return true;  // success!
}

// evaluate actor expression (expression -> value)
BYTE actor_eval(sponsor_t * sponsor, actor_t * actor, DATA_PTR expression, DATA_PTR * value) {
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
    } else if (value_equiv(kind, k_actor_state)) {
        // { "kind":"actor_state", "name":<string> }
        LOG_DEBUG("actor_eval: state expression", (WORD)kind);
        DATA_PTR name;
        if (!object_get(expression, s_name, &name)) {
            LOG_DEBUG("actor_eval: missing 'name' property", (WORD)expression);
            return false;  // missing property
        }
        // FIXME: make sure `name` is a String...
        *value = v_null;  // default value is `null`
        if (!object_get(actor->state, name, value)) {
            LOG_WARN("actor_eval: undefined variable!", (WORD)name);
            // FIXME: we may want to "throw" an exception here...
        }
    } else if (value_equiv(kind, k_actor_create)) {
        // { "kind":"actor_create", "state":<dictionary>, "behavior":<behavior> }
        LOG_DEBUG("actor_eval: create expression", (WORD)kind);
        DATA_PTR state;
        if (!property_eval(sponsor, actor, expression, s_state, &state)) return false;  // evaluation failed!
        DATA_PTR behavior;
        if (!property_eval(sponsor, actor, expression, s_behavior, &behavior)) return false;  // evaluation failed!
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
BYTE actor_exec(sponsor_t * sponsor, actor_t * actor, DATA_PTR command) {
    LOG_TRACE("actor_exec: command", (WORD)command);
    if (!value_print(command, 1)) return false;  // print failed!
    DATA_PTR kind;
    if (!object_get(command, s_kind, &kind)) {
        LOG_WARN("actor_exec: missing 'kind' property", (WORD)command);
        return false;  // missing property
    }
    if (value_equiv(kind, k_actor_assign)) {
        // { "kind":"actor_assign", "name":<string>, "value":<expression> }
        LOG_DEBUG("actor_exec: assign action", (WORD)kind);
        DATA_PTR name;
        if (!object_get(command, s_name, &name)) {
            LOG_DEBUG("actor_exec: missing 'name' property", (WORD)command);
            return false;  // missing property
        }
        // FIXME: make sure `name` is a String...
        DATA_PTR value;
        if (!property_eval(sponsor, actor, command, s_value, &value)) return false;  // evaluation failed!
        //if (!value_print(value, 0)) return false;  // print failed
        DATA_PTR state;
        if (!object_add(sponsor, actor->state, name, value, &state)) return false;  // allocation failure!
        if (!RELEASE(&actor->state)) return false;  // reclamation failure!
        actor->state = TRACK(state);
        LOG_TRACE("actor_exec: state' =", (WORD)state);
        if (!value_print(state, 1)) return false;  // print failed
    } else if (value_equiv(kind, k_actor_send)) {
        // { "kind":"actor_send", "message":<dictionary>, "actor":<address> }
        LOG_DEBUG("actor_exec: send action", (WORD)kind);
        DATA_PTR address;
        if (!property_eval(sponsor, actor, command, s_actor, &address)) return false;  // evaluation failed!
        if (value_equiv(address, v_null)) {  // shortcut -- if address is null, don't even evaluate message...
            LOG_INFO("actor_exec: ignoring message to null.", null);
            return true;  // success!
        }
        // FIXME: make sure `actor` is a Capability...
        DATA_PTR message;
        if (!property_eval(sponsor, actor, command, s_message, &message)) return false;  // evaluation failed!
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
        if (!property_eval(sponsor, actor, command, s_error, &error)) {
            LOG_INFO("actor_exec: DOUBLE-FAULT!", (WORD)command);
            if (!value_print(command, 0)) return false;  // print failed
            return false;  // evaluation failed!
        }
        // FIXME: probably want this to be a WARN, and not always print the error value...
        LOG_INFO("actor_exec: FAIL!", (WORD)error);
        sponsor_fail(sponsor, error);
        return false;  // force failure!
    } else if (value_equiv(kind, k_log_print)) {
        // { "kind":"log_print", "level":<number>, "value":<expression> }  // --DEPRECATED--
        LOG_DEBUG("actor_exec: print action", (WORD)kind);
        DATA_PTR value;
        if (!property_eval(sponsor, actor, command, s_value, &value)) return false;  // evaluation failed!
        if (!value_print(value, 0)) return false;  // print failed
    } else {
        LOG_WARN("actor_exec: unknown 'kind' of command", (WORD)kind);
        // FIXME: probably want to return `false` here and fail the script execution, but we just ignore it...
    }
    return true;  // success!
}

/*
 * WARNING! All the `run_...` procedures return 0 on success, and 1 on failure. (not true/false)
 */

int run_actor_script(sponsor_t * sponsor, actor_t * actor) {
    LOG_TRACE("run_actor_script: sponsor =", (WORD)sponsor);
    LOG_TRACE("run_actor_script: actor =", (WORD)actor);
    DATA_PTR script;
    if (!object_get(actor->behavior, s_script, &script)) {
        LOG_WARN("run_actor_script: script required!", (WORD)actor->behavior);
        return false;  // script required!
    }
    LOG_DEBUG("run_actor_script: script =", (WORD)script);
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
        if (!actor_exec(sponsor, actor, command)) {
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
    if (!object_get(item, s_script, &script)) {
        LOG_WARN("run_actor_config: script required!", (WORD)item);
        return 1;  // script required!
    }
    LOG_INFO("run_actor_config: script =", (WORD)script);
    sponsor_t * sponsor = new_bounded_sponsor(actors, events, heap_pool);
    assert(sponsor);
    LOG_DEBUG("run_actor_config: sponsor =", (WORD)sponsor);
    BYTE behavior_template[] = { object_n, n_30, n_2,
        utf8, n_4, 'k', 'i', 'n', 'd', utf8, n_14, 'a', 'c', 't', 'o', 'r', '_', 'b', 'e', 'h', 'a', 'v', 'i', 'o', 'r',
        utf8, n_4, 'n', 'a', 'm', 'e', string_0
    };
    DATA_PTR behavior;
    if (!object_add(sponsor, behavior_template, s_script, script, &behavior)) return false;  // allocation failure!
    behavior = TRACK(behavior);
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "run_actor_config: behavior =", (WORD)behavior);
    //if (!value_print(behavior, 1)) return 1;  // print failed!
    actor_t bootstrap_actor = {
        .capability = { null },  // can't send messages to bootstrap
        .behavior = behavior
    };
    if (!COPY(&bootstrap_actor.state, o_)) return false;  // allocation failure!
    if (run_actor_script(sponsor, &bootstrap_actor) != 0) {
        LOG_WARN("run_actor_config: boot-script execution failed!", (WORD)&bootstrap_actor);
        return 1;  // boot-script execution failed!
    }
    while (sponsor_dispatch(sponsor))  // dispatch message-events
        ;
    return 0;  // success!
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
