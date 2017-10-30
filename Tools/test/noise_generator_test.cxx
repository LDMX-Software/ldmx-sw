/** 
 * @file noise_generator_test.cxx
 * @brief Test for NoiseGenerator and all sub-classes.
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
#include "Tools/EcalNoiseGenerator.h"

using namespace ldmx; 

int main(int, const char* argv []) { 
    
    std::cout << "[ EcalNoiseGenerator ]: Running NoiseGenerator test..." << std::endl;

    EcalNoiseGenerator* generator = new EcalNoiseGenerator();
    int emptyChannels = 80000;
    
    std::vector<EcalHit*> ecalNoiseHits = generator->generateNoiseHits(emptyChannels);
    std::cout << "[ EcalNoiseGenerator ]: Total number of noise hits: " 
              << ecalNoiseHits.size() << std::endl;

    std::cout << "[ EcalNoiseGenerator ]: Printing hit information: " << std::endl; 
    for (EcalHit* ecalNoiseHit : ecalNoiseHits) { 
        ecalNoiseHit->Print(); 
    }

    return 0; 
}
