/*
 * device.h -- Device native interface
 */
#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "bose.h"

extern CODE_PTR device_startup_hook;
extern CODE_PTR device_call_hook;
extern CODE_PTR device_shutdown_hook;

BYTE device_startup();
BYTE device_call(WORD input, WORD * output);
BYTE device_shutdown();

#endif // _DEVICE_H_
