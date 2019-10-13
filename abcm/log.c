/*
 * log.c -- event logging instrumentation
 */
#include "log.h"

log_config_t log_config = {
    .level = LOG_LEVEL_INFO
};

// FIXME: should automatically include timestamp in log event
/*
uint64_t log_time();    // log-event timestamp
*/

/*
 *  All logging comes through a single synchronous enter-point.
 *  Different implementations of `log_event()` can vary recording/output strategies.
 */

#include <stdio.h>
#include <string.h>

void log_event(         // log event synchronously
    char * _file_,
    int _line_,
    log_level_t level,
    char * label,
    uintptr_t value
) {
    static char * level_name[] = { "NONE", "INFO", "WARN", "DEBUG", "TRACE" };

    if (level > log_config.level) return;  // ignore logging above configured threshold
    char *s = strrchr(_file_, '/');
    if (s) {
        _file_ = s + 1;  // use just the basename
    }
    if (level > LOG_LEVEL_TRACE) {
        int plus = level - LOG_LEVEL_TRACE;
        fprintf(stderr, "%s:%d %s%+d %s (%p)\n", 
            _file_, _line_, level_name[LOG_LEVEL_TRACE], plus, label, (void *)value);
    } else {
        fprintf(stderr, "%s:%d %s %s (%p)\n", 
            _file_, _line_, level_name[level], label, (void *)value);
    }
    fflush(stderr);
}
