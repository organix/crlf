/*
 * sponsor.h -- actor resource provider/manager
 */
#ifndef _SPONSOR_H_
#define _SPONSOR_H_

#include "bose.h"
#include "pool.h"
#include "event.h"

/**

#### Configuration
A collection of _Actors_ and (pending) _Events_.

Fields:
  * _Actors_: All Actors contained in this Configuration
  * _Events_: Pending message-delivery Events
  * _Limits_: Configuration resource limits

Methods:
  * _Dispatch_: Attempt to deliver a pending _Event_
  * _Apply_: Apply message-delivery Effects

**/

typedef struct config_struct config_t;
extern config_t * sponsor_config;  // FIXME: THIS SHOULD BE A MEMBER OF SPONSOR!
#define SPONSOR_CONFIG(sponsor) sponsor_config  // FIXME: this macro will eventually access through sponsor, but for now...

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

BYTE config_dispatch(config_t * config);
BYTE config_apply(config_t * config, effect_t * effect);

/**
Instead of storing new actors/events in Effects,
we create them directly in the Configuration,
because most-likely they will persist.
If we have to revert (on failure), we clean up the extra actors/events.
**/
BYTE config_create(config_t * config, scope_t * parent, DATA_PTR state, DATA_PTR behavior, DATA_PTR * address);
BYTE config_send(config_t * config, DATA_PTR address, DATA_PTR message);

/**

#### Sponsor
A root object, providing access to resource-management mechanisms for computations.

Fields:
  * _Pool_: Working-memory pool
  * _Configuration_: A collection of _Actors_ and _Events_
  * _Event_: Current message-delivery event

Methods:
  * _(none)_

**/

typedef struct sponsor_struct sponsor_t;
extern sponsor_t * sponsor;  // WE DECLARE A GLOBAL SPONSOR TO AVOID THREADING IT THROUGH ALL OTHER CALLS...

BYTE event_lookup_behavior(sponsor_t * sponsor, event_t * event, DATA_PTR * behavior);
BYTE event_update_behavior(sponsor_t * sponsor, event_t * event, DATA_PTR behavior);
BYTE event_lookup_actor(sponsor_t * sponsor, event_t * event, DATA_PTR * self);
BYTE event_lookup_message(sponsor_t * sponsor, event_t * event, DATA_PTR * message);
BYTE event_lookup_scope(sponsor_t * sponsor, event_t * event, scope_t ** scope);
BYTE event_init_effects(sponsor_t * sponsor, event_t * event, WORD actors, WORD events);
BYTE event_apply_effects(sponsor_t * sponsor, event_t * event);
BYTE event_revert_effects(sponsor_t * sponsor, event_t * event);

typedef struct sponsor_struct {
    BYTE        (*dispatch)(sponsor_t * sponsor);
    // actor primitives
    BYTE        (*create)(sponsor_t * sponsor, scope_t * scope, DATA_PTR state, DATA_PTR behavior, DATA_PTR * address);
    BYTE        (*send)(sponsor_t * sponsor, DATA_PTR address, DATA_PTR message);
    BYTE        (*become)(sponsor_t * sponsor, DATA_PTR behavior);
    BYTE        (*fail)(sponsor_t * sponsor, event_t * event, DATA_PTR error);
} sponsor_t;

BYTE sponsor_dispatch(sponsor_t * sponsor);

BYTE sponsor_create(sponsor_t * sponsor, scope_t * scope, DATA_PTR state, DATA_PTR behavior, DATA_PTR * address);
BYTE sponsor_send(sponsor_t * sponsor, DATA_PTR address, DATA_PTR message);
BYTE sponsor_become(sponsor_t * sponsor, DATA_PTR behavior);
BYTE sponsor_fail(sponsor_t * sponsor, event_t * event, DATA_PTR error);

sponsor_t * new_bounded_sponsor(WORD actors, WORD events, pool_t * pool);

#endif // _SPONSOR_H_
