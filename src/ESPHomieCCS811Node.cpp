/*
 * ESPHomieCCS811Node.cpp
 *
 *  Created on: 01.05.2019
 *      Author: ian
 */

#include <ESPHomieCCS811Node.h>

ESP_HomieCCS811Node::ESP_HomieCCS811Node():
HomieNode("ccs811", "CCS881 Sensor", "sensor_co2_vcoc"),
sensor(0x5B), curTemp(0), curTVOC(0), curCO2(0)
{
	advertise("temperature").setName("Temperatur").setDatatype("float").setUnit("°C");
	advertise("co2").setName("CO2").setDatatype("float").setUnit("ppm");
	advertise("tvoc").setName("Org. fl. Gase").setDatatype("float").setUnit("ppm");
}

void ESP_HomieCCS811Node::setup() {
	  delay(100);
	  Wire.begin(SDA, SCL);
	  delay(100);
	  CCS811Core::status returnCode = sensor.begin();
	  if (returnCode != CCS811Core::SENSOR_SUCCESS)
	  {
	    Serial.printf("CCS811::begin() returned with error %x.\n", returnCode);
	  }
}

void ESP_HomieCCS811Node::loop() {
	static uint32_t nextRead = 0;
	uint32_t now = millis();
	if (now > nextRead && sensor.dataAvailable()) {
		nextRead = now + 12000; //60000;
		//If so, have the sensor read and calculate the results.
		//Get them later
		sensor.readAlgorithmResults();

		curTemp = sensor.getTemperature();
		curCO2 = sensor.getCO2();
		curTVOC =  sensor.getTVOC();

		Serial.print("CO2[");
		//Returns calculated CO2 reading
		Serial.print(curCO2);
		Serial.print("] tVOC[");
		//Returns calculated TVOC reading
		Serial.print(curTVOC);
		Serial.print("] millis[");
		//Simply the time since program start
		Serial.print(millis());
		Serial.print("]");
		Serial.println();

		setProperty("temperature").send(String(curTemp));
		setProperty("tvoc").send(String(curTVOC));
		setProperty("co2").send(String(curCO2));
	}
}
