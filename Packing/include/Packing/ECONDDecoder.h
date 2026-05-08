#pragma once
#ifndef PACKING_ECONDDECODER_H
#define PACKING_ECONDDECODER_H

#include "Framework/EventProcessor.h"
#include "Framework/Exception/Exception.h"

namespace packing {

/**
 * decode event packets built by ECON-D chips
 *
 * The ECON-D chip is used to construct the event packets
 * for both the Ecal and Hcal subsystems, so both will share
 * this decoder.
 */
class ECONDDecoder : public framework::Producer {
 public:
  /// normal constructor
  ECONDDecoder(const std::string& name, framework::Process& p)
      : framework::Producer(name, p) {}
  /// empty destructor
  virtual ~ECONDDecoder() = default;

  /**
   * configure the decoder
   *
   * mostly focused on telling it which subsystem we are decoding
   * (to know how to pack the ID numbers correctly), the input
   * and output event objects.
   */
  void configure(framework::config::Parameters& ps) override;

  /**
   * decode the raw binary data into HgcrocDigiCollection that can
   * be used later for reconstruction or analysis
   */
  void produce(framework::Event& event) override;

 private:
  /// name of event object of raw binary data
  std::string raw_data_name_;
  /// name of pass to get raw binary data from
  std::string raw_data_pass_;
  /// output name of digi collection
  std::string digi_output_name_;
  /**
   * which calorimeter subsystem we are decoding
   * (true == Ecal, false == Hcal)
   */
  bool is_ecal_;
};

}

#endif
