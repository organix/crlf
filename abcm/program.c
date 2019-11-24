/*
 * program.c -- bootstrap program(s)
 */
#include <stddef.h>  // for NULL, size_t, et. al.
#include <assert.h>

#include "program.h"
#include "bose.h"
#include "pool.h"
#include "sponsor.h"
#include "event.h"
#include "string.h"
#include "array.h"
#include "object.h"
#include "equiv.h"
#include "print.h"

#define LOG_ALL // enable all logging
#include "log.h"

BYTE s_kind[] = { utf8, n_4, 'k', 'i', 'n', 'd' };
BYTE s_actors[] = { utf8, n_6, 'a', 'c', 't', 'o', 'r', 's' };
BYTE s_events[] = { utf8, n_6, 'e', 'v', 'e', 'n', 't', 's' };
BYTE s_script[] = { utf8, n_6, 's', 'c', 'r', 'i', 'p', 't' };
BYTE s_message[] = { utf8, n_7, 'm', 'e', 's', 's', 'a', 'g', 'e' };
BYTE s_actor[] = { utf8, n_5, 'a', 'c', 't', 'o', 'r' };
BYTE s_state[] = { utf8, n_5, 's', 't', 'a', 't', 'e' };
BYTE s_behavior[] = { utf8, n_8, 'b', 'e', 'h', 'a', 'v', 'i', 'o', 'r' };
BYTE s_name[] = { utf8, n_4, 'n', 'a', 'm', 'e' };
BYTE s_value[] = { utf8, n_5, 'v', 'a', 'l', 'u', 'e' };
BYTE s_type[] = { utf8, n_4, 't', 'y', 'p', 'e' };
BYTE s_args[] = { utf8, n_4, 'a', 'r', 'g', 's' };
BYTE s_if[] = { utf8, n_2, 'i', 'f' };
BYTE s_do[] = { utf8, n_2, 'd', 'o' };
BYTE s_in[] = { utf8, n_2, 'i', 'n' };
BYTE s_with[] = { utf8, n_4, 'w', 'i', 't', 'h' };
BYTE s_index[] = { utf8, n_5, 'i', 'n', 'd', 'e', 'x' };
BYTE s_string[] = { utf8, n_6, 's', 't', 'r', 'i', 'n', 'g' };
BYTE s_array[] = { utf8, n_5, 'a', 'r', 'r', 'a', 'y' };
BYTE s_const[] = { utf8, n_5, 'c', 'o', 'n', 's', 't' };
BYTE s_level[] = { utf8, n_5, 'l', 'e', 'v', 'e', 'l' };
BYTE s_error[] = { utf8, n_5, 'e', 'r', 'r', 'o', 'r' };

