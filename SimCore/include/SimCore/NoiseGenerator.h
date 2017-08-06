/**
 * @file NoiseGenerator.h
 * @brief Utility used to generate noise hits.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_NOISEGENERATOR_H
#define SIMCORE_NOISEGENERATOR_H

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>
#include <time.h>
#include <vector>

//----------//
//   ROOT   //
//----------//
#include "Math/DistFunc.h"
#include "TRandom3.h"

//----------//
//   LDMX   //
//----------//
#include "Event/EcalHit.h"

namespace ldmx { 

    class NoiseGenerator { 

        public: 

            /** Constructor */
            NoiseGenerator();

            /** Destructor */
            ~NoiseGenerator(); 
        
            /**
             * Generate noise hits.
             *
             * @param emptyChannels The total number of channels without a hit 
             *                      on them.
             * @return A vector containing the amplitude of the noise hits.
             */
            std::vector<double> generateNoiseHits(int emptyChannels); 

            /**
             * Generate Ecal noise hits.
             *
             * @param emptyChannels The total number of channels without a hit 
             *                      on them.
             * @return A vector containing EcalHits.
             */
            std::vector<EcalHit*> generateEcalNoiseHits(int emptyChannels);

            /** Set the noise threshold. */
            void setNoiseThreshold(double noiseThreshold) { noiseThreshold_ = noiseThreshold; }

            /** Set the mean noise. */
            void setNoise(double noise) { noise_ = noise; };

            /** Set the pedestal. */
            void setPedestal(double pedestal) { pedestal_ = pedestal; }; 
        
        private:

            /** Random number generator. */
            TRandom3* random_{new TRandom3(time(nullptr))}; 

            /** The noise threshold. */
            double noiseThreshold_{4}; 

            /** Mean noise. */
            double noise_{1};
            
            /** Pedestal or baseline. */
            double pedestal_{0};  

    }; // NoiseGenerator
} // ldmx

#endif // SIMCORE_NOISEGENERATOR_H
