#ifndef __TSTRIGGERCOLLECTION_HPP__
#define __TSTRIGGERCOLLECTION_HPP__

#include "../src/TSTriggerParameters.hpp"
#include "../src/TSTrack.hpp"
#include <map>

class TSTrackCollection final {
public:
	struct TSTrackArray {
		TSTrack tracks[TS_PARAM_MAX_TRACKS];
	};

	TSTrackCollection() {};

	void add(size_t event, TSTrackArray &tracks);
	bool get(size_t event, TSTrackArray &tracks) const;

	void read(const std::string &filename);
	void write(const std::string &filename) const;

	bool operator ==(const TSTrackCollection &b) const;
	inline bool operator !=(const TSTrackCollection &b) const {return !(*this == b);};

	inline size_t size() const {return this->events.size();};

private:

	typedef std::map<size_t, TSTrackArray> TrackCollectionMap;
	TrackCollectionMap events;
};


#endif /* !__TSTRIGGERCOLLECTION_HPP__ */