/*
// Containers
    { "kind":"actor_sponsor", "actors":<number>, "events":<number>, "script":[<action>, ...] }
// Actions
    { "kind":"actor_send", "message":<dictionary>, "actor":<address> }
    { "kind":"actor_become", "behavior":<behavior> }
    { "kind":"actor_ignore" }
    { "kind":"actor_assign", "name":<string>, "value":<expression> }
    { "kind":"actor_fail", "error":<expression> }
    { "kind":"conditional", "args":[{ "if":<expression>, "do":[<action>, ...] }, ...] }
    { "kind":"log_print", "level":<number>, "value":<expression> }  // --DEPRECATED--
// Address Expressions
    { "kind":"actor_create", "state":<dictionary>, "behavior":<behavior> }
    { "kind":"actor_self" }
// Behavior Expressions
    { "kind":"actor_behavior", "name":<string>, "script":[<action>, ...] }
// Value Expressions
    { "kind":"actor_state", "name":<string> }
    { "kind":"dict_get", "name":<string>, "in":<dictionary> }
    { "kind":"list_get", "at":<number>, "from":<list> }
    { "kind":"expr_literal", "const":<value> }
    { "kind":"expr_operation", "name":<string>, "args":[<expression>, ...] }
// Number Expressions
    { "kind":"list_length", "of":<list> }
// Boolean Expressions
    { "kind":"actor_has_state", "name":<string> }
    { "kind":"dict_has", "name":<string>, "in":<dictionary> }
// List (Array) Expressions
    { "kind":"list_add", "value":<expression>, "at":<number>, "to":<list> }
    { "kind":"list_remove", "at":<number>, "from":<list> }
// Dictionary (Object) Expressions
    { "kind":"actor_message" }
    { "kind":"dict_bind", "name":<string>, "value":<expression>, "with":<dictionary> }
*/
BYTE k_actor_sponsor[] = { utf8, n_13, 'a', 'c', 't', 'o', 'r', '_', 's', 'p', 'o', 'n', 's', 'o', 'r' };
BYTE k_actor_send[] = { utf8, n_10, 'a', 'c', 't', 'o', 'r', '_', 's', 'e', 'n', 'd' };
BYTE k_actor_become[] = { utf8, n_12, 'a', 'c', 't', 'o', 'r', '_', 'b', 'e', 'c', 'o', 'm', 'e' };
BYTE k_actor_ignore[] = { utf8, n_12, 'a', 'c', 't', 'o', 'r', '_', 'i', 'g', 'n', 'o', 'r', 'e' };
BYTE k_actor_assign[] = { utf8, n_12, 'a', 'c', 't', 'o', 'r', '_', 'a', 's', 's', 'i', 'g', 'n' };
BYTE k_actor_fail[] = { utf8, n_10, 'a', 'c', 't', 'o', 'r', '_', 'f', 'a', 'i', 'l' };
BYTE k_conditional[] = { utf8, n_11, 'c', 'o', 'n', 'd', 'i', 't', 'i', 'o', 'n', 'a', 'l' };
BYTE k_actor_behavior[] = { utf8, n_14, 'a', 'c', 't', 'o', 'r', '_', 'b', 'e', 'h', 'a', 'v', 'i', 'o', 'r' };
BYTE k_actor_create[] = { utf8, n_12, 'a', 'c', 't', 'o', 'r', '_', 'c', 'r', 'e', 'a', 't', 'e' };
BYTE k_actor_message[] = { utf8, n_13, 'a', 'c', 't', 'o', 'r', '_', 'm', 'e', 's', 's', 'a', 'g', 'e' };
BYTE k_actor_self[] = { utf8, n_10, 'a', 'c', 't', 'o', 'r', '_', 's', 'e', 'l', 'f' };
BYTE k_actor_has_state[] = { utf8, n_15, 'a', 'c', 't', 'o', 'r', '_', 'h', 'a', 's', '_', 's', 't', 'a', 't', 'e' };
BYTE k_actor_state[] = { utf8, n_11, 'a', 'c', 't', 'o', 'r', '_', 's', 't', 'a', 't', 'e' };
BYTE k_dict_has[] = { utf8, n_8, 'd', 'i', 'c', 't', '_', 'h', 'a', 's' };
BYTE k_dict_get[] = { utf8, n_8, 'd', 'i', 'c', 't', '_', 'g', 'e', 't' };
BYTE k_dict_bind[] = { utf8, n_9, 'd', 'i', 'c', 't', '_', 'b', 'i', 'n', 'd' };
// { "kind":"string_length", "string":<string> }
// { "kind":"string_at", "index":<number>, "string":<string> }
// { "kind":"string_insert", "index":<number>, "value":<number>, "string":<string> }
BYTE k_string_length[] = { utf8, n_13, 's', 't', 'r', 'i', 'n', 'g', '_', 'l', 'e', 'n', 'g', 't', 'h' };
BYTE k_string_at[] = { utf8, n_9, 's', 't', 'r', 'i', 'n', 'g', '_', 'a', 't' };
BYTE k_string_insert[] = { utf8, n_13, 's', 't', 'r', 'i', 'n', 'g', '_', 'i', 'n', 's', 'e', 'r', 't' };
// { "kind":"array_length", "array":<array> }
// { "kind":"array_at", "index":<number>, "array":<array> }
// { "kind":"array_insert", "index":<number>, "value":<expression>, "array":<array> }
BYTE k_array_length[] = { utf8, n_12, 'a', 'r', 'r', 'a', 'y', '_', 'l', 'e', 'n', 'g', 't', 'h' };
BYTE k_array_at[] = { utf8, n_8, 'a', 'r', 'r', 'a', 'y', '_', 'a', 't' };
BYTE k_array_insert[] = { utf8, n_12, 'a', 'r', 'r', 'a', 'y', '_', 'i', 'n', 's', 'e', 'r', 't' };
BYTE k_expr_literal[] = { utf8, n_12, 'e', 'x', 'p', 'r', '_', 'l', 'i', 't', 'e', 'r', 'a', 'l' };
BYTE k_expr_operation[] = { utf8, n_14, 'e', 'x', 'p', 'r', '_', 'o', 'p', 'e', 'r', 'a', 't', 'i', 'o', 'n' };
BYTE k_log_print[] = { utf8, n_9, 'l', 'o', 'g', '_', 'p', 'r', 'i', 'n', 't' };


