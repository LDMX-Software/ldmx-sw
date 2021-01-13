#include <argp.h>
#include <iostream>
#include "../src/TSTriggerAlgo.hpp"
#include "TSDigiCollection.hpp"
#include "TSTrackCollection.hpp"

using namespace std;

/* argp declarations */
const char *algo_top_tb_version = "TSTriggerAlgoTB 1.0";
const char *algo_top_tb_bug_address = "<marcelo.vicente@stanford.edu>";

static char doc[] = "Trigger Scintillator HLS C/RTL Testbed";

static char args_doc[] = "TEST_FILE_IN";

static struct argp_option options[] = {
    {"write", 'w', "FILE", 0, "Writes algo result to target file", 0},
	{"compare", 'c', "FILE", 0, "Compare the algo output to target file", 0},
    {"verbose", 'v', 0, 0, "Produce verbose output", 0},
    {0},
};

struct arguments {
    char *readfile;
    char *writefile;
    char *cmpfile;
    bool verbose;
};

struct arguments arguments = {0};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = (struct arguments *)state->input;

    switch (key) {
    case ARGP_KEY_ARG: {
    	switch (state->arg_num) {
    	case 0: arguments->readfile = arg; break;
    	default: argp_usage(state); break; /* Too many arguments */
    	}
    } break;
    case 'w': arguments->writefile = arg; break;
    case 'c': arguments->cmpfile = arg; break;
    case 'v': arguments->verbose = true; break;
    default: return ARGP_ERR_UNKNOWN; break;
    }

    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

// Not used, just an example
void generate_test_data(TSDigiCollection& events, int n) {
	for (int i = 0; i < n; i++) {

		vector<TSDigi> digis;

		for (int j = 0; j < TS_PARAM_DIGIS_PER_EVENT; j++) {
			digis.push_back(TSDigi(i+j, 0));
		}

		events.add(i, digis);
	}
}

int main(int argc, char** argv) {
	TSDigiCollection eventsCollection;
	TSTrackCollection tracksCollection;

	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	// Read input data file
	if (!arguments.readfile) {
		cerr << "An input test vector is required" << endl;
		return -1;
	}
	eventsCollection.read(string(arguments.readfile));

	// Run all events through algo
	for (auto &event : eventsCollection.map()) {
		TSTrackCollection::TSTrackArray trackArray;

		trackArray.tracks[0] = TSTrack(10,10,10,10);

		TSTriggerAlgo(event.second.data(), trackArray.tracks);

		if (arguments.verbose) {
			cout << "Event " << event.first << ": tracks=";
			for (unsigned int i = 0; i < TS_PARAM_MAX_TRACKS; i++) {
				cout << "[" << trackArray.tracks[i].stringify() << "]";
			}
			cout << endl;
		}

		tracksCollection.add(event.first, trackArray);
	}

	// Write results to file
	if (arguments.writefile) {
		tracksCollection.write(string(arguments.writefile));
	}

	// Compare output results to reference
	if (arguments.cmpfile) {
		TSTrackCollection tracksCollection_ref;
		tracksCollection_ref.read(string(arguments.cmpfile));

		if (tracksCollection == tracksCollection_ref) {
			cout << "Algo output MATCHES reference file" << endl;
			return 0;
		} else {
			cout << "Algo output DOES NOT MATCH reference file" << endl;
			return 1;
		}
	}

	return 0;
}
