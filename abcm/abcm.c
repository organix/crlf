/*
 * abcm.c -- Actor Byte-Code Machine
 */
#include <assert.h>

#include "abcm.h"
#include "bose.h"
#include "equal.h"
#include "log.h"
#include "test.h"

char * _semver = "0.0.1";

// NOTE: the '\0'-terminators are not required, but interoperate better with C
BYTE s_kind[] = { utf8, n_4, 'k', 'i', 'n', 'd', '\0' };
BYTE s_message[] = { utf8, n_7, 'm', 'e', 's', 's', 'a', 'g', 'e', '\0' };
BYTE s_actor[] = { utf8, n_5, 'a', 'c', 't', 'o', 'r', '\0' };
BYTE s_behavior[] = { utf8, n_8, 'b', 'e', 'h', 'a', 'v', 'i', 'o', 'r', '\0' };
BYTE s_name[] = { utf8, n_4, 'n', 'a', 'm', 'e', '\0' };
BYTE s_value[] = { utf8, n_5, 'v', 'a', 'l', 'u', 'e', '\0' };
BYTE s_error[] = { utf8, n_5, 'e', 'r', 'r', 'o', 'r', '\0' };

/*
// Actions
    { "kind":"actor_send", "message":<dictionary>, "actor":<address> }
    { "kind":"actor_become", "behavior":<behavior> }
    { "kind":"actor_ignore" }
    { "kind":"actor_assign", "name":<string>, "value":<expression> }
    { "kind":"actor_fail", "error":<expression> }
*/
BYTE k_actor_send[] = { utf8, n_10, 'a', 'c', 't', 'o', 'r', '_', 's', 'e', 'n', 'd', '\0' };
BYTE k_actor_become[] = { utf8, n_12, 'a', 'c', 't', 'o', 'r', '_', 'b', 'e', 'c', 'o', 'm', 'e', '\0' };
BYTE k_actor_ignore[] = { utf8, n_12, 'a', 'c', 't', 'o', 'r', '_', 'i', 'g', 'n', 'o', 'r', 'e', '\0' };
BYTE k_actor_assign[] = { utf8, n_12, 'a', 'c', 't', 'o', 'r', '_', 'a', 's', 's', 'i', 'g', 'n', '\0' };
BYTE k_actor_fail[] = { utf8, n_10, 'a', 'c', 't', 'o', 'r', '_', 'f', 'a', 'i', 'l', '\0' };

int start_abcm() {  // ok == 0, fail != 0
    //log_config.level = LOG_LEVEL_WARN;
    log_config.level = LOG_LEVEL_DEBUG;
    //log_config.level = LOG_LEVEL_TRACE+1;

    // FIXME: vacuous use of `_semver`, to satisfy compiler...
    assert(_semver == _semver);
    LOG_INFO(_semver, (WORD)_semver);

    memo_clear();

    int result = run_test_suite();  // pass == 0, fail != 0

    return result;
}

#ifdef MAIN
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int exit_code = start_abcm();  // ok == 0, fail != 0
    return (exit(exit_code), exit_code);
}

#endif // MAIN
