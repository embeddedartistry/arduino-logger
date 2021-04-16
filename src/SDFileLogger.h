#ifndef SD_FILE_LOGGER_H_
#define SD_FILE_LOGGER_H_

#include "Arduino.h"
#include "ArduinoLogger.h"
#include "SdFat.h"
#include "internal/circular_buffer.hpp"

/** SD File Buffer
 *
 * Logs to a file on the SD card.
 *
 * This class uses the SdFat Arduino Library.
 *
 *	@code
 *	using PlatformLogger =
 *		PlatformLogger_t<SDFileLogger>;
 *  @endcode
 *
 * @ingroup LoggingSubsystem
 */
class SDFileLogger final : public LoggerBase
{
  private:
	static constexpr size_t BUFFER_SIZE = 512;

  public:
	/// Default constructor
	SDFileLogger() : LoggerBase() {}

	/// Default destructor
	~SDFileLogger() noexcept = default;

	size_t size() const noexcept final
	{
		return file_.size();
	}

	size_t capacity() const noexcept final
	{
		// size in blocks * bytes per block (512 Bytes = 2^9)
		return fs_ ? fs_->card()->sectorCount() << 9 : 0;
	}

	void log_customprefix() noexcept final
	{
		print("[%d ms] ", millis());
	}

	void begin(SdFs& sd_inst)
	{
		fs_ = &sd_inst;

		if(!file_.open(filename_, O_WRITE | O_CREAT))
		{
			errorHalt("Failed to open file");
		}

		// Clear current file contents
		file_.truncate(0);

		// Flush the buffer since the file is open
		flush();

		file_.close();
	}

  protected:
	void log_putc(char c) noexcept final
	{
		log_buffer_.put(c);
	}

	size_t internal_size() const noexcept override
	{
		return log_buffer_.size();
	}

	size_t internal_capacity() const noexcept override
	{
		return log_buffer_.capacity();
	}

	void flush_() noexcept final
	{
		writeBufferToSDFile();
	}

	void clear_() noexcept final
	{
		log_buffer_.reset();
	}

  private:
	void errorHalt(const char* msg)
	{
		printf("Error: %s\n", msg);
		if(fs_->sdErrorCode())
		{
			if(fs_->sdErrorCode() == SD_CARD_ERROR_ACMD41)
			{
				printf("Try power cycling the SD card.\n");
			}
			printSdErrorSymbol(&Serial, fs_->sdErrorCode());
			printf(", ErrorData: 0x%x\n", fs_->sdErrorData());
		}
		while(true)
		{
		}
	}

	void writeBufferToSDFile()
	{
		if(!file_.open(filename_, O_WRITE | O_APPEND))
		{
			errorHalt("Failed to open file");
		}

		int bytes_written = 0;

		// We need to get the front, the rear, and potentially write the files in two steps
		// to prevent ordering problems
		size_t head = log_buffer_.head();
		size_t tail = log_buffer_.tail();
		const char* buffer = log_buffer_.storage();

		if((head < tail) || ((tail > 0) && (log_buffer_.size() == log_buffer_.capacity())))
		{
			// we have a wraparound case
			// We will write from buffer[tail] to buffer[size] in one go
			// Then we'll reset head to 0 so that we can write 0 to tail next
			bytes_written = file_.write(&buffer[tail], log_buffer_.capacity() - tail);
			bytes_written += file_.write(buffer, head);
		}
		else
		{
			// Write from tail position and send the specified number of bytes
			bytes_written = file_.write(&buffer[tail], log_buffer_.size());
		}

		if(static_cast<size_t>(bytes_written) != log_buffer_.size())
		{
			errorHalt("Failed to write to log file");
		}

		log_buffer_.reset();

		file_.close();
	}

  private:
	SdFs* fs_;
	const char* filename_ = "log.txt";
	mutable FsFile file_;

	CircularBuffer<char, BUFFER_SIZE> log_buffer_;
};

#endif // SD_FILE_LOGGER_H_
