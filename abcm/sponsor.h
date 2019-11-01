/*
 * sponsor.h -- actor resource provider/manager
 */
#ifndef _SPONSOR_H_
#define _SPONSOR_H_

#include "bose.h"
#include "pool.h"
#include "event.h"

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
sponsor_t * new_pool_sponsor(sponsor_t * parent, pool_t * pool);

#endif // _SPONSOR_H_
