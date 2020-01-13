#include <Homie.hpp>
//#include <LoggerNode.h>

#include <Wire.h>
#include <ESPHomieCCS811Node.h>
#include <SensorNode.h>
#include <LoggerNode.h>

#define FW_NAME "SimpleCCS811Node-HTU21D"
#define FW_MAJOR "0"
#define COMMIT_COUNTER "1"
#define BUILD_NUMBER "0"

#define FW_VERSION FW_MAJOR "." COMMIT_COUNTER "." BUILD_NUMBER


LoggerNode LN;

SensorNode sensor;
ESP_HomieCCS811Node ccs811;



void setup() {
  Homie_setFirmware(FW_NAME, FW_VERSION);
  Serial.begin(74880);  // Baudrate of ESP8266 Boot loader. Set it here to same value, so you can see both the boot loader's messages and your's.
  Serial.println(FW_NAME "-" FW_VERSION);
  Serial.flush();
  Homie.disableLedFeedback();
  Homie.disableResetTrigger();
  Homie.setLoggingPrinter(&Serial);

  Homie.setup();
  Wire.begin(SDA, SCL);

  sensor.setEnvDataCallback([](float t, float h, float p)->bool { ccs811.setEnvironmentalData(t, h);});

}


void loop() {
  Homie.loop();
}


