/*
 *	LOG output facilities
 */

typedef enum {
	LOG_LEVEL_NONE,
	LOG_LEVEL_INFO,
	LOG_LEVEL_WARN,
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_TRACE
} log_level;

struct {
	log_level level;
} log_config = {
	.level = LOG_LEVEL_INFO
};

void log_event(			// log event synchronously
	char * _file_,
	int _line_,
	log_level level,
	char * label,
	uintptr_t value
);

#define LOG_NONE(label, value) /* LOG_NONE */

#ifdef LOG_ALL
#define LOG_INFO
#define LOG_WARN
#define LOG_DEBUG
#define LOG_TRACE
#define LOG_LEVEL
#endif

#ifdef LOG_INFO
#undef LOG_INFO
#define LOG_INFO(label, value) log_event( \
	__FILE__, __LINE__,  \
	LOG_LEVEL_INFO,      \
	(label), (value) )
#else
#define LOG_INFO(label, value) /* LOG_INFO */
#endif

#ifdef LOG_WARN
#undef LOG_WARN
#define LOG_WARN(label, value) log_event( \
	__FILE__, __LINE__,  \
	LOG_LEVEL_WARN,      \
	(label), (value) )
#else
#define LOG_WARN(label, value) /* LOG_WARN */
#endif

#ifdef LOG_DEBUG
#undef LOG_DEBUG
#define LOG_DEBUG(label, value) log_event( \
	__FILE__, __LINE__,	\
	LOG_LEVEL_DEBUG,    \
	(label), (value) )
#else
#define LOG_DEBUG(label, value) /* LOG_DEBUG */
#endif

#ifdef LOG_TRACE
#undef LOG_TRACE
#define LOG_TRACE(label, value) log_event( \
	__FILE__, __LINE__, \
	LOG_LEVEL_TRACE,    \
	(label), (value) )
#else
#define LOG_TRACE(label, value) /* LOG_TRACE */
#endif

#ifdef LOG_LEVEL
#undef LOG_LEVEL
#define LOG_LEVEL(level, label, value) log_event( \
	__FILE__, __LINE__, \
	(level),            \
	(label), (value) )
#else
#define LOG_LEVEL(level, label, value) /* LOG_LEVEL */
#endif


/*
 *	All logging comes through a single synchronous enter-point.
 *	Different implementations of `log_event()` can vary recording/output strategies.
 */

#include <stdio.h>

// FIXME: should automatically include timestamp in log event
uint64_t log_time();	// log-event timestamp

void log_event(			// log event synchronously
	char * _file_,
	int _line_,
	log_level level,
	char * label,
	uintptr_t value
) {
	static char * level_name[] = { "NONE", "INFO", "WARN", "DEBUG", "TRACE" };

	if (level > log_config.level) return;  // ignore logging above configured threshold
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
