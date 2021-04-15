#ifndef CIRCULAR_BUFFER_LOGGER_H_
#define CIRCULAR_BUFFER_LOGGER_H_

// By default, this logging strategy does not auto-flush
// You can still override this default setting if desired.
#ifndef LOG_AUTOFLUSH_DEFAULT
#define LOG_AUTOFLUSH_DEFAULT false
#endif

#include "ArduinoLogger.h"
#include "internal/circular_buffer.hpp"

/** Circular log buffer
 *
 * A circular log buffer manages a memoroy buffer as if it were neverending. When the capacity
 * of the buffer is reached, insertions will wrap around to the beginning of the buffer and
 * overwrite old data. This enables seemingly "infinite" memory with a fixed capacity, preferring
 * the newest data be kept.
 *
 * @tparam TBufferSize Defines the size of the circular log buffer.
 * Set to 0 to disable logging completely (for memory constrained systems).
 * @note Size requirement: power-of-2 for optimized queue logic.
 *
 *	@code
 *	using PlatformLogger =
 *		PlatformLogger_t<CircularLogBufferLogger<8 * 1024>>;
 *  @endcode
 *
 * @ingroup LoggingSubsystem
 */
template<size_t TBufferSize = (1 * 1024)>
class CircularLogBufferLogger final : public LoggerBase
{
  public:
	/// Default constructor
	CircularLogBufferLogger() : LoggerBase() {}

	/** Initialize the circular log buffer with options
	 *
	 * @param enable If true, log statements will be output to the log buffer. If false,
	 * logging will be disabled and log statements will not be output to the log buffer.
	 * @param l Runtime log filtering level. Levels greater than the target will not be output
	 * to the log buffer.
	 * @param echo If true, log statements will be logged and printed to the console with printf().
	 * If false, log statements will only be added to the log buffer.
	 */
	explicit CircularLogBufferLogger(bool enable, log_level_e l = LOG_LEVEL_LIMIT(),
									 bool echo = LOG_ECHO_EN_DEFAULT) noexcept
		: LoggerBase(enable, l, echo)
	{
	}

	/// Default destructor
	~CircularLogBufferLogger() noexcept = default;

	size_t size() const noexcept final
	{
		return log_buffer_.size();
	}

	size_t capacity() const noexcept final
	{
		return log_buffer_.capacity();
	}

  protected:
	void log_putc(char c) noexcept final
	{
		log_buffer_.put(c);
	}

	void flush_() noexcept final
	{
		while(!log_buffer_.empty())
		{
			_putchar(log_buffer_.get());
		}
	}

	void clear_() noexcept final
	{
		log_buffer_.reset();
	}

  private:
	CircularBuffer<char, TBufferSize> log_buffer_;
};

#endif // CIRCULAR_BUFFER_LOGGER_H_
