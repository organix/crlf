/*
 * device.c -- Device native interface
 */
//#include <string.h>  // for memcpy, et. al.
//#include <assert.h>

#include "bose.h"

#define LOG_ALL // enable all logging
//#define LOG_INFO
//#define LOG_WARN
#include "log.h"

CODE_PTR device_startup_hook;
CODE_PTR device_call_hook;
CODE_PTR device_shutdown_hook;

BYTE device_startup() {
    if (device_startup_hook) {
        LOG_DEBUG("device_startup: calling device_startup_hook", (WORD)device_startup_hook);
        WORD status = (*device_startup_hook)((WORD)&device_call_hook);
        device_startup_hook = (CODE_PTR)0;  // clear startup hook, so we only run once...
        return (status == 0);
    }
    return true;
}

BYTE device_call(WORD input, WORD * output) {
    if (device_startup_hook) {
        if (!device_startup()) return false;
    }
    if (device_call_hook) {
        LOG_DEBUG("device_call: calling device_call_hook", (WORD)device_call_hook);
        LOG_TRACE("device_call: input =", input);
        *output = (*device_call_hook)(input);
        LOG_TRACE("device_call: output =", *output);
    }
    return true;
}

BYTE device_shutdown() {
    if (device_shutdown_hook) {
        LOG_DEBUG("device_shutdown: calling device_shutdown_hook", (WORD)device_shutdown_hook);
        WORD status = (*device_shutdown_hook)((WORD)&device_call_hook);
        device_shutdown_hook = (CODE_PTR)0;  // clear shutdown hook, so we only run once...
        return (status == 0);
    }
    return true;
}
