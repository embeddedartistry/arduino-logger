#define NO_PRAGMA_MARK

#include "TeensySDRotationalLogger.h"

static TeensySDRotationalLogger Log;
static SdFs sd;
static int iterations = 0;

void setup()
{
  Serial.begin(115200);
  while(!Serial) delay(10); 
  // wait for Arduino Serial Monitor (native USB boards)
  
  sd.begin(SdioConfig(FIFO_SDIO));
  Log.begin(sd);
  Log.echo(true); // log calls will be printed over Serial
  Log.debug("Finished with setup(), first log statement to SD card\n");
}

void loop()
{
  // put your main code here, to run repeatedly:
  Log.info("Loop iteration %d\n", iterations);
  iterations++;

  if((iterations % 10) == 0)
  {
   	Log.flush();
  }

  delay(1000);
}
