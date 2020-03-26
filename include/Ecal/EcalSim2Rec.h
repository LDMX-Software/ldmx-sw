/**
 * @file EcalSim2Rec.h
 * @brief Class that performs basic ECal digitization
 * @author Owen Colegrove, UCSB
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef EVENTPROC_ECALSIM2REC_H_
#define EVENTPROC_ECALSIM2REC_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <time.h>
#include <memory>

//----------//
//   ROOT   //
//----------//
#include "TRandom3.h"

//----------//
//   LDMX   //
//----------//
#include "Event/EventDef.h"
#include "DetDescr/DetectorID.h"
#include "DetDescr/EcalDetectorID.h"
#include "DetDescr/EcalHexReadout.h"
#include "Framework/EventProcessor.h"
#include "Tools/NoiseGenerator.h"
#include "Framework/Parameters.h" 

namespace ldmx {

    /**
     * @class EcalSim2Rec
     * @brief Performs basic ECal digitization
     */
    class EcalSim2Rec : public Producer {

        public:

            typedef std::pair<int, int> layer_cell_pair;

            typedef std::pair<int, float> cell_energy_pair;

            EcalSim2Rec(const std::string& name, Process& process);

            virtual ~EcalSim2Rec() {
            }

            /**
             * Callback for the processor to configure itself from the given set
             * of parameters.
             * 
             * @param parameters ParameterSet for configuration.
             */
            void configure(Parameters& parameters) final override; 

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
            
            inline layer_cell_pair hitToPair(const SimCalorimeterHit &hit) {
                int detIDraw = hit.getID();
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

            /** Layer Weights for Digitization of Energy accounting for the effect of the absorber */
            std::vector<double> layerWeights_;

            /** Second Order Energy Correction to the accounting done by the layer weights 
             *  SHOULD BE CLOSE TO 1.
             */
            double secondOrderEnergyCorrection_;

    };
}

#endif
