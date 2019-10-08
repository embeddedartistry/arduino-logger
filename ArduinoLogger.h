#ifndef ARDUINO_LOGGER_H_
#define ARDUINO_LOGGER_H_

#include "_log_common_defs.h"
#include <string>
#include "ArduinoPrintf.h" // for definition of _putchar, fctprintf

enum log_level_e
{
	off = LOG_LEVEL_OFF,
	critical = LOG_LEVEL_CRITICAL,
	error = LOG_LEVEL_ERROR,
	warn = LOG_LEVEL_WARNING,
	info = LOG_LEVEL_INFO,
	debug = LOG_LEVEL_DEBUG,
};

constexpr log_level_e LOG_LEVEL_LIMIT() {
	return static_cast<log_level_e>(LOG_LEVEL);
}

constexpr const char* LOG_LEVEL_TO_C_STRING(log_level_e level)
{
	constexpr const char* level_string_names[] = LOG_LEVEL_NAMES;
	return level_string_names[level];
}

constexpr const char* LOG_LEVEL_TO_SHORT_C_STRING(log_level_e level)
{
	constexpr const char* level_short_names[] = LOG_LEVEL_SHORT_NAMES;
	return level_short_names[level];
}

class LoggerBase
{
public:
	/** Get the current log buffer size
	 *
	 * Derived classes must implement this function.
	 *
	 * @returns The current size of the log buffer, in bytes.
	 */
	virtual size_t size() const noexcept = 0;

	/** Get the log buffer capacity
	 *
	 * Derived classes must implement this function.
	 *
	 * @returns The total capacity of the log buffer, in bytes.
	 */
	virtual size_t capacity() const noexcept = 0;

	/** Check if the log is enabled.
	 *
	 * @returns true if log output is enabled, false if it is disabled.
	 */
	bool enabled() const noexcept
	{
		return enabled_;
	}

	/** Check the echo setting
	 *
	 * @returns true if echo to console is enabled, false if disabled.
	 */
	bool echo() const noexcept
	{
		return echo_;
	}

	/** Enable/disable echo to console.
	 *
	 * @param en Echo switch. If true, log statements will also be echo'd to the console through
	 * printf(). If false, log statements will only go to the log buffer.
	 * @returns true if echo to console is enabled, false if disabled.
	 */
	bool echo(bool en) noexcept
	{
		echo_ = en;
		return echo_;
	}

	/** Get the maximum log level (filtering)
	 *
	 * @returns the current log level maximum.
	 */
	log_level_e level() const noexcept
	{
		return level_;
	}

	/** Set the maximum log level (filtering)
	 *
	 * @param l The maximum log level. Levels grater than `l` will not be added to the log buffer.
	 * @returns the current log level maximum.
	 */
	log_level_e level(log_level_e l) noexcept
	{
		if(l <= LOG_LEVEL_LIMIT())
		{
			level_ = l;
		}

		return level_;
	}

	/** Add data to the log buffer
	 *
	 * @tparam Args Variadic template args. Will be deduced by the compiler. Enables support for
	 *	a variadic function template.
	 * @param l The log level associated with this statement.
	 * @param fmt The log format string.
	 * @param args The variadic arguments that are associated with the format string.
	 */
	template<typename... Args>
	void log(log_level_e l, const char* fmt, const Args&... args) noexcept
	{
		if(enabled_ && l <= level_)
		{
			// TODO: timestamp
#if 0
			if(system_clock_)
			{
				fctprintf(&LoggerBase::log_putc_bounce, this, "[%llu] ", system_clock_->ticks());
			}
#endif

			// Add our prefix
			fctprintf(&LoggerBase::log_putc_bounce, this, "<%s> ", LOG_LEVEL_TO_SHORT_C_STRING(l));

			// Send the primary log statement
			fctprintf(&LoggerBase::log_putc_bounce, this, fmt, args...);

			if(echo_)
			{
			// TODO: timestamp
#if 0
				if(system_clock_)
				{
					printf("[%llu] ", system_clock_->ticks());
				}
#endif
				printf("<%s> ", LOG_LEVEL_TO_SHORT_C_STRING(l));

				// cppcheck-suppress wrongPrintfScanfArgNum
				printf("%s", fmt, args...);
			}
		}
	}



	/** Print the contents of the log buffer in the console.
	 *
	 * When called, the contents of the log buffer will be echo'd to the console through printf.
	 * The entire log buffer will be displayed.
	 *
	 * Derived classes must implement this function.
	 */
	virtual void dump() noexcept = 0;

