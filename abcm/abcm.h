/*
 * abcm.h -- Actor Byte-Code Machine
 */
#ifndef _ABCM_H_
#define _ABCM_H_

#include "bose.h"

extern char * _semver;  // semantic version number (C-string)

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
extern BYTE s_args[];
extern BYTE s_in[];
extern BYTE s_with[];
extern BYTE s_const[];
extern BYTE s_level[];
extern BYTE s_error[];

extern BYTE k_actor_sponsor[];
extern BYTE k_actor_send[];
extern BYTE k_actor_become[];
extern BYTE k_actor_ignore[];
extern BYTE k_actor_assign[];
extern BYTE k_actor_fail[];
extern BYTE k_actor_behavior[];
extern BYTE k_actor_create[];
extern BYTE k_actor_message[];
extern BYTE k_actor_self[];
extern BYTE k_actor_has_state[];
extern BYTE k_actor_state[];
extern BYTE k_dict_has[];
extern BYTE k_dict_get[];
extern BYTE k_dict_bind[];
extern BYTE k_expr_literal[];
extern BYTE k_expr_operation[];
extern BYTE k_log_print[];  // --DEPRECATED--

int start_abcm();  // ok == 0, fail != 0

#endif // _ABCM_H_
