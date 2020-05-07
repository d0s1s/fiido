#include "I2CScanner.h"

byte I2CScanner::scan(byte address) {
	Wire.beginTransmission(address);
	byte error = Wire.endTransmission();
	return error;
}

void I2CScanner::Init(){
	dacAddr = 0;
	Wire.begin();
	for (uint8_t index = 0; index < sizeof(addresses); index++) {
		if (Check(addresses[index])) {
			dacAddr = addresses[index];
			break;
		}
	}
}

void I2CScanner::Init(uint8_t address) {
	Wire.begin();
	dacAddr = Check(address)?address:0;
}

bool I2CScanner::Check(byte address) {
	return (scan(address) == 0);
}

bool I2CScanner::isDacDetected() {
	return dacAddr != 0;
}

uint8_t I2CScanner::getDacAddress() {
	return dacAddr;
}

// EOF
