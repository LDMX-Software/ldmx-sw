#include "TSDigi.hpp"
#include <sstream>

uint32_t TSDigi::getDecodedAdc() const {
	// This is a quick and dirty conversion from QIE exponent to linear scale.
	// A better understanding of the ranges and offsets need to be considered.

	// Implementation based on QIE
	uint8_t code = 0;
	uint8_t range = 0;
	uint32_t lin = 0;

	uint8_t adc = this->getEncodedAdc();

	code = adc & 0x3F;
	range = (adc >> 6) & 0x3;

	switch (range) {
		default:
		case 0: {
			if (code < 16) lin = code; // TODO since there is a negative
			else if (code < 36) lin = 11 + ((code - 16) << 1);
			else if (code < 57) lin = 51 + ((code - 36) << 2);
			else lin = 135 + ((code - 57) << 3);
		} break;

		case 1: {
			if (code < 16) lin = 167 + (code << 4);
			else if (code < 36) lin = 295 + ((code - 16) << 5);
			else if (code < 57) lin = 616 + ((code - 36) << 6);
			else lin = 1287 + ((code - 57) << 7);
		} break;

		case 2: {
			if (code < 16) lin = 1542 + (code << 8);
			else if (code < 36) lin = 2568 + ((code - 16) << 9);
			else if (code < 57) lin = 5129 + ((code - 36) << 10);
			else lin = 10516 + ((code - 57) << 11);
		} break;

		case 3: {
			if (code < 16) lin = 12548 + (code << 12);
			else if (code < 36) lin = 20742 + ((code - 16) << 13);
			else if (code < 57) lin = 41290 + ((code - 36) << 14);
			else lin = 84194 + ((code - 57) << 15);
		} break;
	}

	return lin;
}

std::string TSDigi::stringify() const {
	std::ostringstream oss;
	oss << (unsigned int)this->getEncodedAdc() << "," << (unsigned int)this->getTdc();
	return oss.str();
}

