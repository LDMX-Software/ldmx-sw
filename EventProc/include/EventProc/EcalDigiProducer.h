/**
 * @file EcalDigiProducer.h
 * @brief Class that performs basic ECal digitization
 * @author Owen Colegrove, UCSB
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef EVENTPROC_ECALDIGIPRODUCER_H_
#define EVENTPROC_ECALDIGIPRODUCER_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <time.h>

//----------//
//   ROOT   //
//----------//
#include "TRandom3.h"
#include "TClonesArray.h"

//----------//
//   LDMX   //
//----------//
#include "Event/EcalHit.h"
#include "Event/EventConstants.h"
#include "Event/SimCalorimeterHit.h"
#include "DetDescr/DetectorID.h"
#include "DetDescr/EcalDetectorID.h"
#include "DetDescr/EcalHexReadout.h"
#include "Framework/EventProcessor.h"
#include "Tools/NoiseGenerator.h"

namespace ldmx {

    /**
     * @class EcalDigiProducer
     * @brief Performs basic ECal digitization
     */
    class EcalDigiProducer : public Producer {

        public:

            typedef std::pair<int, int> layer_cell_pair;

            typedef std::pair<int, float> cell_energy_pair;

            EcalDigiProducer(const std::string& name, Process& process);

            virtual ~EcalDigiProducer() {
            }

            virtual void configure(const ParameterSet&);

            virtual void produce(Event& event);

        private:
            
            /** 
             * Calculate the noise in electrons given the pad capacitance. 
             *
             * @param capacitance capacitance in pF
             * @return noise in electrons
             */
            double calculateNoise(const double capacitance, const double noiseIntercept, const double noiseSlope) { 
                return noiseIntercept + noiseSlope*capacitance;
            } 
            
            inline layer_cell_pair hitToPair(SimCalorimeterHit* hit) {
                int detIDraw = hit->getID();
                detID_.setRawValue(detIDraw);
                detID_.unpack();
                int layer = detID_.getFieldValue("layer");
                int cellid = detID_.getFieldValue("cell");
                return (std::make_pair(layer, cellid));
            }

        private:

            /** Electrons per MIP. */
            static const double ELECTRONS_PER_MIP; 

            /** MIP response in MeV. */
            static const double MIP_SI_RESPONSE;

            /** Total number of Ecal layers. */
<<<<<<< HEAD
            static const int NUM_ECAL_LAYERS{34};
=======
            static const int NUM_ECAL_LAYERS{33};
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8

            /** Total number of hex modules per layer. */
            static const int HEX_MODULES_PER_LAYER{7}; 

            /** Total number of cells (channels) per hex module. */
            static const int CELLS_PER_HEX_MODULE{397};

            /** Total number of cells across all modules. */
            static const int TOTAL_CELLS{NUM_ECAL_LAYERS*HEX_MODULES_PER_LAYER*CELLS_PER_HEX_MODULE};


            TRandom3* noiseInjector_{new TRandom3(time(nullptr))};
            TClonesArray* ecalDigis_{nullptr};
            EcalDetectorID detID_;
            EcalHexReadout* hexReadout_{nullptr};
          
            /** Generator of noise hits. */ 
<<<<<<< HEAD
            NoiseGenerator* noiseGenerator_; 
=======
            NoiseGenerator* noiseGenerator_{new NoiseGenerator{}}; 
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
           
            /** Set the noise (in electrons) when the capacitance is 0. */
            double noiseIntercept_{900.};

            /** Noise RMS in units of electrons. */
            double noiseRMS_{0}; 

            /** Set the capacitative noise slope (electrons/pF). */
            double noiseSlope_{22.}; 

            /** Capacitance per cell pad. */
            double padCapacitance_{27.56}; // pF 
            
            /** 
             * Set the threshold for reading out a channel. Units are 
             * multiples of RMS noise. 
             */
            double readoutThreshold_{4.};
    };
}

#endif
