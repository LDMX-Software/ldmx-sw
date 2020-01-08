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
#include <memory> //for smart pointers

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

            virtual ~EcalDigiProducer();

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
            static const int NUM_ECAL_LAYERS{34};

            /** Total number of hex modules per layer. */
            static const int HEX_MODULES_PER_LAYER{7}; 

            /** Total number of cells (channels) per hex module. */
            static const int CELLS_PER_HEX_MODULE{397};

            /** Total number of cells across all modules. */
            static const int TOTAL_CELLS{NUM_ECAL_LAYERS*HEX_MODULES_PER_LAYER*CELLS_PER_HEX_MODULE};


            std::unique_ptr<TRandom3> noiseInjector_;
            TClonesArray* ecalDigis_{nullptr};
            EcalDetectorID detID_;
            std::unique_ptr<EcalHexReadout> hexReadout_;
          
            /** Generator of noise hits. */ 
            std::unique_ptr<NoiseGenerator> noiseGenerator_; 
           
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
