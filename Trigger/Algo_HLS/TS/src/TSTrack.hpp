#ifndef __TSTRIGGER_H__
#define __TSTRIGGER_H__

#include <stdint.h>
#include <sstream>
#include <string>

class TSTrack {
public:
	TSTrack() : downId(0), upId(0), taggerId(0), weight(0) {};
	TSTrack(uint8_t downId, uint8_t upId, uint8_t taggerId, uint16_t weight) :
		downId(downId), upId(upId), taggerId(taggerId), weight(weight) {};

	// Getters
	inline uint8_t getDownId() const {return this->downId;};
	inline uint8_t getUpId() const {return this->upId;};
	inline uint8_t getTaggerId() const {return this->taggerId;};
	inline uint16_t getWeight() const {return this->weight;};

	std::string stringify() const;

	// Comparators
	bool operator ==(const TSTrack &b) const;
	inline bool operator !=(const TSTrack &b) const {return !(*this == b);};

private:
	uint8_t downId;
	uint8_t upId;
	uint8_t taggerId;
	uint16_t weight;
};

#endif /* !__TSTRIGGER_H__ */
