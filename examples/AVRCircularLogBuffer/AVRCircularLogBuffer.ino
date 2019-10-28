#include "platform_logger.h"
#include <Adafruit_SleepyDog.h>

static int iterations = 0;

void setup() {
  Serial.begin(115200);
  while(!Serial) delay(10); 
  // wait for Arduino Serial Monitor (native USB boards)

  // This call is optional. It will log the reboot reason (e.g. WDT reset, power-on reset)
  PlatformLogger::inst().resetCause();
  
  logdebug("This line is added to the log buffer from setup\n");
}

void loop() {
  // put your main code here, to run repeatedly:
  loginfo("Loop iteration %d\n", iterations);
  iterations++;

  if((iterations % 1) == 0)
  {
    printf("Log buffer contents:\n");
    logflush();
  }

  delay(1000);
}
