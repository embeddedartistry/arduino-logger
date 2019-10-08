#include "platform_logger.h"

void setup() {
  Serial.begin(115200);
  logdebug("This is adding to the test log!\n");
}

void loop() {
  // put your main code here, to run repeatedly:
  PlatformLogger::inst().dump();
  delay(6000);
}
