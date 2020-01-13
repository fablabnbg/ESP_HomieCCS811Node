/*
 * ESPHomieCCS811Node.cpp
 *
 *  Created on: 01.05.2019
 *      Author: ian
 */

#include <ESPHomieCCS811Node.h>
#include <LoggerNode.h>

ESP_HomieCCS811Node::ESP_HomieCCS811Node():
	HomieNode("ccs811", "CCS881 Sensor", "sensor_co2_vcoc"),
	sensor(0x5A), curTemp(0), curTVOC(0), curCO2(0)
{
	advertise("temperature").setName("Temperatur").setDatatype("float").setUnit("°C");
	advertise("co2").setName("CO2").setDatatype("float").setUnit("ppm");
	advertise("tvoc").setName("Org. fl. Gase").setDatatype("float").setUnit("ppm");
	advertise("mode").setName("Drive mode").setDatatype("enum").setFormat("Idle:M1s:M10s:M60s:M.25s").settable();
	advertise("baseline").setName("Sensor Baseline").setDatatype("int").settable();
}

void ESP_HomieCCS811Node::setup() {
	Wire.begin(SDA, SCL);
	CCS811Core::status returnCode = sensor.begin();
	if (returnCode != CCS811Core::SENSOR_SUCCESS) {
		Serial.printf("CCS811::begin() returned with error %x.\n", returnCode);
	}
	sensor.setDriveMode(2);
	sensor.setEnvironmentalData(50, 22);  // set reasonable values to start
}

void ESP_HomieCCS811Node::loop() {
	static uint32_t nextRead = 0;
	uint32_t now = millis();
	if (now > nextRead && sensor.dataAvailable()) {
		nextRead = now + 3000; // 120000; //60000;
		sensor.readAlgorithmResults();
		curTemp = sensor.getTemperature();
		curCO2 = sensor.getCO2();
		curTVOC =  sensor.getTVOC();
		uint16_t bl = sensor.getBaseline();
		if (LN.loglevel(LoggerNode::INFO)) {
			String logstring("Read values: CO2: ");
			logstring.concat(curCO2);
			logstring.concat("\tTVOC: ");
			logstring.concat(curTVOC);
			logstring.concat("\tBaseline: ");
			logstring.concat(bl);
			LN.log("ESP_HomieCCS811Node::loop()", LoggerNode::INFO, logstring);
		}
		if (sensor.checkForStatusError()) {
			String errStr("Error: ");
			getSensorError(errStr);
			LN.log("ESP_HomieCCS811Node::loop()", LoggerNode::WARNING, errStr);
		}
		if (Homie.isConnected()) {
			//setProperty("temperature").send(String(curTemp));
			setProperty("tvoc").send(String(curTVOC));
			setProperty("co2").send(String(curCO2));
			setProperty("baseline").send(String(bl));
		} else {
			LN.log("ESP_HomieCCS811Node::loop()", LoggerNode::DEBUG, "Not connected");
		}
	}
}

bool ESP_HomieCCS811Node::handleInput(const HomieRange &range, const String &property, const String &value) {
	LN.logf("ESP_HomieCCS811Node::handleInput", LoggerNode::DEBUG,	"Received property %s with value %s", property.c_str(), value.c_str());
	uint_fast8_t newMode = 0xFF;
	if (property.equals("mode")) {
		if (value.equals("idle")) newMode = 0;
		if (value.equals("M1s")) newMode = 1;
		if (value.equals("M10s")) newMode = 2;
		if (value.equals("M60s")) newMode = 3;
		if (value.equals("M.25s")) newMode = 4;
		if (newMode != 0xFF) {
			if (sensor.setDriveMode(newMode) == CCS811Core::SENSOR_SUCCESS) {
				setProperty("mode").setRetained(true).send(value);
			} else {
				LN.logf("ESP_HomieCCS811Node::handleInput", LoggerNode::ERROR, "Cannot set drive mode to %0x (%s)", newMode, value.c_str());
			}
			return true; // return true only if property was recognized and value in range (so also in case of correct values but sensor error)
		}
	}
	if (property.equals("baseline")) {
		uint16_t bl = value.toInt();
		if (sensor.setBaseline(bl) ==  CCS811Core::SENSOR_SUCCESS) {
			setProperty("baseline").setRetained(true).send(String(bl));
		} else {
			LN.logf("ESP_HomieCCS811Node::handleInput", LoggerNode::ERROR, "Cannot set baseline to %d (%s)", bl, value.c_str());
		}
		return true; // return true only if property was recognized and value in range (so also in case of correct values but sensor error)
	}

	return false;
}

void ESP_HomieCCS811Node::setEnvironmentalData(float temp, float hum) {
	bool err = temp > 100 || temp < 0 || hum < 5 || hum > 100 ;
	LN.logf("ESP_HomieCCS811Node::setEnvironmentalData()", err?LoggerNode::ERROR : LoggerNode::DEBUG, "%s Env data: [t: %f°C, h: %f relH%%]\n",  err ? "Invalid":"Set", temp, hum);
	if (!err) sensor.setEnvironmentalData(hum, temp);
}

bool ESP_HomieCCS811Node::getSensorError(String& errStr) {
	uint8_t error = sensor.getErrorRegister();
	if (error == 0xFF)
	{
		errStr = "Failed to get ERROR_ID register.";
		return false;
	} else {
		errStr = "Error: ";
		if (error & (1 << 5))
			errStr += "HeaterSupply ";
		if (error & (1 << 4))
			errStr += "HeaterFault ";
		if (error & (1 << 3))
			errStr += "MaxResistance ";
		if (error & (1 << 2))
			errStr += "MeasModeInvalid ";
		if (error & (1 << 1))
			errStr += "ReadRegInvalid ";
		if (error & (1 << 0))
			errStr += "MsgInvalid ";
		errStr.trim();
		return true;
	}
}