	/** Clear the contents of the log buffer.
	 *
	 * Reset the log buffer to an empty state.
	 *
	 * @post The log buffer will be empty.
	 */
	virtual void clear() noexcept = 0;

protected:
	/// Default constructor
	LoggerBase() = default;

	/** Initialize the logger with options
	 *
	 * @param enable If true, log statements will be output to the log buffer. If false,
	 * logging will be disabled and log statements will not be output to the log buffer.
	 * @param l Runtime log filtering level. Levels greater than the target will not be output
	 * to the log buffer.
	 * @param echo If true, log statements will be logged and printed to the console with printf().
	 * If false, log statements will only be added to the log buffer.
	 */
	explicit LoggerBase(bool enable, log_level_e l = LOG_LEVEL_LIMIT(),
						bool echo = LOG_ECHO_EN_DEFAULT) noexcept
		: enabled_(enable), level_(l), echo_(echo)
	{
	}

	/// Default destructor
	virtual ~LoggerBase() = default;

	/** Log buffer putc function
	 *
	 * This function adds a character to the underlying log buffer.
	 *
	 * This function is used with the fctprintf() interface to output to the log buffer.
	 * This enables the framework to reuse the same print formatting for both logging and printf().
	 *
	 * Derived classes must implement this function.
	 *
	 * @param c The character to insert into the log buffer.
	 */
	virtual void log_putc(char c) = 0;

	/** putc bounce function
	 *
	 * This is a bounce function which registers with the C printf API. We use the private parameter
	 * to store the `this` pointer so we can get back to our logger's putc instance.
	 *
	 * @param c The character to log.
	 * @param this_ptr The this pointer of the logger instance. Used to invoke log_putc() on the
	 * correct instance.
	 */
	static void log_putc_bounce(char c, void* this_ptr)
	{
		reinterpret_cast<LoggerBase*>(this_ptr)->log_putc(c);
	}

  private:
	/// Indicates whether logging is currently enabled
	bool enabled_ = LOG_EN_DEFAULT;

	/// The current log level.
	/// Levels greater than the current setting will be filtered out.
	log_level_e level_ = LOG_LEVEL_LIMIT();

	/// Console echoing.
	/// If true, log statements will be printed to the console through printf().
	bool echo_ = LOG_ECHO_EN_DEFAULT;
};

/** Declare a static platform logger instance.
 *
 * This class is used to declare a static platform logger instance.
 *
 * Define a platform logger which is templated on the log type:
 *
 * @code
 * using PlatformLogger = PlatformLogger_t<CircularLogBufferLogger<8 * 1024>>;
 * @endcode
 *
 * This `PlatformLogger` type will then work with the logging macros (loginfo(), etc.).
 *
 * The instance can also be grabbed manually:
 *
 * @code
 * PlatformLogger::inst().dump();
 * @endcode
 */
template<class TLogger>
class PlatformLogger_t
{
  public:
	PlatformLogger_t() = default;
	~PlatformLogger_t() = default;

	static TLogger& inst()
	{
		static TLogger logger_;
		return logger_;
	}
};

/** @name Logging Macros
 *
 * The log macros can be overridden by defining them in your platform_logger.h file
 *
 * Warning is the default log level if one is not supplied
 *
 * For more information see @ref docs/development/ExtendingTheFramework/customizing_log_macros.md
 *
 * @{
 */

// Supply a default log level
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_WARN
#endif

#if LOG_LEVEL >= LOG_LEVEL_CRITICAL
#ifndef logcritical
#define logcritical(...) PlatformLogger::inst().log(log_level_e::critical, __VA_ARGS__)
#endif
#else
#define logcritical(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_ERROR
#ifndef logerror
#define logerror(...) PlatformLogger::inst().log(log_level_e::error, __VA_ARGS__)
#endif
#else
#define logcerror(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_WARN
#ifndef logwarn
#define logwarn(...) PlatformLogger::inst().log(log_level_e::warn, __VA_ARGS__)
#endif
#else
#define logwarn(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_INFO
#ifndef loginfo
#define loginfo(...) PlatformLogger::inst().log(log_level_e::info, __VA_ARGS__)
#endif
#else
#define loginfo(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_DEBUG
#ifndef logdebug
#define logdebug(...) PlatformLogger::inst().log(log_level_e::debug, __VA_ARGS__)
#endif
#else
#define logdebug(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
#ifndef logverbose
#define logverbose(...) PlatformLogger::inst().log(log_level_e::verbose, __VA_ARGS__)
#endif
#else
#define logverbose(...)
#endif

#endif //ARDUINO_LOGGER_H_
