/*
 * abcm.h -- Actor Byte-Code Machine
 */
#ifndef _ABCM_H_
#define _ABCM_H_

#include "bose.h"

extern char * _semver;  // semantic version number (C-string)

#if 0  // FIXME: these types are currently unused, do we really need them?
typedef uint8_t NAT8;           // 8-bit natural ring
typedef uint16_t NAT16;         // 16-bit natural ring
typedef uint32_t NAT32;         // 32-bit natural ring
typedef uint64_t NAT64;         // 64-bit natural ring
#endif

extern BYTE s_kind[];
extern BYTE s_actors[];
extern BYTE s_events[];
extern BYTE s_script[];
extern BYTE s_message[];
extern BYTE s_actor[];
extern BYTE s_state[];
extern BYTE s_behavior[];
extern BYTE s_name[];
extern BYTE s_value[];
extern BYTE s_type[];
extern BYTE s_const[];
extern BYTE s_level[];
extern BYTE s_error[];

extern BYTE k_actor_sponsor[];
extern BYTE k_actor_send[];
extern BYTE k_actor_become[];
extern BYTE k_actor_ignore[];
extern BYTE k_actor_assign[];
extern BYTE k_actor_fail[];
extern BYTE k_log_print[];  // --DEPRECATED--
extern BYTE k_actor_behavior[];
extern BYTE k_actor_create[];
extern BYTE k_actor_state[];
extern BYTE k_actor_self[];
extern BYTE k_expr_literal[];

int start_abcm();  // ok == 0, fail != 0

#endif // _ABCM_H_
