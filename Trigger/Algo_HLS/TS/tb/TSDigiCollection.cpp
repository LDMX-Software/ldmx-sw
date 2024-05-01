#include "TSDigiCollection.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace std;

void TSDigiCollection::add(size_t event, vector<TSDigi> &digis) {
	if (digis.size() == 0) runtime_error("Input vector is empty");
	if (this->event_size == -1) this->event_size = digis.size();
	if (this->event_size != digis.size()) {
		throw runtime_error("Trying to add digi vector with mismatched size");
	}

	events[event] = digis;
}

bool TSDigiCollection::get(size_t event, vector<TSDigi>& digis) const {
	DigiCollectionMap::const_iterator it = this->events.find(event);
	if (it == this->events.end()) return false;

	digis = it->second;
	return true;
}

void TSDigiCollection::read(const string &filename) {
	ifstream rs(filename);
	if (!rs) throw runtime_error("Cannot open input file: " + filename);

	for (string line; getline(rs, line);) {
		if (line.length() <= 1 || line[0] == '#') continue;

		size_t event;

		istringstream ss(line);
		ss >> event;

		vector<TSDigi> digis;
		string sadc, stdc;

		while (ss >> sadc >> stdc) {
			size_t adc = stoi(sadc, nullptr, 0);
			size_t tdc = stoi(stdc, nullptr, 0);

			digis.push_back(TSDigi(adc, tdc));
		}

		this->add(event, digis);
	}
}

void TSDigiCollection::write(const string &filename) const {
	ofstream of(filename);
	if (!of) throw runtime_error("Cannot open output file: " + filename);

	of << "# Automatically generated" << endl;

	for (const auto& event : this->events) {
		of << dec << setfill(' ') << setw(4) << event.first << "   ";

		for (const auto& digi : event.second) {
			of << dec << setfill(' ') << setw(3) << (unsigned int)digi.getEncodedAdc() << " ";
			of << dec << setfill(' ') << setw(2) << (unsigned int)digi.getTdc() << "   ";
		}

		of << endl;
	}

	of.close();
}

bool TSDigiCollection::operator ==(const TSDigiCollection &b) const {
	return this->event_size == b.event_size &&
			this->events.size() == b.events.size() &&
			equal(this->events.begin(), this->events.end(), b.events.begin());
}
