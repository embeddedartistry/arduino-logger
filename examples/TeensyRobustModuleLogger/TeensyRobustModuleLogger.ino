#define NO_PRAGMA_MARK

#include "TeensyRobustModuleLogger.h"

// NOTE that module APIs are not routed to the global instance manager,
// so you cannot use that class or the macros with this strategy.
// You can implement your own that forwards the appropriate APIs, however.

// For this strategy, you need two things:
// - A maximal number of modules (passed to the logging strategy as a template parameter
// - A mapping of modules-to-ids that you can use with the APIs
enum module {
  SETUP = 0,
  LOOP,
  MODULE_COUNT
};

const size_t EEPROM_LOG_STORAGE_ADDR = 1024;
const size_t EEPROM_LOG_STORAGE_SIZE = 512;
char eeprom_log_buffer_copy[EEPROM_LOG_STORAGE_SIZE];

// We need to initialize our logging strategy with the number of modules we have
static TeensyRobustModuleLogger<module::MODULE_COUNT> Log;
static SdFs sd;
static int iterations = 0;

/// Here is an example approach for reading the contents of the EEPROM log buffer
static void readEEPROMLogBuffer()
{
   for(size_t i = 0; i < EEPROM_LOG_STORAGE_SIZE; i++)
   {
    eeprom_log_buffer_copy[i] = EEPROM.read(EEPROM_LOG_STORAGE_ADDR + i);
   }

   if(eeprom_log_buffer_copy[0] == 0xff)
   {
    printf("EEPROM Log is Empty\n");
   }
   else
   {
    printf("--- EEPROM Log Contents ---\n%s\n", eeprom_log_buffer_copy);
    printf("--- End of EEPROM Log Contents ---\n");
   }
}

void setup()
{
  Serial.begin(115200);
  while(!Serial) delay(10);
  // wait for Arduino Serial Monitor (native USB boards)

  // We'll check the EEPROM log buffer contents on boot.
  readEEPROMLogBuffer();

  /// By default, we will try to initialize the SD card to store log files
  printf("Initializing the logger with the SD Card\n");
  bool r = sd.begin(SdioConfig(FIFO_SDIO));
  if(r)
  {
    /// If begin is called with the SD card, log data will be flushed to a file.
    Log.begin(sd);
  }
  else
  {
    printf("Failed to initialize the SD card. Falling back to EEPROM\n");
    // When using EEPROM storage, we recommend reducing the log levels!
    Log.level(log_level_e::error);
    Log.error(module::SETUP, "Could not initialize SD card\n");
    Log.begin(EEPROM_LOG_STORAGE_ADDR, EEPROM_LOG_STORAGE_SIZE);
    /// If you comment out the Log.begin() call above and use the one below instead,
    /// you will have a circular log buffer stored in memory and flushed to the serial console
    /// When using the circular log buffer, it is likely not necessary to reduce the log levels!
    // Log.begin();
  }

  // We'll set our modules to different level limits
  Log.level(module::SETUP, log_level_e::debug);
  Log.level(module::LOOP, log_level_e::info);
  Log.echo(true); // log calls will also be printed over Serial

  printf("Configured log levels are:\nSystem: %d, SETUP: %d, LOOP: %d\n",
    Log.level(), Log.level(module::SETUP), Log.level(module::LOOP));

  // For demonstration purposes, we'll turn off auto-flush behavior
  // This will trigger overflow conditions in the loop() due to our flush
  // iteration count - we wait too long between flushes.
  Log.auto_flush(false);


  // This call should be found in the SD log or circular buffer log because debug is the limit for SETUP
  Log.debug(module::SETUP, "Finished with setup(), first log statement to SD card\n");
}

void loop()
{
  // This call should be found in the log because the LOOP module limit is set to info
  Log.info(module::LOOP, "Loop iteration %d, present in all but EEPROM logging\n", iterations);
  // This is excluded from the log due to the LOOP module setting
  Log.debug(module::LOOP, "You should not see this in any logs!\n");
  iterations++;

  if(Log.has_overrun())
  {
    // Let's flush what we have so far
    Log.flush();
  }

  if((iterations % 10) == 0)
  {
   	Log.flush();
  }

  delay(1000);
}
