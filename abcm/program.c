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
BYTE bootstrap[] = {
//#include "hello_world.abc"
//#include "basic_scope.abc"
//#include "fail_example.abc"
#include "two_sponsor.abc"
//#include "stream_reader.abc"
//#include "lambda_calculus.abc"
};

#if 1
BYTE boot2nd[] = {
#include "fail_example.abc"
};
#endif

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

static int run_actor_config(config_t * config, DATA_PTR script) {
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
    return 0;  // success!
}

int run_program(DATA_PTR program) {
    LOG_INFO("run_program: program =", (WORD)program);
    pool_t * pool = SPONSOR_POOL(sponsor);
    if (!RESERVE((DATA_PTR *)&sponsor->memo, sizeof(memo_t))) {
        LOG_WARN("run_program: failed allocation of memo structure!", (WORD)sizeof(memo_t));
        return 1;  // failed allocation of memo structure!
    }
    memo_t * memo = SPONSOR_MEMO(sponsor);
    if (!memo_init(memo)) return 1;  // memo init failed!
    if (!validate_value(program)) return 1;  // validate failed!
    WORD count;
    if (!array_count(program, &count)) {
        LOG_WARN("run_program: top-level array required!", (WORD)program);
        return 1;  // top-level array required!
    }
    LOG_INFO("run_program: top-level array count", count);
    sponsor_t * sponsor_group;  // collection of sponsors
    if (!RESERVE((DATA_PTR *)&sponsor_group, sizeof(sponsor_t) * count)) {
        LOG_WARN("run_program: failed allocation of sponsor group!", (WORD)count);
        return 1;  // failed allocation of sponsor group!
    }
    sponsor_t * boot_sponsor = sponsor;  // save current global sponsor

    // create each sponsor from specification
    for (WORD i = 0; i < count; ++i) {
        LOG_DEBUG("run_program: creating sponsor #", i);
        DATA_PTR spec;
        DATA_PTR kind;
        if (!array_get(program, i, &spec)
        ||  !object_get(spec, s_kind, &kind)
        ||  !value_equiv(kind, k_actor_sponsor)) {
            LOG_WARN("run_program: sponsor object required!", (WORD)spec);
            return 1;  // sponsor object required!
        }
        DATA_PTR value;
        WORD actors = 0;
        if (object_get(spec, s_actors, &value) && value_integer(value, &actors)) {
            LOG_INFO("run_program: actors =", actors);
        }
        WORD events = 0;
        if (object_get(spec, s_events, &value) && value_integer(value, &events)) {
            LOG_INFO("run_program: events =", events);
        }
        DATA_PTR script;
        if (!object_get(spec, s_script, &script)) {
            LOG_WARN("run_program: script required!", (WORD)spec);
            return 1;  // script required!
        }
        LOG_INFO("run_program: script =", (WORD)script);
        IF_LEVEL(LOG_LEVEL_TRACE+1, value_print(script, 1));

        sponsor = &sponsor_group[i];  // set global sponsor
        // +1 to account for initial actor and event
        if (!init_sponsor(sponsor, pool, memo, actors + 1, events + 1)) return false;  // allocation failure!
        LOG_DEBUG("run_program: sponsor =", (WORD)sponsor);
        config_t * config = SPONSOR_CONFIG(sponsor);
        LOG_DEBUG("run_program: config =", (WORD)config);
        if (run_actor_config(config, script) != 0) {
            LOG_WARN("run_program: actor configuration failed!", (WORD)spec);
            return 1;  // configuration failed!
        }
        sponsor = boot_sponsor;  // restore previous global sponsor
    }

    // round-robin scheduling to each sponsor for message dispatching
    BYTE working = true;
    while (working) {
        working = false;  // if this remains `false`, there's no more work.
        for (WORD i = 0; i < count; ++i) {
            sponsor = &sponsor_group[i];  // set global sponsor
            config_t * config = SPONSOR_CONFIG(sponsor);
            if (config) {  // config is cleared by shutdown...
                LOG_INFO("run_program: dispatching sponsor", (WORD)sponsor);
                // dispatch message-events
                if (config_dispatch(config)) {
                    working = true;  // still working...
                } else {
                    // shut down and clean up...
                    if (!sponsor_shutdown(sponsor)) return 1;  // failure!
                }
            }
        }
    }

    // clean up and prepare to exit
    sponsor = boot_sponsor;  // restore previous global sponsor
    LOG_DEBUG("run_program: boot_sponsor =", (WORD)sponsor);
    if (!RELEASE((DATA_PTR *)&sponsor_group)) {
        LOG_WARN("run_program: failed to release sponsor group!", (WORD)sponsor_group);
        return 1;  // failed to release sponsor group!
    }
    LOG_INFO("run_program: final memo index =", MEMO_INDEX(memo));
    if (!RELEASE((DATA_PTR *)&sponsor->memo)) {
        LOG_WARN("run_program: failed to release memo structure!", (WORD)memo);
        return 1;  // failed to release memo structure!
    }
    return 0;  // success
}
