#ifndef ECAL_ECALRAWDECODER_H_ 
#define ECAL_ECALRAWDECODER_H_ 

//----------//
//   LDMX   //
//----------//
#include "Framework/EventProcessor.h"

namespace ecal {

/**
 * @class EcalRawDecoder
 */
class EcalRawDecoder : public framework::Producer {
 public:
  /**
   * Constructor
   */
  EcalRawDecoder(const std::string& name, framework::Process& process);

  /**
   * Destructor
   */
  virtual ~EcalRawDecoder();

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
  /// output object to put onto event bus
  std::string output_name_;
  /// version of HGC ROC we are decoding
  int roc_version_;

};
}  // namespace ecal

#endif
