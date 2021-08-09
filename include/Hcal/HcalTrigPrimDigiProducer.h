#ifndef HCAL_HCALTRIGPRIMDIGIPRODUCER_H_
#define HCAL_HCALTRIGPRIMDIGIPRODUCER_H_

//----------------//
//   LDMX Core    //
//----------------//
#include "Framework/EventProcessor.h"

namespace hcal {

  /**
   * @class HcalTrigPrimDigiProducer
   * @brief Performs basic Hcal trigger reconstruction
   */
  class HcalTrigPrimDigiProducer : public framework::Producer {
 public:
    /**
     * Constructor
     */
    HcalTrigPrimDigiProducer(const std::string& name,
                             framework::Process& process);

    /**
     * Grabs configure parameters from the python config file.
     *
     * Parameter        Default
     * inputDigiCollName     HcalDigis
     * inputDigiPassName     "" <-- blank means take any pass if only one
     * collection exists
     */
    virtual void configure(framework::config::Parameters&);

    /**
     * Produce HcalHits and put them into the event bus using the
     * HcalDigis as input.
     */
    virtual void produce(framework::Event& event);

 private:
    /** Digi Collection Name to use as input */
    std::string digiCollName_;

    /** Digi Pass Name to use as input */
    std::string digiPassName_;

    /** Conditions object for the calibration information */
    std::string condObjName_;

    std::map<unsigned int, unsigned int> stq_tps;
  };
}  // namespace hcal

#endif  // HCAL_HCALTRIGPRIMDIGIPRODUCER_H_
