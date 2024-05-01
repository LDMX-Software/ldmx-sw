#include "TSTrackCollection.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace std;

void TSTrackCollection::add(size_t event, TSTrackArray &tracks) {
	this->events[event] = tracks;
}

bool TSTrackCollection::get(size_t event, TSTrackArray &tracks) const {
	TrackCollectionMap::const_iterator it = this->events.find(event);
	if (it == this->events.end()) return false;

	tracks = it->second;

	return true;
}

void TSTrackCollection::read(const string &filename) {
	ifstream rs(filename);
	if (!rs) throw runtime_error("Cannot open input file: " + filename);

	for (string line; getline(rs, line);) {
		if (line.length() <= 1 || line[0] == '#') continue;

		size_t event;

		istringstream ss(line);
		ss >> event;

		TSTrackArray trackArray;
		for (unsigned int i = 0; i < TS_PARAM_MAX_TRACKS; i++) {
			int downId, upId, taggerId, weight;

			if (ss >> downId >> upId >> taggerId >> weight) {
				trackArray.tracks[i] = TSTrack(downId, upId, taggerId, weight);
			} else {
				runtime_error("Failed to parse file");
			}
		}

		this->add(event, trackArray);
	}
}

void TSTrackCollection::write(const string &filename) const {
	ofstream of(filename);
	if (!of) throw runtime_error("Cannot open output file: " + filename);

	of << "# Automatically generated" << endl;

	for (const auto& event : this->events) {
		of << dec << setfill(' ') << setw(4) << event.first << "   ";
		for (unsigned int i = 0; i < TS_PARAM_MAX_TRACKS; i++) {
			of << event.second.tracks[i].stringify() << "   ";
		}
		of << endl;
	}

	of.close();
}

bool TSTrackCollection::operator ==(const TSTrackCollection &b) const {
	if (this->events.size() != b.events.size()) return false;

	for (unsigned int i = 0; i < this->events.size(); i++){
		for (unsigned int j = 0; j < TS_PARAM_MAX_TRACKS; j++) {
			if (this->events.at(i).tracks[j] != b.events.at(i).tracks[j]) {
				cout << "Failed at " << i << ", " << j << endl;
//				cout << "Fail to match at " << i << ", " << j << endl;
//				cout << "a: " << this->events.at(i).tracks[j].stringify() << endl;
//				cout << "b: " << b.events.at(i).tracks[j].stringify() << endl;
				return false;
			}
		}
	}
	
	return true;
}
