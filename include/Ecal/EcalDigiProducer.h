/**
 * @file EcalDigiProducer.h
 * @brief Class that performs basic ECal digitization
 * @author Owen Colegrove, UCSB
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef EVENTPROC_ECALDIGIPRODUCER_H_
#define EVENTPROC_ECALDIGIPRODUCER_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <memory> //for smart pointers
#include <set> //for tracking used detector IDs

//----------//
//   LDMX   //
//----------//
#include "Event/HgcrocDigiCollection.h"
#include "Event/EventConstants.h"
#include "Event/SimCalorimeterHit.h"
#include "DetDescr/EcalID.h"
#include "DetDescr/EcalHexReadout.h"
#include "Framework/EventProcessor.h"
#include "Tools/NoiseGenerator.h"
#include "Tools/HgcrocEmulator.h"

namespace ldmx {

/**
 * @class EcalDigiProducer
 * @brief Performs basic ECal digitization
 */
class EcalDigiProducer : public Producer {

 public:

  /**
   * Constructor
   * Makes unique noise generator and injector for this class
   */
  EcalDigiProducer(const std::string& name, Process& process);

  /**
   * Destructor
   * Deletes digi collection if it has been created
   */
  virtual ~EcalDigiProducer();

            /**
             * Configure this producer from the python configuration.
             * Sets event constants and configures the noise generator, noise injector, and pulse function.
             * Creates digi collection
             */
            virtual void configure(Parameters&);

            /**
             * Simulates measurement of pulse and creates digi collection for input event.
             */
            virtual void produce(Event& event);

        private:

            ///////////////////////////////////////////////////////////////////////////////////////
            //Python Configuration Parameters

	        /// input hit collection name
            std::string inputCollName_;

	        /// input pass name
            std::string inputPassName_;

	        /// output hit collection name
            std::string outputCollName_;
	  
            /// Time interval for chip clock in ns
            double clockCycle_;

            /// The gain in ADC units per MeV. 
            double gain_;

            /// The pedestal in ADC units 
            double pedestal_;

            /// Depth of ADC buffer. 
            int nADCs_; 

            /// Index for the Sample Of Interest in the list of digi samples 
            int iSOI_;

            /// Readout threshold [mV]
            double readoutThreshold_;

            /// Conversion from energy in MeV to voltage in mV
            double MeV_;

            ///////////////////////////////////////////////////////////////////////////////////////
            // Other member variables

            /// Put noise into empty channels, not configurable, only helpful in development
            bool noise_{true};

            /// Hgcroc Emulator to digitize analog voltage signals
            std::unique_ptr<HgcrocEmulator> hgcroc_;

            /// Total number of channels in the ECal
            int nTotalChannels_;

            /// Conversion from time in ns to ticks of the internal clock
            double ns_;

            /// Generates noise hits based off of number of cells that are not hit
            std::unique_ptr<NoiseGenerator> noiseGenerator_;

            /// Generates Gaussian noise on top of real hits
            std::unique_ptr<TRandom3> noiseInjector_;

    };
}

#endif
