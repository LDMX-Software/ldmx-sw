/**
 * @file EcalNoiseGenerator.h
 * @brief Utility used to generate Ecal noise hits.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef TOOLS_ECALNOISEGENERATOR_H
#define TOOLS_ECALNOISEGENERATOR_H

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>
#include <time.h>
#include <vector>

//----------//
//   LDMX   //
//----------//
#include "Event/EcalHit.h"
#include "Tools/NoiseGenerator.h"

namespace ldmx { 

    class EcalNoiseGenerator : public NoiseGenerator { 

        public: 

            /** Constructor */
            EcalNoiseGenerator();

            /** Destructor */
            ~EcalNoiseGenerator(); 
        
            /**
             * Generate Ecal noise hits.
             *
             * @param emptyChannels The total number of channels without a hit 
             *                      on them.
             * @return A vector containing EcalHits.
             */
            std::vector<EcalHit*> generateNoiseHits(int emptyChannels); 

    }; // EcalEcalNoiseGenerator

} // ldmx

#endif // TOOLS_ECALNOISEGENERATOR_H
