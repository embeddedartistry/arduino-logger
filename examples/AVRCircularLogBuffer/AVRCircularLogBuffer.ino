#include "platform_logger.h"

static int iterations = 0;

void setup() {
  Serial.begin(115200);
  logdebug("This line is added to the log buffer from setup\n");
}

void loop() {
  // put your main code here, to run repeatedly:
  loginfo("Loop iteration %d\n", iterations);
  iterations++;

  if((iterations % 10) == 0)
  {
    printf("Log buffer contents:\n");
    logflush();
  }

  delay(1000);
}
