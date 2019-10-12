/*
 * abcm.c -- Actor Byte-Code Machine
 */
#include <stdlib.h>
#include <assert.h>

#include "abcm.h"
#include "bose.h"
#include "equal.h"
#include "log.h"
#include "test.h"

static char * _semver = "0.0.1";

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

int main(int argc, char *argv[]) {
    log_config.level = LOG_LEVEL_WARN;
    //log_config.level = LOG_LEVEL_TRACE+1;
    assert(_semver == _semver);  // FIXME: vacuous use of `_semver`, to satisfy compiler...
    LOG_INFO(_semver, (WORD)_semver);
    memo_clear();
    int result = run_test_suite();  // pass == 0, fail != 0
    return (exit(result), result);
}
