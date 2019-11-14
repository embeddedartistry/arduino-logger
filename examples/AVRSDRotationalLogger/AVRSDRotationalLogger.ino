#include <AVRSDRotationalLogger.h>

static int iterations = 0;
static SdFs sd;
static AVRSDRotationalLogger logger;

void setup() {
  Serial.begin(115200);
  while(!Serial) delay(10); 
  // wait for Arduino Serial Monitor (native USB boards)

  sd.begin(SdSpiConfig(4, DEDICATED_SPI));
  logger.echo(true); // log calls will be printed over Serial
  logger.begin(sd);
  
  logger.debug("Finished with setup(), first log statement to SD card\n");
}

void loop() {
  // put your main code here, to run repeatedly:
  logger.info("Loop iteration %d\n", iterations);
  iterations++;

  delay(1000);
}
