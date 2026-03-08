#ifndef TRIGSCINT_ZCCMDECODER_H
#define TRIGSCINT_ZCCMDECODER_H

#include <TTimeStamp.h>

#include <fstream>
#include <iostream>

#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor
#include "TrigScint/Event/TrigScintQIEDigis.h"
#include "TrigScint/Event/ZCCMOutput.h"
namespace trigscint {

class ZCCMDecoder : public framework::Producer {
 public:
  ZCCMDecoder(const std::string &name, framework::Process &process)
      : Producer(name, process) {}

  /**
   * Default destructor, closes up boost archive and input stream
   */
  virtual ~ZCCMDecoder() = default;

  /**
   * Configure our converter based off the configuration parameters
   * decoded from the passed python script
   */
  void configure(framework::config::Parameters &ps) override;

  void produce(framework::Event &event) override;

  void onProcessStart() override;

  void onProcessEnd() override;

 private:
  /// the channel mapping
  std::string channel_map_file_name_;
  std::ifstream channel_map_file_;
  std::map<int, int> channel_map_;
  /// the module mapping
  std::string module_map_file_name_;
  std::ifstream module_map_file_;
  std::map<int, int> module_map_;

  // input/output collection and pass name
  std::string input_collection_;
  std::string output_collection_;
  std::string input_pass_name_;

  // number of lanes in the readout message
  int number_of_lanes_{14};
  // number of channels served by each lane
  //  int number_of_channels_per_lane_{6}; //consider this static

  // number of channels in the readout (all pads)
  int n_channels_{number_of_lanes_ * ZCCMOutput::NUM_CHAN_PER_LANE};
  // number of time samples making up the event
  int n_samples_{70};
  // configurable flag, to set the isRealData bit in the event header
  bool is_real_data_{true};
  // keep track of which of max 4 modules are used (get from lane/module map)
  int modules_used_[4] = {0};

};  // decoder

}  // namespace trigscint

#endif  // TRIGSCINT_ZCCMDECODER_H
