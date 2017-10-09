/**
 * @file EcalNoiseGenerator.h
 * @brief Utility used to generate Ecal noise hits.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_ECALNOISEGENERATOR_H
#define SIMCORE_ECALNOISEGENERATOR_H

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
#include "SimCore/NoiseGenerator.h"

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

#endif // SIMCORE_ECALNOISEGENERATOR_H
