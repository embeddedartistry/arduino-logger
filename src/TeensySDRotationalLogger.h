#ifndef SD_FILE_LOGGER_H_
#define SD_FILE_LOGGER_H_

#include "Arduino.h"
#include "ArduinoLogger.h"
#include "SdFat.h"
#include <EEPROM.h>
#include <internal/ring_span.hpp>
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

		set_filename();

		if(!file_.open(filename_, O_WRITE | O_CREAT))
		{
			errorHalt("Failed to open file");
		}

		// Clear current file contents
		file_.truncate(0);

		log_reset_reason();

		// Manually flush, since the file is open
		if(counter)
		{
			if(counter != file_.write(buffer_, counter))
			{
				errorHalt("Failed to write to log file");
			}

			counter = 0;
		}

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
	FsFile file_;

	char buffer_[BUFFER_SIZE] = {0};
	size_t counter = 0;
};

#endif // SD_FILE_LOGGER_H_
