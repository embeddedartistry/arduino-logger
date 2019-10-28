#ifndef SD_FILE_LOGGER_H_
#define SD_FILE_LOGGER_H_

#include "Arduino.h"
#include "ArduinoLogger.h"
#include "SdFat.h"
#include <internal/ring_span.hpp>

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

	void flush() noexcept final
	{
		if(!file_.open(filename_, O_WRITE | O_APPEND))
		{
			errorHalt("Failed to open file");
		}

		if(counter != file_.write(buffer_, counter))
		{
			errorHalt("Failed to write to log file");
		}

		counter = 0;

		file_.close();
	}

	void clear() noexcept final
	{
		counter = 0;
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

		file_.close();
	}

  protected:
	void log_putc(char c) noexcept final
	{
		buffer_[counter] = c;
		counter++;

		if(counter == BUFFER_SIZE)
		{
			flush();
		}
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

  private:
	SdFs* fs_;
	const char* filename_ = "log.txt";
	FsFile file_;

	char buffer_[BUFFER_SIZE] = {0};
	size_t counter = 0;
};

#endif // SD_FILE_LOGGER_H_
