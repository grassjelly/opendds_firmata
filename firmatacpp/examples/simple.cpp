#include <iostream>

#include "firmata.h"
#include "firmserial.h"

#ifndef WIN32
#include "unistd.h"
#endif

/*
 * Detect first serial port with a StandardFirmata interface
 * Read analog inputs A0 and A1 and digital pin 2 (eg, a Playstation analog stick + button)
 * as well as I2C address 8 (eg, the slave_sender example that comes with Arduino IDE)
 * and print to stdout
 */

int main(int argc, const char* argv[])
{
	std::vector<firmata::PortInfo> ports = firmata::FirmSerial::listPorts();
	firmata::Firmata<firmata::Base, firmata::I2C>* f = NULL;
	firmata::FirmSerial* serialio;

	serialio = new firmata::FirmSerial("/dev/ttyACM0");
	while(serialio->available()) {
		f = new firmata::Firmata<firmata::Base, firmata::I2C>(serialio);
	}

	try {
		f->setSamplingInterval(100);

		f->reportAnalog(0, 1);
		f->reportAnalog(1, 1);

		f->reportDigital(0, 1);

		while (true) {
			f->parse();
			int a0 = f->analogRead("A0");
			float angle = (float)(a0 / 1023.0) * 180.0;
			std::cout << "Analog Read: " << angle << std::endl;
		};

		delete f;
	}
	catch (firmata::IOException e) {
		std::cout << e.what() << std::endl;
	}
	catch (firmata::NotOpenException e) {
		std::cout << e.what() << std::endl;
	}
}
