/**
 * @file NoiseGenerator.h
 * @brief Utility used to generate noise hits.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef TOOLS_NOISEGENERATOR_H
#define TOOLS_NOISEGENERATOR_H

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>
#include <time.h>
#include <vector>

//--------------//
//  boost::math // 
//--------------//
#include <boost/math/distributions/poisson.hpp>

//----------//
//   ROOT   //
//----------//
#include "Math/DistFunc.h"
#include "TRandom3.h"

namespace ldmx { 

    class NoiseGenerator { 

        public: 

            /** Constructor */
            NoiseGenerator(double noiseValue=0.0001, bool gauss=true);

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

            /** Gaussian flag */
            bool useGaussianModel_{true};

            /** pdf for poisson errors */
            boost::math::poisson_distribution<>* poisson_dist_;
    }; // NoiseGenerator

} // ldmx

#endif // TOOLS_NOISEGENERATOR_H
