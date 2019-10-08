#ifndef LOGGING_COMMON_DEFS_H_
#define LOGGING_COMMON_DEFS_H_

/// Logging is disabled
#define LOG_LEVEL_OFF 0
/// Indicates the system is unusable, or an error that is unrecoverable
#define LOG_LEVEL_CRITICAL 1
/// Indicates an error condition
#define LOG_LEVEL_ERROR 2
/// Indicates a warning condition
#define LOG_LEVEL_WARNING 3
/// Informational messages
#define LOG_LEVEL_INFO 4
/// Debug-level messages
#define LOG_LEVEL_DEBUG 5
/// The maximum log level that can be set
#define LOG_LEVEL_MAX LOG_LEVEL_DEBUG
/// The number of possible log levels
#define LOG_LEVEL_COUNT (LOG_LEVEL_MAX + 1)

#define LOG_LEVEL_CRITICAL_PREFIX "!"
#define LOG_LEVEL_ERROR_PREFIX "E"
#define LOG_LEVEL_WARNING_PREFIX "W"
#define LOG_LEVEL_INFO_PREFIX "I"
#define LOG_LEVEL_DEBUG_PREFIX "D"
#define LOG_LEVEL_INTERRUPT_PREFIX "int"

#ifndef LOG_LEVEL
/** Default maximum log level.
 *
 * This is the maximum log level that will be compiled in.
 * To set a custom log level, define the LOG_LEVEL before including this header
 * (e.g., as a compiler definition)
 */
#define LOG_LEVEL LOG_LEVEL_DEBUG
#endif

#ifndef LOG_EN_DEFAULT
/// Whether the logging module is enabled automatically on boot.
#define LOG_EN_DEFAULT true
#endif

#ifndef LOG_ECHO_EN_DEFAULT
/// Indicates that log statements should be echoed to the console
/// If true, log statements will be echoed.
/// If false, log statements will only go to the log.
#define LOG_ECHO_EN_DEFAULT false
#endif

#ifndef LOG_LEVEL_NAMES
/// Users can override these default names with a compiler definition
#define LOG_LEVEL_NAMES                                                   \
	{                                                                     \
		"off", "critical", "error", "warning", "info", "debug", 		  \
	}
#endif

#ifndef LOG_LEVEL_SHORT_NAMES
/// Users can override these default short names with a compiler definition
#define LOG_LEVEL_SHORT_NAMES                                                             \
	{                                                                                     \
		"Off", LOG_LEVEL_CRITICAL_PREFIX, LOG_LEVEL_ERROR_PREFIX, LOG_LEVEL_WARNING_PREFIX, \
			LOG_LEVEL_INFO_PREFIX, LOG_LEVEL_DEBUG_PREFIX,						          \
	}
#endif

/// @}

#endif // LOGGING_COMMON_DEFS_H_
