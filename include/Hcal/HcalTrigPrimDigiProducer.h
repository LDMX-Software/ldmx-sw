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

  virtual ~HcalTrigPrimDigiProducer() = default;
  /**
   * Grabs configure parameters from the python config file.
   *
   * Parameter        Default
   * digiCollName     HcalDigis
   * digiPassName     "" <-- blank means take any pass if only one
   * collection exists
   */
  void configure(framework::config::Parameters&) override;

  /**
   * Produce HcalTrigPrimDigis and put them into the event bus using the
   * HcalDigis as input.
   */
  void produce(framework::Event& event) override;

 private:
  /** Digi Collection Name to use as input */
  std::string digiCollName_;

  /** Digi Pass Name to use as input */
  std::string digiPassName_;

  /** Conditions object for the calibration information */
  std::string condObjName_;

  /** map of digis to the super trigger primitives */
  std::map<unsigned int, unsigned int> stq_tps;
};
}  // namespace hcal

#endif  // HCAL_HCALTRIGPRIMDIGIPRODUCER_H_
