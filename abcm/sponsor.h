/*
 * sponsor.h -- actor resource provider/manager
 */
#ifndef _SPONSOR_H_
#define _SPONSOR_H_

#include "bose.h"
#include "pool.h"

typedef struct sponsor_struct sponsor_t;
typedef struct sponsor_struct {
	// actor primitives
	BYTE 		(*create)(sponsor_t * sponsor, DATA_PTR state, DATA_PTR behavior);
	BYTE 		(*send)(sponsor_t * sponsor, DATA_PTR address, DATA_PTR message);
	BYTE 		(*become)(sponsor_t * sponsor, DATA_PTR behavior);
	BYTE 		(*fail)(sponsor_t * sponsor, DATA_PTR error);
	// memory management
	BYTE 		(*reserve)(sponsor_t * sponsor, DATA_PTR * data, WORD size);
	BYTE 		(*share)(sponsor_t * sponsor, DATA_PTR * data);
	BYTE 		(*copy)(sponsor_t * sponsor, DATA_PTR * data, DATA_PTR value);
	BYTE 		(*release)(sponsor_t * sponsor, DATA_PTR * data);
} sponsor_t;

BYTE sponsor_create(sponsor_t * sponsor, DATA_PTR state, DATA_PTR behavior);
BYTE sponsor_send(sponsor_t * sponsor, DATA_PTR address, DATA_PTR message);
BYTE sponsor_become(sponsor_t * sponsor, DATA_PTR behavior);
BYTE sponsor_fail(sponsor_t * sponsor, DATA_PTR error);

BYTE sponsor_reserve(sponsor_t * sponsor, DATA_PTR * data, WORD size);
BYTE sponsor_share(sponsor_t * sponsor, DATA_PTR * data);
BYTE sponsor_copy(sponsor_t * sponsor, DATA_PTR * data, DATA_PTR value);
BYTE sponsor_release(sponsor_t * sponsor, DATA_PTR * data);

sponsor_t * new_bounded_sponsor(DATA_PTR actors, DATA_PTR events, pool_t * work_pool);

#endif // _SPONSOR_H_
