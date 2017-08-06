/** 
 * @file noise_generator_test.cxx
 * @brief Test for NoiseGenerator.
 * @author Omar Moreno, SLAC National Accelerator Laboratory 
 */

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>

//----------//
//   LDMX   //
//----------//
#include "Event/EcalHit.h"
#include "SimCore/NoiseGenerator.h"

using namespace ldmx; 

int main(int, const char* argv []) { 
    
    std::cout << "[ NoiseGenerator ]: Running NoiseGenerator test..." << std::endl;

    NoiseGenerator* generator = new NoiseGenerator();
    int emptyChannels = 80000;
    
    std::vector<EcalHit*> ecalNoiseHits = generator->generateEcalNoiseHits(emptyChannels);
    std::cout << "[ NoiseGenerator ]: Total number of noise hits: " 
              << ecalNoiseHits.size() << std::endl;

    std::cout << "[ NoiseGenerator ]: Printing hit information: " << std::endl; 
    for (EcalHit* ecalNoiseHit : ecalNoiseHits) { 
        ecalNoiseHit->Print(); 
    }

    return 0; 
}
