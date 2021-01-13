#ifndef __TSDIGI_H__
#define __TSDIGI_H__

#include <stdint.h>
#include <sstream>
#include <string>
//#include <ap_int.h>

class TSDigi {
public:
	TSDigi() : adc(0), tdc(0) {};
	TSDigi(uint8_t adc, uint8_t tdc) : adc(adc), tdc(tdc) {};

	// ADC
	inline uint8_t getEncodedAdc() const {return this->adc;};
	uint32_t getDecodedAdc() const;

	// TDC
	inline bool isTdcValid() const {return this->tdc < 50;}
	inline bool isTdcInvalid() const {return this->tdc == INVALID;};
	inline bool isTdcNotLocked() const {return this->tdc == DLL_NOT_LOCKED;};
	inline bool didSignalStartHigh() const {return this->tdc == STARTED_HIGH;};
	inline bool didSignalRemainLow() const {return this->tdc == REMAINED_LOW;};

	inline uint8_t getTdc() const {return this->tdc;};

	enum TDC_SPECIAL_CODE {
		INVALID = 60, DLL_NOT_LOCKED, STARTED_HIGH, REMAINED_LOW
	};

	std::string stringify() const;

	// Comparators
	inline bool operator ==(const TSDigi &b) const {return (this->adc == b.adc && this->tdc == b.tdc);};
	inline bool operator !=(const TSDigi &b) const {return !(*this == b);};

private:
	uint8_t adc;
	uint8_t tdc;
};

#endif /* !__TSDIGI_H__ */
