#ifndef ECAL_ECALRAWENCODER_H_ 
#define ECAL_ECALRAWENCODER_H_ 

//----------//
//   LDMX   //
//----------//
#include "Framework/EventProcessor.h"

namespace ecal {

/**
 * @class EcalRawEncoder
 */
class EcalRawEncoder : public framework::Producer {
 public:
  /**
   * Constructor
   */
  EcalRawEncoder(const std::string& name, framework::Process& process);

  /**
   * Destructor
   */
  virtual ~EcalRawEncoder();

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