BYTE validate_value(DATA_PTR value) {
    LOG_INFO("validate_value @", (WORD)value);
    /*
     * FIXME: IMPLEMENT SINGLE-PASS PARSE/VALIDATE TO ENSURE SANITY AND CAPTURE MEMOIZED STRINGS...
     */
    if (!value_print(value, 1)) {
        LOG_WARN("validate_value: FAILED VALIDATION!", false);
        return false;  // print failed!
    }
    LOG_INFO("validate_value: final memo index", MEMO_INDEX(SPONSOR_MEMO(sponsor)));
    return true;  // success!
}

static BYTE config_exec(config_t * config, DATA_PTR script) {
    LOG_TRACE("config_exec: config =", (WORD)config);
    LOG_DEBUG("config_exec: script =", (WORD)script);
    IF_LEVEL(LOG_LEVEL_TRACE+1, value_print_limit(script, 1, 2));
    /*
     * --WARNING-- THIS CODE HAS INTIMATE KNOWLEDGE OF THE ACTOR AND EVENT STRUCTURES
     */
    BYTE behavior_template[] = { object_n, n_30, n_2,
        utf8, n_4, 'k', 'i', 'n', 'd', utf8, n_14, 'a', 'c', 't', 'o', 'r', '_', 'b', 'e', 'h', 'a', 'v', 'i', 'o', 'r',
        utf8, n_4, 'n', 'a', 'm', 'e', string_0
    };
    DATA_PTR behavior;
    if (!object_add(behavior_template, s_script, script, &behavior)) return false;  // allocation failure!
    behavior = TRACK(behavior);
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "config_exec: behavior =", (WORD)behavior);
    IF_LEVEL(LOG_LEVEL_TRACE+1, value_print_limit(behavior, 1, 2));
    scope_t * scope = NULL;  // no parent scope
    DATA_PTR state = o_;  // empty initial state
    DATA_PTR address;
    if (!config_create(config, scope, state, behavior, &address)) {
        LOG_WARN("config_exec: failed to create initial actor!", (WORD)config);
        return false;  // create failure!
    }
    if (!RELEASE(&behavior)) return false;  // reclamation failure!
    DATA_PTR message = o_;  // empty initial message
    if (!config_send(config, address, message)) {
        LOG_WARN("config_exec: failed sending to initial actor!", (WORD)config);
        return false;  // send failure!
    }
    LOG_INFO("config_exec: created initial actor/event.", (WORD)config);
    return true;  // success!
}

