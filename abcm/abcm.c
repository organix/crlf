/*
 * abcm.c -- Actor Byte-Code Machine
 */
#include <assert.h>

#include "abcm.h"
#include "bose.h"
#include "test.h"
#include "program.h"

#define LOG_ALL // enable all logging
#include "log.h"


char * _semver = "0.0.2";

// NOTE: the '\0'-terminators are not required, but interoperate better with C
BYTE s_kind[] = { utf8, n_4, 'k', 'i', 'n', 'd', '\0' };
BYTE s_actors[] = { utf8, n_6, 'a', 'c', 't', 'o', 'r', 's', '\0' };
BYTE s_events[] = { utf8, n_6, 'e', 'v', 'e', 'n', 't', 's', '\0' };
BYTE s_script[] = { utf8, n_6, 's', 'c', 'r', 'i', 'p', 't', '\0' };
BYTE s_message[] = { utf8, n_7, 'm', 'e', 's', 's', 'a', 'g', 'e', '\0' };
BYTE s_actor[] = { utf8, n_5, 'a', 'c', 't', 'o', 'r', '\0' };
BYTE s_behavior[] = { utf8, n_8, 'b', 'e', 'h', 'a', 'v', 'i', 'o', 'r', '\0' };
BYTE s_name[] = { utf8, n_4, 'n', 'a', 'm', 'e', '\0' };
BYTE s_value[] = { utf8, n_5, 'v', 'a', 'l', 'u', 'e', '\0' };
BYTE s_level[] = { utf8, n_5, 'l', 'e', 'v', 'e', 'l', '\0' };
BYTE s_error[] = { utf8, n_5, 'e', 'r', 'r', 'o', 'r', '\0' };

/*
// Containers
    { "kind":"actor_sponsor", "actors":<number>, "events":<number>, "script":[<action>, ...] }
// Actions
    { "kind":"actor_send", "message":<dictionary>, "actor":<address> }
    { "kind":"actor_become", "behavior":<behavior> }
    { "kind":"actor_ignore" }
    { "kind":"actor_assign", "name":<string>, "value":<expression> }
    { "kind":"actor_fail", "error":<expression> }
// Device Interface
    { "kind":"log_print", "level":<number>, "value":<expression> }
*/
BYTE k_actor_sponsor[] = { utf8, n_13, 'a', 'c', 't', 'o', 'r', '_', 's', 'p', 'o', 'n', 's', 'o', 'r', '\0' };
BYTE k_actor_send[] = { utf8, n_10, 'a', 'c', 't', 'o', 'r', '_', 's', 'e', 'n', 'd', '\0' };
BYTE k_actor_become[] = { utf8, n_12, 'a', 'c', 't', 'o', 'r', '_', 'b', 'e', 'c', 'o', 'm', 'e', '\0' };
BYTE k_actor_ignore[] = { utf8, n_12, 'a', 'c', 't', 'o', 'r', '_', 'i', 'g', 'n', 'o', 'r', 'e', '\0' };
BYTE k_actor_assign[] = { utf8, n_12, 'a', 'c', 't', 'o', 'r', '_', 'a', 's', 's', 'i', 'g', 'n', '\0' };
BYTE k_actor_fail[] = { utf8, n_10, 'a', 'c', 't', 'o', 'r', '_', 'f', 'a', 'i', 'l', '\0' };
BYTE k_log_print[] = { utf8, n_9, 'l', 'o', 'g', '_', 'p', 'r', 'i', 'n', 't', '\0' };

int start_abcm() {  // ok == 0, fail != 0
    int result = 0;
    //log_config.level = LOG_LEVEL_WARN;
    //log_config.level = LOG_LEVEL_DEBUG;
    log_config.level = LOG_LEVEL_TRACE+1;

    assert(_semver == _semver);  // FIXME: vacuous use of `_semver`, to satisfy compiler...
    LOG_INFO(_semver, (WORD)_semver);

    result = run_test_suite();  // pass == 0, fail != 0
    if (result) return result;

    result = run_program(bootstrap);  // pass == 0, fail != 0
    if (result) return result;

    return result;
}

#ifdef MAIN
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int exit_code = start_abcm();  // ok == 0, fail != 0
    return (exit(exit_code), exit_code);
}

#endif // MAIN
