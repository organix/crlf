/*
 * abcm.h -- Actor Byte-Code Machine
 */
#ifndef _ABCM_H_
#define _ABCM_H_

#include "bose.h"

extern char * _semver;  // semantic version number (C-string)

int run_abcm();  // ok == 0, fail != 0

#endif // _ABCM_H_
