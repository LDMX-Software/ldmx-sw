#ifndef HCAL_HCALRAWDECODER_H_ 
#define HCAL_HCALRAWDECODER_H_ 

//----------//
//   LDMX   //
//----------//
#include "Framework/EventProcessor.h"

namespace ecal {

/**
 * @class HcalRawDecoder
 */
class HcalRawDecoder : public framework::Producer {
 public:
  /**
   * Constructor
   */
  HcalRawDecoder(const std::string& name, framework::Process& process);

  /**
   * Destructor
   */
  virtual ~HcalRawDecoder();

  /**
   */
  virtual void configure(framework::config::Parameters&);

  /**
   */
  virtual void produce(framework::Event& event);

 private:
  /// input object of encoded data
  std::string input_name_;
  /// input pass of creating encoded data
  std::string input_pass_;
  /// input raw binary file with only HGC ROC data
  std::string input_file_;
  /// output object to put onto event bus
  std::string output_name_;
  /// version of HGC ROC we are decoding
  int roc_version_;
  /// are get inputting from event bus?
  bool input_from_bus_;

};
}  // namespace ecal

#endif
