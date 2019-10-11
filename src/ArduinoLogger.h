#ifndef ARDUINO_LOGGER_H_
#define ARDUINO_LOGGER_H_

#include <printf.h>
#include <utility>

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

#define LOG_LEVEL_CRITICAL_PREFIX "<!> "
#define LOG_LEVEL_ERROR_PREFIX "<E> "
#define LOG_LEVEL_WARNING_PREFIX "<W> "
#define LOG_LEVEL_INFO_PREFIX "<I> "
#define LOG_LEVEL_DEBUG_PREFIX "<D> "
#define LOG_LEVEL_INTERRUPT_PREFIX "<int> "

// Supply a default log level
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
		"O", LOG_LEVEL_CRITICAL_PREFIX, LOG_LEVEL_ERROR_PREFIX, LOG_LEVEL_WARNING_PREFIX, \
			LOG_LEVEL_INFO_PREFIX, LOG_LEVEL_DEBUG_PREFIX,						          \
	}
#endif

enum log_level_e
{
	off = LOG_LEVEL_OFF,
	critical = LOG_LEVEL_CRITICAL,
	error = LOG_LEVEL_ERROR,
	warning = LOG_LEVEL_WARNING,
	info = LOG_LEVEL_INFO,
	debug = LOG_LEVEL_DEBUG,
};

class logNames
{
public:
	constexpr static const char* level_short_names[LOG_LEVEL_COUNT] = LOG_LEVEL_SHORT_NAMES;
	constexpr static const char* level_string_names[LOG_LEVEL_COUNT] = LOG_LEVEL_NAMES;
};

constexpr log_level_e LOG_LEVEL_LIMIT() {
	return static_cast<log_level_e>(LOG_LEVEL);
}

constexpr const char* LOG_LEVEL_TO_C_STRING(log_level_e level)
{
	return logNames::level_string_names[level];
}

constexpr const char* LOG_LEVEL_TO_SHORT_C_STRING(log_level_e level)
{
	return logNames::level_short_names[level];
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
	 * @param l The maximum log level. Levels greater than `l` will not be added to the log buffer.
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

	template<typename... Args>
	void critical(const char* fmt, const Args&... args)
	{
		log(log_level_e::critical, fmt, std::forward<const Args>(args)...);
	}

	template<typename... Args>
	void error(const char* fmt, const Args&... args)
	{
		log(log_level_e::error, fmt, std::forward<const Args>(args)...);
	}

	template<typename... Args>
	void warning(const char* fmt, const Args&... args)
	{
		log(log_level_e::warning, fmt, std::forward<const Args>(args)...);
	}

	template<typename... Args>
	void info(const char* fmt, const Args&... args)
	{
		log(log_level_e::info, fmt, std::forward<const Args>(args)...);
	}

	template<typename... Args>
	void debug(const char* fmt, const Args&... args)
	{
		log(log_level_e::debug, fmt, std::forward<const Args>(args)...);
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
			// TODO: put a formatter here?
#if 0
			if(system_clock_)
			{
				fctprintf(&LoggerBase::log_putc_bounce, this, "[%llu] ", system_clock_->ticks());
			}
#endif

			// Add our prefix
			fctprintf(&LoggerBase::log_putc_bounce, this, "%s", LOG_LEVEL_TO_SHORT_C_STRING(l));

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

	/** Print the buffered log contents to the target output stream
	 *
	 * When called, the contents of the log buffer will be removed and placed into
	 * the target output stream.
	 *
	 * Outputs can be any target. You will notice many example implementations print
	 * the log buffer contents to Serial when flush() is called.
	 *
	 * Derived classes must implement this function.
	 */
	virtual void flush() noexcept = 0;

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

	template<typename... Args>
	inline static void critical(const char* fmt, const Args&... args)
	{
		inst().critical(fmt, std::forward<const Args>(args)...);
	}

	template<typename... Args>
	inline static void error(const char* fmt, const Args&... args)
	{
		inst().error(fmt, std::forward<const Args>(args)...);
	}

	template<typename... Args>
	inline static void warning(const char* fmt, const Args&... args)
	{
		inst().warning(fmt, std::forward<const Args>(args)...);
	}

	template<typename... Args>
	inline static void info(const char* fmt, const Args&... args)
	{
		inst().info(fmt, std::forward<const Args>(args)...);
	}

	template<typename... Args>
	inline static void debug(const char* fmt, const Args&... args)
	{
		inst().debug(fmt, std::forward<const Args>(args)...);
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
#define logerror(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_WARNING
#ifndef logwarning
#define logwarning(...) PlatformLogger::inst().log(log_level_e::warning, __VA_ARGS__)
#endif
#else
#define logwarning(...)
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

#define logflush() PlatformLogger::inst().flush();
#define loglevel(lvl) PlatformLogger::inst().level(lvl);
#define logecho(echo) PlatformLogger::inst().echo(lvl);
#define logclear() PlatformLogger::inst().clear();

#endif //ARDUINO_LOGGER_H_
