#ifndef __TSDIGICOLLECTION_HPP__
#define __TSDIGICOLLECTION_HPP__

#include "../src/TSDigi.hpp"
#include <vector>
#include <map>

class TSDigiCollection final {
public:
	typedef std::map<size_t, std::vector<TSDigi> > DigiCollectionMap;

	TSDigiCollection() : event_size(-1) {};

	void add(size_t event, std::vector<TSDigi> &digis);
	bool get(size_t event, std::vector<TSDigi>& digis) const;

	void read(const std::string &filename);
	void write(const std::string &filename) const;

	bool operator ==(const TSDigiCollection &b) const;
	inline bool operator !=(const TSDigiCollection &b) const {return !(*this == b);};

	inline size_t size() const {return this->events.size();};
	inline DigiCollectionMap map() const {return this->events;};

private:
	DigiCollectionMap events;
	size_t event_size;
};


#endif /* !__TSDIGICOLLECTION_HPP__ */
