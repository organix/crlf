/*
 * sponsor.h -- actor resource provider/manager
 */
#ifndef _SPONSOR_H_
#define _SPONSOR_H_

#include "bose.h"
#include "pool.h"
#include "event.h"

/*
 * The following convenience macros make it easy to inject auditing information to track/debug memory leaks
 */
#define AUDIT_ALLOCATION 0 /* track reserve/release calls to check for leaks */

#if AUDIT_ALLOCATION
#define RESERVE(dpp,size) audit_reserve(__FILE__, __LINE__, (sponsor), (dpp), (size))
#define COPY(to_dpp,from_dp) audit_copy(__FILE__, __LINE__, (sponsor), (to_dpp), (from_dp))
#define RELEASE(dpp) audit_release(__FILE__, __LINE__, (sponsor), (dpp))
#define TRACK(dp) audit_track(__FILE__, __LINE__, (sponsor), (DATA_PTR)(dp))
#else
#define RESERVE(dpp,size) sponsor_reserve(sponsor, (dpp), (size))
#define COPY(to_dpp,from_dp) sponsor_copy(sponsor, (to_dpp), (from_dp))
#define RELEASE(dpp) sponsor_release(sponsor, (dpp))
#define TRACK(dp) (dp)
#endif

typedef struct sponsor_struct sponsor_t;
extern sponsor_t * sponsor;  // WE DECLARE A GLOBAL SPONSOR TO AVOID THREADING IT THROUGH ALL OTHER CALLS...

#define PER_MESSAGE_LOCAL_SCOPE 1 // create a new empty scope per message, with the actor state as parent.

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
    // memory management
    BYTE        (*reserve)(sponsor_t * sponsor, DATA_PTR * data, WORD size);
    BYTE        (*copy)(sponsor_t * sponsor, DATA_PTR * data, DATA_PTR value);
    BYTE        (*release)(sponsor_t * sponsor, DATA_PTR * data);
    BYTE        (*temp_pool)(sponsor_t * sponsor, WORD size, sponsor_t ** child);
    BYTE        (*destroy)(sponsor_t * sponsor);
} sponsor_t;

BYTE sponsor_dispatch(sponsor_t * sponsor);

BYTE sponsor_create(sponsor_t * sponsor, scope_t * scope, DATA_PTR state, DATA_PTR behavior, DATA_PTR * address);
BYTE sponsor_send(sponsor_t * sponsor, DATA_PTR address, DATA_PTR message);
BYTE sponsor_become(sponsor_t * sponsor, DATA_PTR behavior);
BYTE sponsor_fail(sponsor_t * sponsor, event_t * event, DATA_PTR error);

BYTE sponsor_reserve(sponsor_t * sponsor, DATA_PTR * data, WORD size);
BYTE sponsor_copy(sponsor_t * sponsor, DATA_PTR * data, DATA_PTR value);
BYTE sponsor_release(sponsor_t * sponsor, DATA_PTR * data);
BYTE sponsor_temp_pool(sponsor_t * sponsor, WORD size, sponsor_t ** child);
BYTE sponsor_destroy(sponsor_t * sponsor);

sponsor_t * new_bounded_sponsor(WORD actors, WORD events, pool_t * pool);
sponsor_t * new_pool_sponsor(sponsor_t * parent, pool_t * pool);

BYTE audit_reserve(char * _file_, int _line_, sponsor_t * sponsor, DATA_PTR * data, WORD size);
BYTE audit_copy(char * _file_, int _line_, sponsor_t * sponsor, DATA_PTR * data, DATA_PTR value);
BYTE audit_release(char * _file_, int _line_, sponsor_t * sponsor, DATA_PTR * data);
VOID_PTR audit_track(char * _file_, int _line_, sponsor_t * sponsor, DATA_PTR address);
int audit_show_leaks();

#endif // _SPONSOR_H_
