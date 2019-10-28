/*
 * log.h -- event logging instrumentation
 */
#ifndef _LOG_H_
#define _LOG_H_

#include <stdint.h>

#ifdef LOG_ALL
#define LOG_INFO
#define LOG_WARN
#define LOG_DEBUG
#define LOG_TRACE
#define LOG_LEVEL
#endif

typedef enum {
    LOG_LEVEL_NONE,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE
} log_level_t;

typedef struct {
    log_level_t level;
} log_config_t;

extern log_config_t log_config;  // logging configuration options

// if you want to leave a statement in place, but disable it, use these...
#define LOG_NONE(label, value) /* LOG_NONE */
#define IF_NONE(statement) /* IF_NONE */

#ifdef LOG_INFO
#undef LOG_INFO
#define LOG_INFO(label, value) log_event( \
    __FILE__, __LINE__,  \
    LOG_LEVEL_INFO,      \
    (label), (value) )
#define IF_INFO(statement) if (log_config.level >= LOG_LEVEL_INFO) statement
#else
#define LOG_INFO(label, value) /* LOG_INFO */
#define IF_INFO(statement) /* IF_INFO */
#endif

#ifdef LOG_WARN
#undef LOG_WARN
#define LOG_WARN(label, value) log_event( \
    __FILE__, __LINE__,  \
    LOG_LEVEL_WARN,      \
    (label), (value) )
#define IF_WARN(statement) if (log_config.level >= LOG_LEVEL_WARN) statement
#else
#define LOG_WARN(label, value) /* LOG_WARN */
#define IF_WARN(statement) /* IF_WARN */
#endif

#ifdef LOG_DEBUG
#undef LOG_DEBUG
#define LOG_DEBUG(label, value) log_event( \
    __FILE__, __LINE__, \
    LOG_LEVEL_DEBUG,    \
    (label), (value) )
#define IF_DEBUG(statement) if (log_config.level >= LOG_LEVEL_DEBUG) statement
#else
#define LOG_DEBUG(label, value) /* LOG_DEBUG */
#define IF_DEBUG(statement) /* IF_DEBUG */
#endif

#ifdef LOG_TRACE
#undef LOG_TRACE
#define LOG_TRACE(label, value) log_event( \
    __FILE__, __LINE__, \
    LOG_LEVEL_TRACE,    \
    (label), (value) )
#define IF_TRACE(statement) if (log_config.level >= LOG_LEVEL_TRACE) statement
#else
#define LOG_TRACE(label, value) /* LOG_TRACE */
#define IF_TRACE(statement) /* IF_TRACE */
#endif

#ifdef LOG_LEVEL
#undef LOG_LEVEL
#define LOG_LEVEL(log_level, label, value) log_event( \
    __FILE__, __LINE__, \
    (log_level),            \
    (label), (value) )
#define IF_LEVEL(log_level, statement) if (log_config.level >= (log_level)) statement
#else
#define LOG_LEVEL(log_level, label, value) /* LOG_LEVEL */
#define IF_LEVEL(log_level, statement) /* IF_LEVEL */
#endif

#define DUMP_PARSE(label, parse) \
    LOG_INFO(label, (WORD)(parse));                   \
    LOG_INFO("      base =", (WORD)((parse)->base));  \
    LOG_INFO("      size =", (parse)->size);          \
    LOG_INFO("     start =", (parse)->start);         \
    LOG_INFO("       end =", (parse)->end);           \
    LOG_INFO("    prefix =", (parse)->prefix);        \
    LOG_INFO("      type =", (parse)->type);          \
    LOG_INFO("     value =", (parse)->value);         \
    LOG_INFO("     count =", (parse)->count);

/*
 *  All logging comes through a single synchronous enter-point.
 *  Different implementations of `log_event()` can vary recording/output strategies.
 */
void log_event(         // log event synchronously
    char * _file_,
    int _line_,
    log_level_t level,
    char * label,
    uintptr_t value
);

#endif // _LOG_H_
