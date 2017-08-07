/**
 * @file EcalNoiseGenerator.h
 * @brief Utility used to generate noise hits.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimCore/EcalNoiseGenerator.h"

namespace ldmx { 

    EcalNoiseGenerator::EcalNoiseGenerator() {
    }

    EcalNoiseGenerator::~EcalNoiseGenerator() {
    }
    
    std::vector<EcalHit*> EcalNoiseGenerator::generateNoiseHits(int emptyChannels) { 
       
        std::vector<double> noiseHits = NoiseGenerator::generateNoiseHits(emptyChannels); 
        
        std::vector<EcalHit*> ecalNoiseHits;
        for (double noiseHit : noiseHits) { 
            EcalHit* hit = new EcalHit();
            hit->setEnergy(noiseHit);
            hit->setAmplitude(noiseHit);
            ecalNoiseHits.push_back(hit); 
        } 

        return ecalNoiseHits; 
    }
}
