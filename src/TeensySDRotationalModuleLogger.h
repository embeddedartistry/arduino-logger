#ifndef SD_FILE_LOGGER_H_
#define SD_FILE_LOGGER_H_

#include "Arduino.h"
#include "ArduinoLogger.h"
#include "SdFat.h"
#include "internal/circular_buffer.hpp"
#include <EEPROM.h>
#include <kinetis.h>

/** Rotational SD File Buffer with per-Module Log Levels
 *
 * Logs to a file on the SD card using a rotation strategy.
 * This class also provides per-module log levels, allowing you to
 * specify different level limits or different sections of code.
 *
 * This class uses the SdFat Arduino Library.
 *
 * NOTE that module APIs are not routed to the global instance manager,
 * so you cannot use that class or the macros with this strategy.
 * You can implement your own that forwards the appropriate APIs, however.
 *
 *	@code
 *	using PlatformLogger =
 *		PlatformLogger_t<TeensySDRotationalLogger>;
 *  @endcode
 *
 * @tparam TModuleCount The maximum number of modules you want to support
 * 	with this logging strategy.
 *
 * @ingroup LoggingSubsystem
 */
template<size_t TModuleCount = 1>
class TeensySDRotationalModuleLogger final : public LoggerBase
{
  private:
	static constexpr size_t BUFFER_SIZE = 512;
	static constexpr size_t FILENAME_SIZE = 32;
	static constexpr unsigned EEPROM_LOG_STORAGE_ADDR = 4095;

  public:
	/// Default constructor
	TeensySDRotationalModuleLogger() : LoggerBase() {}

	/// Default destructor
	~TeensySDRotationalModuleLogger() noexcept = default;

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

	/** Get the maximum log level (filtering) for the specified module
	 *
	 * @param module_id The ID for the corresponding module
	 * @returns the current log level maximum.
	 */
	log_level_e level(unsigned module_id) const noexcept
	{
		return module_levels_[module_id];
	}

	/** Set the maximum log level (filtering) for the specified module
	 *
	 * @param module_id The ID for the corresponding module
	 * @param l The maximum log level. Levels greater than `l` will not be added to the log buffer.
	 * @returns the current log level maximum.
	 */
	log_level_e level(unsigned module_id, log_level_e l) noexcept
	{
		if(l <= LOG_LEVEL_LIMIT())
		{
			module_levels_[module_id] = l;
		}

		return module_levels_[module_id];
	}

	/// The following overrides should be used to log with module IDs

	template<typename... Args>
	void critical(unsigned module_id, const char* fmt, const Args&... args)
	{
		if(module_levels_[module_id] >= log_level_e::critical)
		{
			log(log_level_e::critical, fmt, std::forward<const Args>(args)...);
		}
	}

	template<typename... Args>
	void critical_interrupt(unsigned module_id, const char* fmt, const Args&... args)
	{
		if(module_levels_[module_id] >= log_level_e::critical)
		{
			log_interrupt(log_level_e::critical, fmt, std::forward<const Args>(args)...);
		}
	}

	template<typename... Args>
	void error(unsigned module_id, const char* fmt, const Args&... args)
	{
		if(module_levels_[module_id] >= log_level_e::error)
		{
			log(log_level_e::error, fmt, std::forward<const Args>(args)...);
		}
	}

	template<typename... Args>
	void error_interrupt(unsigned module_id, const char* fmt, const Args&... args)
	{
		if(module_levels_[module_id] >= log_level_e::error)
		{
			log_interrupt(log_level_e::error, fmt, std::forward<const Args>(args)...);
		}
	}

	template<typename... Args>
	void warning(unsigned module_id, const char* fmt, const Args&... args)
	{
		if(module_levels_[module_id] >= log_level_e::warning)
		{
			log(log_level_e::warning, fmt, std::forward<const Args>(args)...);
		}
	}

	template<typename... Args>
	void warning_interrupt(unsigned module_id, const char* fmt, const Args&... args)
	{
		if(module_levels_[module_id] >= log_level_e::warning)
		{
			log_interrupt(log_level_e::warning, fmt, std::forward<const Args>(args)...);
		}
	}

	template<typename... Args>
	void info(unsigned module_id, const char* fmt, const Args&... args)
	{
		if(module_levels_[module_id] >= log_level_e::info)
		{
			log(log_level_e::info, fmt, std::forward<const Args>(args)...);
		}
	}

	template<typename... Args>
	void info_interrupt(unsigned module_id, const char* fmt, const Args&... args)
	{
		if(module_levels_[module_id] >= log_level_e::info)
		{
			log_interrupt(log_level_e::info, fmt, std::forward<const Args>(args)...);
		}
	}

	template<typename... Args>
	void debug(unsigned module_id, const char* fmt, const Args&... args)
	{
		if(module_levels_[module_id] >= log_level_e::debug)
		{
			log(log_level_e::debug, fmt, std::forward<const Args>(args)...);
		}
	}

	template<typename... Args>
	void debug_interrupt(unsigned module_id, const char* fmt, const Args&... args)
	{
		if(module_levels_[module_id] >= log_level_e::debug)
		{
			log_interrupt(log_level_e::debug, fmt, std::forward<const Args>(args)...);
		}
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

		if(head > tail)
		{
			// we have a wraparound case
			// We will write from buffer[head] to buffer[size] in one go
			// Then we'll reset head to 0 so that we can write 0 to tail next
			bytes_written = file_.write(&buffer[head], log_buffer_.capacity());
		}

		bytes_written += file_.write(buffer, tail);

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
			LoggerBase::info("Low-voltage Detect Reset\n");
		}

		if(srs0 & RCM_SRS0_LOL)
		{
			LoggerBase::info("Loss of Lock in PLL Reset\n");
		}

		if(srs0 & RCM_SRS0_LOC)
		{
			LoggerBase::info("Loss of External Clock Reset\n");
		}

		if(srs0 & RCM_SRS0_WDOG)
		{
			LoggerBase::info("Watchdog Reset\n");
		}

		if(srs0 & RCM_SRS0_PIN)
		{
			LoggerBase::info("External Pin Reset\n");
		}

		if(srs0 & RCM_SRS0_POR)
		{
			LoggerBase::info("Power-on Reset\n");
		}

		if(srs1 & RCM_SRS1_SACKERR)
		{
			LoggerBase::info("Stop Mode Acknowledge Error Reset\n");
		}

		if(srs1 & RCM_SRS1_MDM_AP)
		{
			LoggerBase::info("MDM-AP Reset\n");
		}

		if(srs1 & RCM_SRS1_SW)
		{
			LoggerBase::info("Software Reset\n");
		}

		if(srs1 & RCM_SRS1_LOCKUP)
		{
			LoggerBase::info("Core Lockup Event Reset\n");
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

	log_level_e module_levels_[TModuleCount] = {log_level_e(LOG_LEVEL)};

	CircularBuffer<char, BUFFER_SIZE> log_buffer_;
};

#endif // SD_FILE_LOGGER_H_
