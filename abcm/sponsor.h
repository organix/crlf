/*
 * sponsor.h -- actor resource provider/manager
 */
#ifndef _SPONSOR_H_
#define _SPONSOR_H_

#include "bose.h"
#include "pool.h"
#include "event.h"

#define ANNOTATE_DELIVERY 1 /* display each actor-message delivery event dispatched */
#define ANNOTATE_BEHAVIOR 1 /* display the behavior of each actor on delivery */

#define REF_COUNTED_BOOT_SPONSOR 1 /* use ref_pool for boot_sponsor allocations (incl. memo) */
#define USE_HEAP_POOL_FOR_CONFIG 1 /* force use of heap_pool in config, otherwise inherit from sponsor */
#define EVENT_TEMP_POOL_SIZE (1 << 12)  /* if this is 0, temp_pool is not used. */

/*
 * Configuration
 */

//typedef struct config_struct config_t;
typedef struct config_struct {
    pool_t *    pool;           // memory allocation pool
    actor_t *   actor;          // actor roster
    event_t *   event;          // event queue
    WORD        actors;         // actor creation index
    WORD        events;         // message-send event index
    WORD        current;        // current-event index
} config_t;

#define CONFIG_POOL(config) ((config)->pool)  // (config_t *) -> (pool_t *)
#define CONFIG_ACTORS(config) ((config)->actors)  // (config_t *) -> (WORD)
#define CONFIG_EVENTS(config) ((config)->events)  // (config_t *) -> (WORD)
#define CONFIG_CURRENT(config) ((config)->current)  // (config_t *) -> (WORD)
#define CONFIG_EVENT(config) (&(config)->event[CONFIG_CURRENT(config)])  // (config_t *) -> (event_t *)

BYTE config_dispatch(config_t * config);
BYTE config_commit(config_t * config, effect_t * effect);
BYTE config_rollback(config_t * config, effect_t * effect);
/**
Instead of storing new actors/events in Effects,
we create them directly in the Configuration,
because most-likely they will persist.
If we have to revert (on failure), we clean up the extra actors/events.
**/
BYTE config_create(config_t * config, scope_t * parent, DATA_PTR state, DATA_PTR behavior, DATA_PTR * address);
BYTE config_send(config_t * config, DATA_PTR address, DATA_PTR message);

config_t * new_config(pool_t * pool, WORD actors, WORD events);
BYTE config_release(config_t ** config, WORD actors, WORD events);

/*
 * Sponsor
 */

typedef struct sponsor_struct sponsor_t;
extern sponsor_t * sponsor;  // WE DECLARE A GLOBAL SPONSOR TO AVOID THREADING IT THROUGH ALL OTHER CALLS...

typedef struct sponsor_struct {
    pool_t *    pool;           // memory allocation pool
    memo_t *    memo;           // BOSE memoization structure
    WORD        actors;         // actor creation limit
    WORD        events;         // message-send event limit
    config_t *  config;         // configuration state
    sponsor_t * next;           // sponsor chain link
} sponsor_t;

#define SPONSOR_POOL(sponsor) ((sponsor)->pool)  // (sponsor_t *) -> (pool_t *)
#define SPONSOR_MEMO(sponsor) ((sponsor)->memo)  // (sponsor_t *) -> (memo_t *)
#define SPONSOR_CONFIG(sponsor) ((sponsor)->config)  // (sponsor_t *) -> (config_t *)
#define SPONSOR_EVENT(sponsor) CONFIG_EVENT(SPONSOR_CONFIG(sponsor))  // (sponsor_t *) -> (event_t *)
#define SPONSOR_NEXT(sponsor) ((sponsor)->next)  // (sponsor_t *) -> (sponsor_t *)

sponsor_t * new_sponsor(pool_t * pool, memo_t * memo, WORD actors, WORD events);
BYTE sponsor_shutdown(sponsor_t * sponsor);
BYTE sponsor_release(sponsor_t ** sponsor);

#endif // _SPONSOR_H_
