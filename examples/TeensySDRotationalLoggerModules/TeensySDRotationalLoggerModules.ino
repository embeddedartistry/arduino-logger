#define NO_PRAGMA_MARK

#include "TeensySDRotationalModuleLogger.h"

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

// We need to initialize our logging strategy with the number of modules we have
static TeensySDRotationalModuleLogger<module::MODULE_COUNT> Log;
static SdFs sd;
static int iterations = 0;

void setup()
{
  Serial.begin(115200);
  while(!Serial) delay(10); 
  // wait for Arduino Serial Monitor (native USB boards)
  
  sd.begin(SdioConfig(FIFO_SDIO));
  Log.begin(sd);

  // We'll set our  modules to different levels
  Log.level(module::SETUP, log_level_e::debug);
  Log.level(module::LOOP, log_level_e::info);
  
  Log.echo(true); // log calls will be printed over Serial

  // This call should be found in the log because debug is the limit for SETUP
  Log.debug(module::SETUP, "Finished with setup(), first log statement to SD card\n");
}

void loop()
{
  // This call should be found in the log because the LOOP module limit is set to info
  Log.info(module::LOOP, "Loop iteration %d\n", iterations);
  // This is excluded from the log due to the LOOP module setting
  Log.debug(module::LOOP, "You should not see this!\n");
  iterations++;

  if((iterations % 10) == 0)
  {
   	Log.flush();
  }

  delay(1000);
}
