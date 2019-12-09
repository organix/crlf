/*
 * abcm.c -- Actor Byte-Code Machine
 */
#include <stddef.h>  // for NULL, size_t, et. al.
#include <assert.h>

#include "abcm.h"
#include "bose.h"
#include "device.h"
#include "test.h"
#include "sponsor.h"
#include "pool.h"
#include "program.h"

#define LOG_ALL // enable all logging
#include "log.h"

char * _semver = "0.0.8";

/*
 * include actor-byte-code bootstrap program...
 */
BYTE bootstrap[] = {
#include "hello_world.abc"
//#include "basic_scope.abc"
//#include "fail_example.abc"
//#include "two_sponsor.abc"
//#include "stream_reader.abc"
//#include "peg_parser.abc"
//#include "lambda_calculus.abc"
//#include "testcase.abc"
};

#define LOAD_2ND_PROGRAM 0 /* test loading of multiple top-level programs */
#if LOAD_2ND_PROGRAM
BYTE boot2nd[] = {
//#include "hello_world.abc"
//#include "basic_scope.abc"
//#include "fail_example.abc"
//#include "two_sponsor.abc"
//#include "stream_reader.abc"
//#include "peg_parser.abc"
//#include "lambda_calculus.abc"
//#include "testcase.abc"
};
#endif

int run_abcm() {  // ok == 0, fail != 0
    int result = 0;
    //log_config.level = LOG_LEVEL_WARN;
    //log_config.level = LOG_LEVEL_DEBUG;
    //log_config.level = LOG_LEVEL_TRACE+2;

    assert(_semver == _semver);  // FIXME: vacuous use of `_semver`, to satisfy compiler...
    LOG_INFO(_semver, (WORD)_semver);

    // establish (global) testing sponsor
    sponsor = new_sponsor(heap_pool, NULL, 0, 0);
    assert(sponsor);
    result = run_test_suite();  // pass == 0, fail != 0
    if (result) return result;
    if (!sponsor_release(&sponsor)) return 1;  // reclamation failure!
    show_pool_metrics();
    assert(audit_check_leaks() == 0);  // the test suite should not leak memory.

    // establish (global) bootstrap sponsor
    log_config.level = LOG_LEVEL_WARN;
    //log_config.level = LOG_LEVEL_DEBUG;
    //log_config.level = LOG_LEVEL_TRACE;
    //log_config.level = LOG_LEVEL_TRACE+1;
    //log_config.level = LOG_LEVEL_TRACE+2;
#if REF_COUNTED_BOOT_SPONSOR
    pool_t * pool = new_ref_pool(heap_pool);
    if (!pool) return 1;  // allocation failure!
    sponsor_t * boot_sponsor = new_sponsor(pool, NULL, 0, 0);
#else
    sponsor_t * boot_sponsor = new_sponsor(heap_pool, NULL, 0, 0);
#endif
    if (!boot_sponsor) return 1;  // allocation failure!
    sponsor = boot_sponsor;  // set global sponsor
    boot_sponsor = TRACK(sponsor);  // WARNING! can't TRACK until `sponsor` is set...
    if (!device_startup()) return -1;  // device startup failed!
    if (!load_program(bootstrap)) {
        LOG_WARN("run_abcm: load_program failed!", (WORD)bootstrap);
        return 1;  // load_program failed!
    }
#if LOAD_2ND_PROGRAM
    if (!load_program(boot2nd)) {
        LOG_WARN("run_abcm: load_program failed!", (WORD)boot2nd);
        return 1;  // load_program failed!
    }
#endif
    if (!sponsor_dispatch_loop(SPONSOR_NEXT(sponsor))) {
        LOG_WARN("run_abcm: sponsor dispatch loop failed!", (WORD)sponsor);
    }
    sponsor = boot_sponsor;  // restore previous global sponsor
    if (!device_shutdown()) return -1;  // device shutdown failed!
#if REF_COUNTED_BOOT_SPONSOR
    assert(pool == SPONSOR_POOL(sponsor));
    if (!sponsor_release(&sponsor)) return 1;  // reclamation failure!
    if (!RELEASE_FROM(heap_pool, (DATA_PTR *)&pool)) return 1;  // reclamation failure!
#else
    if (!sponsor_release(&sponsor)) return 1;  // reclamation failure!
#endif
    show_pool_metrics();
#if 1
    assert(audit_check_leaks() == 0);
#else
    audit_check_leaks();  // FIXME: the runtime has leaks, fix them...
#endif

    return result;
}

#ifdef MAIN
#include <stdlib.h>

static WORD startup(WORD hook) {
    LOG_INFO("startup: hook =", hook);
    LOG_TRACE("startup: &device_startup_hook", (WORD)&device_startup_hook);
    LOG_TRACE("startup: device_startup_hook", (WORD)device_startup_hook);
    LOG_TRACE("startup: &device_call_hook", (WORD)&device_call_hook);
    LOG_TRACE("startup: device_call_hook", (WORD)device_call_hook);
    LOG_TRACE("startup: &device_shutdown_hook", (WORD)&device_shutdown_hook);
    LOG_TRACE("startup: device_shutdown_hook", (WORD)device_shutdown_hook);
    return 0;
}

static WORD shutdown(WORD hook) {
    LOG_INFO("shutdown: hook =", hook);
    CODE_PTR * hook_ptr = (CODE_PTR *)hook;
    *hook_ptr = (CODE_PTR)0;  // clear device_call_hook
    LOG_TRACE("shutdown: &device_startup_hook", (WORD)&device_startup_hook);
    LOG_TRACE("shutdown: device_startup_hook", (WORD)device_startup_hook);
    LOG_TRACE("shutdown: &device_call_hook", (WORD)&device_call_hook);
    LOG_TRACE("shutdown: device_call_hook", (WORD)device_call_hook);
    LOG_TRACE("shutdown: &device_shutdown_hook", (WORD)&device_shutdown_hook);
    LOG_TRACE("shutdown: device_shutdown_hook", (WORD)device_shutdown_hook);
    return 0;
}

int main(int argc, char *argv[]) {
    device_startup_hook = startup;
    device_shutdown_hook = shutdown;

    int exit_code = run_abcm();  // ok == 0, fail != 0

    return (exit(exit_code), exit_code);
}

#endif // MAIN
