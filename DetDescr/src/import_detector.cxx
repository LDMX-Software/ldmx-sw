#include "TGeoManager.h"

#include <iostream>

/**
 * Import a detector into ROOT from GDML and write a .root file,
 * which can be used by the geometry service.
 *
 * The 'detector_full.gdml' file that is compatible with this program
 * can be generated using 'scripts/export_detector.py' to write a flat
 * GDML file from a modular one.
 */
int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: import_detector [input_gdml_file] [output_root_file]" << std::endl;
        exit(1);
    }
    char* input = argv[1]; // should be a detector_full.gdml file
    char* output = argv[2]; // should be a name ending in .root
    gGeoManager->Import(input);
    gGeoManager->Export(output);
}
