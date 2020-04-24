#ifndef _I2CSCANNER_h
#define _I2CSCANNER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <Wire.h>

typedef void(*I2C_Callback)(byte);

class I2CScanner {
 protected:
	byte inline scan(byte address);
	boolean dacDetected;
	uint8_t dacAddr;

 public:
	// DAC AVAILABLE ADDRESSES.
	const uint8_t addresses[6] = { 0x60, 0x61, 0x62, 0x63, 0x64, 0x65 };
	void Init();
	void Init(uint8_t address);
	bool Check(byte address);
	bool isDacDetected();
	uint8_t getDacAddress();
};

#endif

// EOF
