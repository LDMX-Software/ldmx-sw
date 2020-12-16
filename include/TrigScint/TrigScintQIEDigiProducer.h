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
#include "TrigScint/Event/TrigScintQIEDigis.h"
#include "TrigScint/SimQIE.h"

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
    /** Has been seeded? */
    bool hasSeed() const { return random_.get()!=nullptr; }

 private:
    /// Random number generator 
    std::unique_ptr<TRandom3> random_{nullptr}; 
            
    /// Generate noise
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

    /// QIE Input pulse shape
    std::string input_pulse_shape_;

    /// QIE Input pulse parameters
    std::vector<float> pulse_params;

    /// Overall input pulse time offset
    float toff_overall_;

    /// no. of time samples analysed by QIE
    int maxts_;
    
    /// QIE TDC Current threshold
    float tdc_thr;
    
    /// QIE pedestal
    float pedestal;
    
    /// QIE electronic noise
    float elec_noise;

    /// SiPM Gain
    float sipm_gain;
    
    SimQIE* smq{nullptr};

  };

}

#endif
