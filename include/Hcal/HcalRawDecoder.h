#ifndef HCAL_HCALRAWDECODER_H_ 
#define HCAL_HCALRAWDECODER_H_ 

//----------//
//   LDMX   //
//----------//
#include "Framework/EventProcessor.h"

namespace hcal {

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
  /// read in binary file of HGC ROC data
  std::string input_file_;
  /// output object to put onto event bus
  std::string output_name_;
  /// version of HGC ROC we are decoding
  int roc_version_;
  /// are get translating electronic IDs?
  bool translate_eid_;

};
}  // namespace hcal

#endif