BYTE load_program(DATA_PTR program) {
    LOG_INFO("load_program: program =", (WORD)program);
    pool_t * pool = SPONSOR_POOL(sponsor);
    if (!RESERVE((DATA_PTR *)&sponsor->memo, sizeof(memo_t))) {
        LOG_WARN("load_program: failed allocation of memo structure!", (WORD)sizeof(memo_t));
        return false;  // failed allocation of memo structure!
    }
    memo_t * memo = SPONSOR_MEMO(sponsor);
    if (!memo_init(memo)) return false;  // memo init failed!
    LOG_WARN("load_program: using memo", (WORD)memo);
    if (!validate_value(program)) return false;  // validate failed!
    WORD count;
    if (!array_count(program, &count)) {
        LOG_WARN("load_program: top-level array required!", (WORD)program);
        return false;  // top-level array required!
    }
    LOG_INFO("load_program: top-level array count", count);
    // create each sponsor from specification
    for (WORD i = 0; i < count; ++i) {
        LOG_DEBUG("load_program: creating sponsor #", i);
        DATA_PTR spec;
        DATA_PTR kind;
        if (!array_get(program, i, &spec)
        ||  !object_get(spec, s_kind, &kind)
        ||  !value_equiv(kind, k_actor_sponsor)) {
            LOG_WARN("load_program: sponsor object required!", (WORD)spec);
            return false;  // sponsor object required!
        }
        DATA_PTR value;
        WORD actors = 0;
        if (object_get(spec, s_actors, &value) && value_integer(value, &actors)) {
            LOG_INFO("load_program: actors =", actors);
        }
        WORD events = 0;
        if (object_get(spec, s_events, &value) && value_integer(value, &events)) {
            LOG_INFO("load_program: events =", events);
        }
        DATA_PTR script;
        if (!object_get(spec, s_script, &script)) {
            LOG_WARN("load_program: script required!", (WORD)spec);
            return false;  // script required!
        }
        LOG_DEBUG("load_program: script =", (WORD)script);

        sponsor_t * boot_sponsor = sponsor;  // save current global sponsor
        // +1 to account for initial actor and event
        sponsor = new_sponsor(pool, memo, actors + 1, events + 1);
        if (!sponsor) return false;  // allocation failure!
        LOG_WARN("load_program: sponsor =", (WORD)sponsor);
        if (!config_exec(SPONSOR_CONFIG(sponsor), script)) {
            LOG_WARN("load_program: actor configuration failed!", (WORD)spec);
            return false;  // configuration failed!
        }
        // link new sponsor into dispatch chain (ring)
        sponsor_t * first = SPONSOR_NEXT(boot_sponsor);
        if (!first) {
            sponsor->next = sponsor;  // a ring of one...
            boot_sponsor->next = sponsor;
            LOG_DEBUG("load_program: ring first =", (WORD)sponsor);
        } else {
            sponsor->next = first->next;
            first->next = sponsor;
            LOG_DEBUG("load_program: ring next =", (WORD)sponsor);
        }
        sponsor = boot_sponsor;  // restore previous global sponsor
    }
#if REF_COUNTED_BOOT_SPONSOR
    if (!RELEASE((DATA_PTR *)&sponsor->memo)) {
        LOG_WARN("load_program: failed to reduce memo ref-count!", (WORD)&sponsor->memo);
        return false;  // failed to reduce memo ref-count!
    }
#endif
    LOG_DEBUG("load_program: dispatch ring start =", (WORD)SPONSOR_NEXT(sponsor));
    return true;  // success!
}

BYTE sponsor_dispatch_loop(sponsor_t * start) {
    assert(start);
    // round-robin scheduling to each sponsor for message dispatching
    sponsor = start;  // set global sponsor
    BYTE working = true;
    while (working) {
        working = false;  // if this remains `false`, there's no more work.
        do {
            config_t * config = SPONSOR_CONFIG(sponsor);
            if (config) {  // config is cleared by shutdown...
                LOG_DEBUG("sponsor_dispatch_loop: dispatching sponsor", (WORD)sponsor);
                // dispatch message-events
#if USE_HEAP_POOL_FOR_CONFIG
                pool_t * sponsor_pool = SPONSOR_POOL(sponsor);  // remember original sponsor pool
                sponsor->pool = CONFIG_POOL(config);  // switch to config pool for dispatching
#else
                assert(SPONSOR_POOL(sponsor) == CONFIG_POOL(config));
#endif
                BYTE ok = config_dispatch(config);
#if USE_HEAP_POOL_FOR_CONFIG
                sponsor->pool = sponsor_pool;  // restore original sponsor pool
#endif
                if (ok) {
                    working = true;
                } else {
                    // shut down and clean up...
                    if (!sponsor_shutdown(sponsor)) return false;  // shutdown failed!
                }
            }
            sponsor = SPONSOR_NEXT(sponsor);  // update global sponsor
        } while (sponsor != start);
    }
    LOG_INFO("sponsor_dispatch_loop: no more work.", (WORD)sponsor);
    return true;  // success!
}
