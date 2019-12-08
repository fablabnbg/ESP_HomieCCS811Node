/*
 * ESPHomieCCS811Node.h
 *
 *  Created on: 01.05.2019
 *      Author: ian
 */

#ifndef SRC_ESPHOMIECCS811NODE_H_
#define SRC_ESPHOMIECCS811NODE_H_

#include <Homie.hpp>
#include "SparkFunCCS811.h" //Click here to get the library: http://librarymanager/All#SparkFun_CCS811
#include <HomieBME280Node.h>


//#define CCS811_ADDR 0x5B //Default I2C Address
#define CCS811_ADDR 0x5A //Alternate I2C Address

class ESP_HomieCCS811Node: public HomieNode {
public:
	ESP_HomieCCS811Node(const HomieBME280Node &bme280);
	virtual void setup() override;
	virtual void loop() override;

private:
	const HomieBME280Node& bme280;
	CCS811 sensor;
	float curTemp;
	uint16_t curTVOC;
	uint16_t curCO2;
};

#endif /* SRC_ESPHOMIECCS811NODE_H_ */
