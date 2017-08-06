/**
 * @file NoiseGenerator.h
 * @brief Utility used to generate noise hits.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimCore/NoiseGenerator.h"

namespace ldmx { 

    NoiseGenerator::NoiseGenerator() {
    }

    NoiseGenerator::~NoiseGenerator() {
        delete random_; 
    }
    
    std::vector<double> NoiseGenerator::generateNoiseHits(int emptyChannels) { 
        
        //std::cout << "[ Noise Generator ]: Empty channels: " 
        //          << emptyChannels << std::endl;
        //std::cout << "[ Noise Generator ]: Normalized integration limit: " 
        //          << noiseThreshold_ << std::endl;
        
        double integral = ROOT::Math::normal_cdf_c(noiseThreshold_, noise_, pedestal_);
        //std::cout << "[ Noise Generator ]: Integral: " 
        //          << integral << std::endl;

        double noiseHitCount = random_->Binomial(emptyChannels, integral); 
        //std::cout << "[ Noise Generator ]: # Noise hits: " 
        //          << noiseHitCount << std::endl;

        std::vector<double> noiseHits;
        for (int hitIndex = 0; hitIndex < noiseHitCount; ++hitIndex) { 
            
            double rand = random_->Uniform();
            //std::cout << "[ Noise Generator ]: Rand: " 
            //          << rand << std::endl;
            double draw = integral*rand; 
            //std::cout << "[ Noise Generator ]: Draw: " 
            //          << draw << std::endl;
            
            double cumulativeProb = 1.0 - integral + draw;
            //std::cout << "[ Noise Generator ]: Cumulative probability: " 
            //          << cumulativeProb << std::endl;
            
            double gaussAboveThreshold = ROOT::Math::gaussian_quantile(cumulativeProb, noise_);
            //std::cout << "[ Noise Generator ]: Noise value: " 
            //          << gaussAboveThreshold << std::endl;
            
            noiseHits.push_back(gaussAboveThreshold); 
        }

       return noiseHits;  
    }

    std::vector<EcalHit*> NoiseGenerator::generateEcalNoiseHits(int emptyChannels) {

        std::vector<double> noiseHits = this->generateNoiseHits(emptyChannels);
        
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
