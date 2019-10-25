/*
 * abcm.c -- Actor Byte-Code Machine
 */
#include <assert.h>

#include "abcm.h"
#include "bose.h"
#include "test.h"
#include "sponsor.h"
#include "program.h"

#define LOG_ALL // enable all logging
#include "log.h"


char * _semver = "0.0.4";

BYTE s_kind[] = { utf8, n_4, 'k', 'i', 'n', 'd' };
BYTE s_actors[] = { utf8, n_6, 'a', 'c', 't', 'o', 'r', 's' };
BYTE s_events[] = { utf8, n_6, 'e', 'v', 'e', 'n', 't', 's' };
BYTE s_script[] = { utf8, n_6, 's', 'c', 'r', 'i', 'p', 't' };
BYTE s_message[] = { utf8, n_7, 'm', 'e', 's', 's', 'a', 'g', 'e' };
BYTE s_actor[] = { utf8, n_5, 'a', 'c', 't', 'o', 'r' };
BYTE s_state[] = { utf8, n_5, 's', 't', 'a', 't', 'e' };
BYTE s_behavior[] = { utf8, n_8, 'b', 'e', 'h', 'a', 'v', 'i', 'o', 'r' };
BYTE s_name[] = { utf8, n_4, 'n', 'a', 'm', 'e' };
BYTE s_value[] = { utf8, n_5, 'v', 'a', 'l', 'u', 'e' };
BYTE s_type[] = { utf8, n_4, 't', 'y', 'p', 'e' };
BYTE s_in[] = { utf8, n_2, 'i', 'n' };
BYTE s_with[] = { utf8, n_4, 'w', 'i', 't', 'h' };
BYTE s_const[] = { utf8, n_5, 'c', 'o', 'n', 's', 't' };
BYTE s_level[] = { utf8, n_5, 'l', 'e', 'v', 'e', 'l' };
BYTE s_error[] = { utf8, n_5, 'e', 'r', 'r', 'o', 'r' };

/*
// Containers
    { "kind":"actor_sponsor", "actors":<number>, "events":<number>, "script":[<action>, ...] }
// Actions
    { "kind":"actor_send", "message":<dictionary>, "actor":<address> }
    { "kind":"actor_become", "behavior":<behavior> }
    { "kind":"actor_ignore" }
    { "kind":"actor_assign", "name":<string>, "value":<expression> }
    { "kind":"actor_fail", "error":<expression> }
    { "kind":"log_print", "level":<number>, "value":<expression> }  // --DEPRECATED--
// Address Expressions
    { "kind":"actor_create", "state":<dictionary>, "behavior":<behavior> }
    { "kind":"actor_self" }
// Behavior Expressions
    { "kind":"actor_behavior", "name":<string>, "script":[<action>, ...] }
// Value Expressions
    { "kind":"actor_state", "name":<string> }
    { "kind":"dict_get", "name":<string>, "in":<dictionary> }
    { "kind":"list_get", "at":<number>, "from":<list> }
    { "kind":"expr_literal", "const":<value> }
    { "kind":"expr_operation", "name":<string>, "args":[<expression>, ...] }
// Number Expressions
    { "kind":"list_length", "of":<list> }
// Boolean Expressions
    { "kind":"actor_has_state", "name":<string> }
    { "kind":"dict_has", "name":<string>, "in":<dictionary> }
// List (Array) Expressions
    { "kind":"list_add", "value":<expression>, "at":<number>, "to":<list> }
    { "kind":"list_remove", "at":<number>, "from":<list> }
// Dictionary (Object) Expressions
    { "kind":"actor_message" }
    { "kind":"dict_bind", "name":<string>, "value":<expression>, "with":<dictionary> }
*/
BYTE k_actor_sponsor[] = { utf8, n_13, 'a', 'c', 't', 'o', 'r', '_', 's', 'p', 'o', 'n', 's', 'o', 'r' };
BYTE k_actor_send[] = { utf8, n_10, 'a', 'c', 't', 'o', 'r', '_', 's', 'e', 'n', 'd' };
BYTE k_actor_become[] = { utf8, n_12, 'a', 'c', 't', 'o', 'r', '_', 'b', 'e', 'c', 'o', 'm', 'e' };
BYTE k_actor_ignore[] = { utf8, n_12, 'a', 'c', 't', 'o', 'r', '_', 'i', 'g', 'n', 'o', 'r', 'e' };
BYTE k_actor_assign[] = { utf8, n_12, 'a', 'c', 't', 'o', 'r', '_', 'a', 's', 's', 'i', 'g', 'n' };
BYTE k_actor_fail[] = { utf8, n_10, 'a', 'c', 't', 'o', 'r', '_', 'f', 'a', 'i', 'l' };
BYTE k_actor_behavior[] = { utf8, n_14, 'a', 'c', 't', 'o', 'r', '_', 'b', 'e', 'h', 'a', 'v', 'i', 'o', 'r' };
BYTE k_actor_create[] = { utf8, n_12, 'a', 'c', 't', 'o', 'r', '_', 'c', 'r', 'e', 'a', 't', 'e' };
BYTE k_actor_message[] = { utf8, n_13, 'a', 'c', 't', 'o', 'r', '_', 'm', 'e', 's', 's', 'a', 'g', 'e' };
BYTE k_actor_self[] = { utf8, n_10, 'a', 'c', 't', 'o', 'r', '_', 's', 'e', 'l', 'f' };
BYTE k_actor_has_state[] = { utf8, n_15, 'a', 'c', 't', 'o', 'r', '_', 'h', 'a', 's', '_', 's', 't', 'a', 't', 'e' };
BYTE k_actor_state[] = { utf8, n_11, 'a', 'c', 't', 'o', 'r', '_', 's', 't', 'a', 't', 'e' };
BYTE k_dict_has[] = { utf8, n_8, 'd', 'i', 'c', 't', '_', 'h', 'a', 's' };
BYTE k_dict_get[] = { utf8, n_8, 'd', 'i', 'c', 't', '_', 'g', 'e', 't' };
BYTE k_dict_bind[] = { utf8, n_9, 'd', 'i', 'c', 't', '_', 'b', 'i', 'n', 'd' };
BYTE k_expr_literal[] = { utf8, n_12, 'e', 'x', 'p', 'r', '_', 'l', 'i', 't', 'e', 'r', 'a', 'l' };
BYTE k_log_print[] = { utf8, n_9, 'l', 'o', 'g', '_', 'p', 'r', 'i', 'n', 't' };

int start_abcm() {  // ok == 0, fail != 0
    int result = 0;
    log_config.level = LOG_LEVEL_WARN;
    //log_config.level = LOG_LEVEL_DEBUG;
    //log_config.level = LOG_LEVEL_TRACE+1;

    assert(_semver == _semver);  // FIXME: vacuous use of `_semver`, to satisfy compiler...
    LOG_INFO(_semver, (WORD)_semver);

    result = run_test_suite();  // pass == 0, fail != 0
    if (result) return result;
    assert(audit_show_leaks() == 0);

    result = run_program(bootstrap);  // pass == 0, fail != 0
    if (result) return result;
    assert(audit_show_leaks() == 0);

    return result;
}

#ifdef MAIN
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int exit_code = start_abcm();  // ok == 0, fail != 0
    return (exit(exit_code), exit_code);
}

#endif // MAIN
