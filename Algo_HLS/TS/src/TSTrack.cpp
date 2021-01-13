#include "TSTrack.hpp"
#include <sstream>

	uint8_t downId;
	uint8_t upId;
	uint8_t taggerId;
	uint16_t weight;

std::string TSTrack::stringify() const {
	std::ostringstream oss;
	oss << (int)this->downId << " " << (int)this->upId << " " << (int)this->taggerId << " " << (int)this->weight;
	return oss.str();
}

bool TSTrack::operator ==(const TSTrack &b) const {
	return (this->downId == b.downId &&
			this->upId == b.upId &&
			this->taggerId == b.taggerId &&
			this->weight == b.weight);
}
