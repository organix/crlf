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
extern BYTE s_message[];
extern BYTE s_actor[];
extern BYTE s_behavior[];
extern BYTE s_name[];
extern BYTE s_value[];
extern BYTE s_error[];
/*
// Actions
    { "kind":"actor_send", "message":<dictionary>, "actor":<address> }
    { "kind":"actor_become", "behavior":<behavior> }
    { "kind":"actor_ignore" }
    { "kind":"actor_assign", "name":<string>, "value":<expression> }
    { "kind":"actor_fail", "error":<expression> }
*/
extern BYTE k_actor_send[];
extern BYTE k_actor_become[];
extern BYTE k_actor_ignore[];
extern BYTE k_actor_assign[];
extern BYTE k_actor_fail[];

int start_abcm();  // ok == 0, fail != 0

#endif // _ABCM_H_
