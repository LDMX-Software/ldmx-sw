/**
 * @file TrigScintQIEDigiProducer.h
 * @brief Class that simulates QIE chip of the trigger scintillator
 * @author Niramay Gogate, Texas Tech University
 */

#ifndef EVENTPROC_TRIGSCINTQIEDIGIPRODUCER_H
#define EVENTPROC_TRIGSCINTQIEDIGIPRODUCER_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <time.h>

/*~~~~~~~~~~*/
/*   ROOT   */
/*~~~~~~~~~~*/
#include "TRandom3.h"

// LDMX
#include "DetDescr/TrigScintID.h"
#include "Event/EventConstants.h"
#include "Event/TrigScintHit.h"
#include "Event/SimCalorimeterHit.h"
#include "Tools/NoiseGenerator.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/EventProcessor.h"
#include "Framework/Parameters.h" 

// QIE output class
#include "Event/TrigScintQIEDigis.h"

namespace ldmx {

  enum TrigScintSection{
    UPSTREAM_TAGGER = 1,
    UPSTREAM_TARGET,
    DOWNSTREAM_TARGET,
    NUM_SECTIONS
  };

  /**
   * @class TrigScintQIEDigiProducer
   * @brief Class that simulates QIE chip of the trigger scintillator
   */
  class TrigScintQIEDigiProducer : public Producer {

 public:
    typedef int layer;
    typedef std::pair<double, double> zboundaries;
    TrigScintQIEDigiProducer(const std::string& name, Process& process);
    ~TrigScintQIEDigiProducer(); 

    /**
     * Callback for the processor to configure itself from the given set
     * of parameters.
     * 
     * @param parameters ParameterSet for configuration.
     */
    void configure(Parameters& parameters) final override;
    void produce(Event& event);
    TrigScintID generateRandomID(int module);

 private:
    /// Random number generator 
    std::unique_ptr<TRandom3> random_; 
            
    /// Generate noise hits given the number of channels and mean noise.
    std::unique_ptr<NoiseGenerator> noiseGenerator_;

    /// Class to set the verbosity level.  
    // TODO: Make use of the global verbose parameter. 
    bool verbose_{false};

    /// Name of the input collection containing the sim hits
    std::string inputCollection_;

    /// Name of the pass that the input collection is on (empty string means take any pass)
    std::string inputPassName_;

    /// Name of the output collection that will be used to stored the
    /// digitized trigger scintillator hits
    std::string outputCollection_;

    /// Number of strips per array
    int stripsPerArray_{50};

    /// Number of arrays
    int numberOfArrays_{3};

    /// Mean readout noise
    double meanNoise_{0};

    /// Total MeV per MIP
    double mevPerMip_{1.40};

    /// Total number of photoelectrons per MIP
    double pePerMip_{13.5};

  };

}

#endif
