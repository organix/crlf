/*
 * sponsor.h -- actor resource provider/manager
 */
#ifndef _SPONSOR_H_
#define _SPONSOR_H_

#include "bose.h"
#include "pool.h"
#include "event.h"

/*
 * Configuration
 */

//typedef struct config_struct config_t;
typedef struct config_struct {
    actor_t *   actor;          // actor roster
    event_t *   event;          // event queue
    WORD        actors;         // actor creation limit
    WORD        events;         // message-send event limit
    WORD        current;        // current-event index
} config_t;

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

/*
 * Sponsor
 */

typedef struct sponsor_struct sponsor_t;
extern sponsor_t * sponsor;  // WE DECLARE A GLOBAL SPONSOR TO AVOID THREADING IT THROUGH ALL OTHER CALLS...

typedef struct sponsor_struct {
    pool_t *    pool;
    config_t *  config;
} sponsor_t;

#define SPONSOR_POOL(sponsor) ((sponsor)->pool)  // (sponsor_t *) -> (pool_t *)
#define SPONSOR_CONFIG(sponsor) ((sponsor)->config)  // (sponsor_t *) -> (config_t *)
#define SPONSOR_EVENT(sponsor) CONFIG_EVENT(SPONSOR_CONFIG(sponsor))  // (sponsor_t *) -> (event_t *)

sponsor_t * new_sponsor(pool_t * pool, WORD actors, WORD events);

#endif // _SPONSOR_H_
