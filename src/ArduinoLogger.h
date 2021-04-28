#ifndef ARDUINO_LOGGER_H_
#define ARDUINO_LOGGER_H_

#include <LibPrintf.h>
#if !defined(__AVR__)
#include <utility>
#endif

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

#ifndef LOG_AUTOFLUSH_DEFAULT
/// Whether the log should auto-flush by default on boot
#define LOG_AUTOFLUSH_DEFAULT true
#endif

#ifndef LOG_ECHO_EN_DEFAULT
/// Indicates that log statements should be echoed to the console
/// If true, log statements will be echoed.
/// If false, log statements will only go to the log.
#define LOG_ECHO_EN_DEFAULT false
#endif

#ifndef LOG_LEVEL_NAMES
/// Users can override these default names with a compiler definition
#define LOG_LEVEL_NAMES                                         \
	{                                                           \
		"off", "critical", "error", "warning", "info", "debug", \
	}
#endif

#ifndef LOG_LEVEL_SHORT_NAMES
/// Users can override these default short names with a compiler definition
#define LOG_LEVEL_SHORT_NAMES                                                             \
	{                                                                                     \
		"O", LOG_LEVEL_CRITICAL_PREFIX, LOG_LEVEL_ERROR_PREFIX, LOG_LEVEL_WARNING_PREFIX, \
			LOG_LEVEL_INFO_PREFIX, LOG_LEVEL_DEBUG_PREFIX,                                \
	}
#endif

#ifndef NO_PRAGMA_MARK
#pragma mark - Short File Name Macro -
#endif

using cstr = const char* const;

constexpr cstr past_last_slash(cstr str, cstr last_slash)
{
	return *str == '\0'	 ? last_slash
		   : *str == '/' ? past_last_slash(str + 1, str + 1)
						 : past_last_slash(str + 1, last_slash);
}

constexpr cstr past_last_slash(cstr str)
{
	return past_last_slash(str, str);
}

#define __SHORT_FILE__                                  \
	({                                                  \
		constexpr cstr sf__{past_last_slash(__FILE__)}; \
		sf__;                                           \
	})

#ifndef NO_PRAGMA_MARK
#pragma mark - Tracing Macros -
#endif

#define STRINGPASTE(x) #x
#define TOSTRING(x) STRINGPASTE(x)
#define TRACE()                                                                \
	({                                                                         \
		constexpr cstr sf__{past_last_slash(__FILE__ ":" TOSTRING(__LINE__))}; \
		sf__;                                                                  \
	})

#define FUNC() __FUNCTION__
#define PRETTY_FUNC() __PRETTY_FUNCTION__

#ifndef NO_PRAGMA_MARK
#pragma mark - Logging Class -
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

