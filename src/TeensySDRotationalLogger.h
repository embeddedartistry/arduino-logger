#ifndef SD_FILE_LOGGER_H_
#define SD_FILE_LOGGER_H_

#include "Arduino.h"
#include "ArduinoLogger.h"
#include "SdFat.h"
#include "internal/circular_buffer.hpp"
#include <EEPROM.h>
#include <kinetis.h>

/** SD File Buffer
 *
 * Logs to a file on the SD card.
 *
 * This class uses the SdFat Arduino Library.
 *
 *	@code
 *	using PlatformLogger =
 *		PlatformLogger_t<TeensySDRotationalLogger>;
 *  @endcode
 *
 * @ingroup LoggingSubsystem
 */
class TeensySDRotationalLogger final : public LoggerBase
{
  private:
	static constexpr size_t BUFFER_SIZE = 512;
	static constexpr size_t FILENAME_SIZE = 32;
	static constexpr unsigned EEPROM_LOG_STORAGE_ADDR = 4095;

  public:
	/// Default constructor
	TeensySDRotationalLogger() : LoggerBase() {}

	/// Default destructor
	~TeensySDRotationalLogger() noexcept = default;

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

		set_filename();

		if(!file_.open(filename_, O_WRITE | O_CREAT))
		{
			errorHalt("Failed to open file");
		}

		// Clear current file contents
		file_.truncate(0);

		log_reset_reason();

		// Manually flush, since the file is open
		flush();

		file_.close();
	}

	// Resets the log file counter back to 1
	void resetFileCounter()
	{
		EEPROM.write(EEPROM_LOG_STORAGE_ADDR, 1);
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

	/// Checks the kinetis SoC's reset reason registers and logs them
	/// This should only be called during begin().
	void log_reset_reason()
	{
		auto srs0 = RCM_SRS0;
		auto srs1 = RCM_SRS1;

		// Clear the values
		RCM_SRS0 = 0;
		RCM_SRS1 = 0;

		if(srs0 & RCM_SRS0_LVD)
		{
			info("Low-voltage Detect Reset\n");
		}

		if(srs0 & RCM_SRS0_LOL)
		{
			info("Loss of Lock in PLL Reset\n");
		}

		if(srs0 & RCM_SRS0_LOC)
		{
			info("Loss of External Clock Reset\n");
		}

		if(srs0 & RCM_SRS0_WDOG)
		{
			info("Watchdog Reset\n");
		}

		if(srs0 & RCM_SRS0_PIN)
		{
			info("External Pin Reset\n");
		}

		if(srs0 & RCM_SRS0_POR)
		{
			info("Power-on Reset\n");
		}

		if(srs1 & RCM_SRS1_SACKERR)
		{
			info("Stop Mode Acknowledge Error Reset\n");
		}

		if(srs1 & RCM_SRS1_MDM_AP)
		{
			info("MDM-AP Reset\n");
		}

		if(srs1 & RCM_SRS1_SW)
		{
			info("Software Reset\n");
		}

		if(srs1 & RCM_SRS1_LOCKUP)
		{
			info("Core Lockup Event Reset\n");
		}
	}

	void set_filename()
	{
		uint8_t value = EEPROM.read(EEPROM_LOG_STORAGE_ADDR);

		// 0xFF indicates a byte that's been reset, or value 255. Either way, reset to 0.
		if(value == 0xFF)
		{
			value = 1;
		}

		snprintf(filename_, FILENAME_SIZE, "log_%d.txt", value);

		EEPROM.write(EEPROM_LOG_STORAGE_ADDR, value + 1);
	}

  private:
	SdFs* fs_;
	char filename_[FILENAME_SIZE];
	mutable FsFile file_;

	CircularBuffer<char, BUFFER_SIZE> log_buffer_;
};

#endif // SD_FILE_LOGGER_H_
