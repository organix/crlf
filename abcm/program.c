/*
 * program.c -- bootstrap program
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
#include "abcm.h"

#define LOG_ALL // enable all logging
#include "log.h"

/*
 * include actor-byte-code bootstrap program...
 */
//#include "hello_world.abc"
//#include "basic_scope.abc"
//#include "fail_example.abc"
//#include "stream_reader.abc"
#include "lambda_calculus.abc"


BYTE validate_value(DATA_PTR value) {
    LOG_INFO("validate_value @", (WORD)value);
    /*
     * FIXME: IMPLEMENT SINGLE-PASS PARSE/VALIDATE TO ENSURE SANITY AND CAPTURE MEMOIZED STRINGS...
     */
    if (!value_print(value, 1)) {
        LOG_WARN("validate_value: FAILED VALIDATION!", false);
        return false;  // print failed!
    }
    return true;  // success!
}

/*
 * WARNING! All the `run_...` procedures return 0 on success, and 1 on failure. (not true/false)
 */

int run_actor_config(DATA_PTR spec) {
    LOG_TRACE("run_actor_config @", (WORD)spec);
    if (!validate_value(spec)) return 1;  // validate failed!
    DATA_PTR value;
    WORD actors = 0;
    if (object_get(spec, s_actors, &value) && value_integer(value, &actors)) {
        LOG_INFO("run_actor_config: actors =", actors);
    }
    WORD events = 0;
    if (object_get(spec, s_events, &value) && value_integer(value, &events)) {
        LOG_INFO("run_actor_config: events =", events);
    }
    DATA_PTR script;
    if (!object_get(spec, s_script, &script)) {
        LOG_WARN("run_actor_config: script required!", (WORD)spec);
        return 1;  // script required!
    }
    LOG_INFO("run_actor_config: script =", (WORD)script);
    IF_LEVEL(LOG_LEVEL_TRACE+1, value_print(script, 1));
    // +1 to account for initial actor and event
    sponsor_t * config_sponsor = new_sponsor(heap_pool, actors + 1, events + 1);
    assert(config_sponsor);
    sponsor_t * boot_sponsor = sponsor;  // save current global sponsor
    sponsor = config_sponsor;  // SET GLOBAL SPONSOR!
    LOG_DEBUG("run_actor_config: sponsor =", (WORD)sponsor);
    config_t * config = SPONSOR_CONFIG(sponsor);
    LOG_DEBUG("run_actor_config: config =", (WORD)config);

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
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "run_actor_config: behavior =", (WORD)behavior);
    IF_LEVEL(LOG_LEVEL_TRACE+1, value_print(behavior, 1));
    scope_t * scope = NULL;  // no parent scope
    DATA_PTR state = o_;  // empty initial state
    DATA_PTR address;
    if (!config_create(config, scope, state, behavior, &address)) {
        LOG_WARN("run_actor_config: failed to create initial actor!", (WORD)config);
        return 1;  // failure!
    }
    if (!RELEASE(&behavior)) return 1;  // failure!
    DATA_PTR message = o_;  // empty initial message
    if (!config_send(config, address, message)) {
        LOG_WARN("run_actor_config: failed sending to initial actor!", (WORD)config);
        return 1;  // failure!
    }

    // dispatch message-events
    while (config_dispatch(config))
        ;

    // shut down and clean up...
    if (!sponsor_shutdown(&sponsor, actors + 1, events + 1)) return 1;  // failure!
    assert(sponsor == 0);
    sponsor = boot_sponsor;  // restore previous global sponsor
    LOG_DEBUG("run_actor_config: boot_sponsor =", (WORD)sponsor);
    return 0;  // success!
}

int run_program(DATA_PTR program) {
    LOG_INFO("bootstrap", (WORD)program);
    if (!memo_reset()) return 1;  // memo reset failed!
    WORD count;
    if (!array_count(program, &count)) {
        LOG_WARN("run_program: top-level array required!", (WORD)program);
        return 1;  // top-level array required!
    }
    LOG_INFO("run_program: top-level array count", count);
    for (WORD i = 0; i < count; ++i) {
        DATA_PTR item;
        DATA_PTR kind;
        if (!array_get(program, i, &item)
        ||  !object_get(item, s_kind, &kind)
        ||  !value_equiv(kind, k_actor_sponsor)) {
            LOG_WARN("run_program: sponsor object required!", (WORD)item);
            return 1;  // sponsor object required!
        }
        // FIXME: we should create each sponsor, run its boot-script, then round-robin among the sponsors for dispatching
        if (run_actor_config(item) != 0) {
            LOG_WARN("run_program: actor configuration failed!", (WORD)item);
            return 1;  // configuration failed!
        }
    }
    if (!memo_reset()) return 1;  // memo reset failed!
    return 0;  // success
}
