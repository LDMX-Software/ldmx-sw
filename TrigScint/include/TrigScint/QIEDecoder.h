#ifndef TRIGSCINT_QIEDECODER_H
#define TRIGSCINT_QIEDECODER_H

#include <TTimeStamp.h>

#include <fstream>
#include <iostream>

#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor
#include "TrigScint/Event/QIEStream.h"
#include "TrigScint/Event/TrigScintQIEDigis.h"
namespace trigscint {

class QIEDecoder : public framework::Producer {
 public:
  QIEDecoder(const std::string& name, framework::Process& process)
      : Producer(name, process) {}

  /**
   * Default destructor, closes up boost archive and input stream
   */
  virtual ~QIEDecoder() = default;

  /**
   * Configure our converter based off the configuration parameters
   * decoded from the passed python script
   */
  void configure(framework::config::Parameters& ps) override;

  void produce(framework::Event& event) override;

  void onProcessStart() override;

  void onProcessEnd() override;

 private:
  /// the channel mapping
  std::string channel_map_file_name_;
  std::ifstream channel_map_file_;
  std::map<int, int> channel_map_;

  // input/output collection and pass name
  std::string input_collection_;
  std::string output_collection_;
  std::string input_pass_name_;

  // number of channels in the pad
  int n_channels_{50};
  // number of time samples making up the event
  int n_samples_{5};
  // configurable flag, to set the isRealData bit in the event header
  bool is_real_data_{false};

};  // encoder

}  // namespace trigscint

#endif  // TRIGSCINT_QIEDECODER_H