constexpr log_level_e LOG_LEVEL_LIMIT() noexcept
{
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
	 *	The base class returns SIZE_MAX to indicate a potentially invalid condition.
	 */
	virtual size_t size() const noexcept
	{
		return SIZE_MAX;
	}

	/** Get the log buffer capacity
	 *
	 * Derived classes must implement this function.
	 *
	 * @returns The total capacity of the log buffer, in bytes.
	 *	The base class returns SIZE_MAX to indicate a potentially invalid condition.
	 */
	virtual size_t capacity() const noexcept
	{
		return SIZE_MAX;
	}

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
	 * @returns The prior value that was configured before the call was made, which allows
	 *	users to restore the previous setting if necessary.
	 */
	bool echo(bool en) noexcept
	{
		bool setting = echo_;
		echo_ = en;
		return setting;
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

	/** Enable or disable the auto-flush behavior
	 *
	 * If enabled, the log() call will flush the contents of the buffer whenever
	 * a write is attempted while the buffer is full. If disabled, this will
	 * not occur. Instead, the user must manually call `flush()` to
	 *
	 * @param enabled True enables auto-flush behavior in log(), false disables it.
	 *	When disabled, the user must manually flush() the log buffer contents.
	 * @returns The auto-flush setting in place prior to this call. This allows
	 * 	you to capture the return value, perform some actions, and restore the value
	 *	once you have completed your work.
	 *	@code
	 *	// Disable auto-flush
	 *	bool setting = log.auto_flush(false);
	 *	log.warning(...);
	 *	// Restore previous setting
	 *	log.auto_flush(setting);
	 *	@endcode
	 */
	bool auto_flush(bool enabled)
	{
		bool prior = auto_flush_;
		auto_flush_ = enabled;
		return prior;
	}

	/** Check whether auto-flush is enabled.
	 *
	 * @returns the current auto-flush behavior setting
	 */
	bool auto_flush()
	{
		return auto_flush_;
	}

	/** Check for a buffer overrun condition.
	 *
	 * @returns a boolean indicating whether or not an overrun condition has occured
	 *	since the last time flush() was called. If `true`, this indicates that data
	 * 	has been lost from the log buffer.
	 */
	bool has_overrun()
	{
		return overrun_occurred_;
	}

	template<typename... Args>
	void critical(const char* fmt, const Args&... args)
	{
#if defined(__AVR__)
		log(log_level_e::critical, fmt, args...);
#else
		log(log_level_e::critical, fmt, std::forward<const Args>(args)...);
#endif
	}

	template<typename... Args>
	void critical_interrupt(const char* fmt, const Args&... args)
	{
#if defined(__AVR__)
		log_interrupt(log_level_e::critical, fmt, args...);
#else
		log_interrupt(log_level_e::critical, fmt, std::forward<const Args>(args)...);
#endif
	}

	template<typename... Args>
	void error(const char* fmt, const Args&... args)
	{
#if defined(__AVR__)
		log(log_level_e::error, fmt, args...);
#else
		log(log_level_e::error, fmt, std::forward<const Args>(args)...);
#endif
	}

	template<typename... Args>
	void error_interrupt(const char* fmt, const Args&... args)
	{
#if defined(__AVR__)
		log_interrupt(log_level_e::error, fmt, args...);
#else
		log_interrupt(log_level_e::error, fmt, std::forward<const Args>(args)...);
#endif
	}

	template<typename... Args>
	void warning(const char* fmt, const Args&... args)
	{
#if defined(__AVR__)
		log(log_level_e::warning, fmt, args...);
#else
		log(log_level_e::warning, fmt, std::forward<const Args>(args)...);
#endif
	}

	template<typename... Args>
	void warning_interrupt(const char* fmt, const Args&... args)
	{
#if defined(__AVR__)
		log_interrupt(log_level_e::warning, fmt, args...);
#else
		log_interrupt(log_level_e::warning, fmt, std::forward<const Args>(args)...);
#endif
	}

	template<typename... Args>
	void info(const char* fmt, const Args&... args)
	{
#if defined(__AVR__)
		log(log_level_e::info, fmt, args...);
#else
		log(log_level_e::info, fmt, std::forward<const Args>(args)...);
#endif
	}

	template<typename... Args>
	void info_interrupt(const char* fmt, const Args&... args)
	{
#if defined(__AVR__)
		log_interrupt(log_level_e::info, fmt, args...);
#else
		log_interrupt(log_level_e::info, fmt, std::forward<const Args>(args)...);
#endif
	}

	template<typename... Args>
	void debug(const char* fmt, const Args&... args)
	{
#if defined(__AVR__)
		log(log_level_e::debug, fmt, args...);
#else
		log(log_level_e::debug, fmt, std::forward<const Args>(args)...);
#endif
	}

	template<typename... Args>
	void debug_interrupt(const char* fmt, const Args&... args)
	{
#if defined(__AVR__)
		log_interrupt(log_level_e::debug, fmt, args...);
#else
		log_interrupt(log_level_e::debug, fmt, std::forward<const Args>(args)...);
#endif
	}

	/// Prints directly to the log with no extra characters added to the message.
	template<typename... Args>
	void print(const Args&... args) noexcept
	{
		fctprintf(&LoggerBase::log_add_char_to_buffer_bounce, this, args...);

		if(echo_)
		{
			// cppcheck-suppress wrongPrintfScanfArgNum
			printf(args...);
		}
	}

	/** Add data to the log buffer from an interrupt context
	 *
	 * This call will disable auto-echo and auto-flush behavior for the duration
	 * of this function call.
	 *
	 * @tparam Args Variadic template args. Will be deduced by the compiler. Enables support for
	 *	a variadic function template.
	 * @param l The log level associated with this statement.
	 * @param fmt The log format string.
	 * @param args The variadic arguments that are associated with the format string.
	 */
	template<typename... Args>
	void log_interrupt(log_level_e l, const char* fmt, const Args&... args) noexcept
	{
		if(enabled_ && l <= level_)
		{
			bool flush_setting = auto_flush(false);
			bool echo_setting = echo(false);

			// Add our prefix
			print("%s", LOG_LEVEL_TO_SHORT_C_STRING(l));

			log_customprefix();

			// Send the primary log statement
			print(fmt, args...);

			// Restore prior settings
			auto_flush(flush_setting);
			echo(echo_setting);
		}
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
			// Add our prefix
			print("%s", LOG_LEVEL_TO_SHORT_C_STRING(l));

			log_customprefix();

			// Send the primary log statement
			print(fmt, args...);
		}
	}

	/// Flush the buffered log contents to the target output stream
	/// Wrapper for flush_ that resets the overrun_occurred_ flag
	/// Can be overridden if desired
	virtual void flush() noexcept
	{
		if(internal_size() > 0)
		{
			flush_();
			if(overrun_occurred_)
			{
				critical("---Log buffer overrun detected---\n");
				flush_();
			}
			overrun_occurred_ = false;
		}
	}

	/// Clear the contents of the log buffer
	/// Wrapper for clear_ that resets the overrun_occurred_ flag
	/// Can be overridden if desired
	virtual void clear() noexcept
	{
		overrun_occurred_ = false;
		clear_();
	}

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

	/** Flush the buffered log contents to the target output stream
	 *
	 * When called, the contents of the internal log buffer will be removed and placed into
	 * the target output stream.
	 *
	 * Outputs can be any target. You will notice many example implementations print
	 * the log buffer contents to Serial when flush() is called.
	 *
	 * Derived classes must implement this function.
	 */
	virtual void flush_() noexcept {}

	/** Clear the contents of the log buffer.
	 *
	 * Reset the log buffer to an empty state.
	 *
	 * @post The log buffer will be empty.
	 */
	virtual void clear_() noexcept {}

	/** Add a custom prefix to the log file
	 *
	 * Define this function in your derived class to supply a custom prefix
	 * to the log statement. This will appear *after* the log level indicator
	 * (e.g., <!>), but before the message.
	 *
	 * Recommended use of this field might include generating a timestamp:
	 *	<!> [0081838ms] Message goes here.
	 *
	 * Derived classes must implement this function.
	 */
	virtual void log_customprefix() {}

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

	/** Helper function for logging to the buffer.
	 *
	 * If auto-flushing is enabled, we check whether the RAM buffer storage
	 * is full and then call flush(). Otherwise, we forward the character to
	 * the logging strategy's log_putc() implementation.
	 *
	 * Deived classes may override this function if desired.
	 *
	 * @param c The character to insert into the log buffer.
	 */
	virtual void log_add_char_to_buffer(char c)
	{
		if(internal_size() == internal_capacity())
		{
			if(auto_flush())
			{
				flush();
			}
			else
			{
				overrun_occurred_ = true;
			}
		}

		log_putc(c);
	}

	/** putc bounce function
	 *
	 * This is a bounce function which registers with the C printf API. We use the private parameter
	 * to store the `this` pointer so we can get back to our logger's add_char_to_buffer function.
	 *
	 * @param c The character to log.
	 * @param this_ptr The this pointer of the logger instance. Used to invoke log_putc() on the
	 * correct instance.
	 */
	static void log_add_char_to_buffer_bounce(char c, void* this_ptr)
	{
		reinterpret_cast<LoggerBase*>(this_ptr)->log_add_char_to_buffer(c);
	}

	/** Get the current size of the log buffer internal storage.
	 *
	 * This function returns the size of the internal storage, usually a buffer
	 * or circular buffer in RAM. This buffer size is required for controlling
	 * auto-flush behavior.
	 *
	 * For example, you might store log files on an SD card, but have an
	 * internal memory representation as well. The SD file size is likely
	 * much larger than the internal buffer size, so this difference is important.
	 *
	 * @returns The current size of the internal staging log buffer, in bytes.
	 *	By default, this returns size(). Override if necessary.
	 */
	virtual size_t internal_size() const noexcept
	{
		return size();
	}

	/** Get the capacity of the log buffer internal storage.
	 *
	 * This function returns the capacity of the internal storage, usually a buffer
	 * or circular buffer in RAM. This buffer size is required for controlling
	 * auto-flush behavior.
	 *
	 * For example, you might store log files on an SD card, but have an
	 * internal memory representation as well. The SD file capacity is likely
	 * much larger than the internal buffer capacity, so this difference is important.
	 *
	 * @returns The current capacity of the internal staging log buffer, in bytes.
	 *	By default, this returns capacity(). Override if necessary.
	 */
	virtual size_t internal_capacity() const noexcept
	{
		return capacity();
	}

  private:
	/// Indicates whether logging is currently enabled
	bool enabled_ = LOG_EN_DEFAULT;

	/// Indicates whether the library will auto-flush during log() if the buffer
	/// is full. If disabled, the user is responsible for coordinating flush() calls.
	bool auto_flush_ = LOG_AUTOFLUSH_DEFAULT;

	/// Indicates whether an overrun has occurred between flush() calls. If this is `true`,
	/// then you know data has been lost.
	bool overrun_occurred_ = false;

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
#if defined(__AVR__)
		inst().critical(fmt, args...);
#else
		inst().critical(fmt, std::forward<const Args>(args)...);
#endif
	}

	template<typename... Args>
	inline static void error(const char* fmt, const Args&... args)
	{
#if defined(__AVR__)
		inst().error(fmt, args...);
#else
		inst().error(fmt, std::forward<const Args>(args)...);
#endif
	}

	template<typename... Args>
	inline static void warning(const char* fmt, const Args&... args)
	{
#if defined(__AVR__)
		inst().warning(fmt, args...);
#else
		inst().warning(fmt, std::forward<const Args>(args)...);
#endif
	}

	template<typename... Args>
	inline static void info(const char* fmt, const Args&... args)
	{
#if defined(__AVR__)
		inst().info(fmt, args...);
#else
		inst().info(fmt, std::forward<const Args>(args)...);
#endif
	}

	template<typename... Args>
	inline static void debug(const char* fmt, const Args&... args)
	{
#if defined(__AVR__)
		inst().debug(fmt, args...);
#else
		inst().debug(fmt, std::forward<const Args>(args)...);
#endif
	}

	template<typename... Args>
	inline static void critical_interrupt(const char* fmt, const Args&... args)
	{
#if defined(__AVR__)
		inst().critical_interrupt(fmt, args...);
#else
		inst().critical_interrupt(fmt, std::forward<const Args>(args)...);
#endif
	}

	template<typename... Args>
	inline static void error_interrupt(const char* fmt, const Args&... args)
	{
#if defined(__AVR__)
		inst().error_interrupt(fmt, args...);
#else
		inst().error_interrupt(fmt, std::forward<const Args>(args)...);
#endif
	}

	template<typename... Args>
	inline static void warning_interrupt(const char* fmt, const Args&... args)
	{
#if defined(__AVR__)
		inst().warning_interrupt(fmt, args...);
#else
		inst().warning_interrupt(fmt, std::forward<const Args>(args)...);
#endif
	}

	template<typename... Args>
	inline static void info_interrupt(const char* fmt, const Args&... args)
	{
#if defined(__AVR__)
		inst().info_interrupt(fmt, args...);
#else
		inst().info_interrupt(fmt, std::forward<const Args>(args)...);
#endif
	}

	template<typename... Args>
	inline static void debug_interrupt(const char* fmt, const Args&... args)
	{
#if defined(__AVR__)
		inst().debug_interrupt(fmt, args...);
#else
		inst().debug_interrupt(fmt, std::forward<const Args>(args)...);
#endif
	}

	template<typename... Args>
	inline static void print(const char* fmt, const Args&... args)
	{
#if defined(__AVR__)
		inst().print(fmt, args...);
#else
		inst().print(fmt, std::forward<const Args>(args)...);
#endif
	}

	inline static void flush()
	{
		inst().flush();
	}

	inline static void clear()
	{
		inst().clear();
	}

	inline static log_level_e level(log_level_e l)
	{
		return inst().level(l);
	}

	inline static bool echo(bool en)
	{
		return inst().echo(en);
	}

	inline static bool auto_flush(bool enabled)
	{
		return inst().auto_flush(enabled);
	}

	inline static bool has_overrun()
	{
		return inst().has_overrun();
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
#define logcritical(...) PlatformLogger::critical(__VA_ARGS__)
#endif
#else
#define logcritical(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_ERROR
#ifndef logerror
#define logerror(...) PlatformLogger::error(__VA_ARGS__)
#endif
#else
#define logerror(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_WARNING
#ifndef logwarning
#define logwarning(...) PlatformLogger::warning(__VA_ARGS__)
#endif
#else
#define logwarning(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_INFO
#ifndef loginfo
#define loginfo(...) PlatformLogger::info(__VA_ARGS__)
#endif
#else
#define loginfo(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_DEBUG
#ifndef logdebug
#define logdebug(...) PlatformLogger::debug(__VA_ARGS__)
#endif
#else
#define logdebug(...)
#endif

#define logflush() PlatformLogger::flush();
#define loglevel(lvl) PlatformLogger::level(lvl);
#define logecho(echo) PlatformLogger::echo(lvl);
#define logclear() PlatformLogger::clear();

#endif // ARDUINO_LOGGER_H_
