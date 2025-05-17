#pragma once

#include "Framework/Event.h"
#include "Framework/EventProcessor.h"

namespace tracking::dqm {

class TrackerDigiDQM : public framework::Analyzer {
 public:
  /**
   */
  TrackerDigiDQM(const std::string& name, framework::Process& process)
      : framework::Analyzer(name, process){};

  /// Destructor
  ~TrackerDigiDQM() = default;

<<<<<<< HEAD
=======
  /**
   *
   */

>>>>>>> cf1ed4f7755547696813c2a1d162109827d0e9d5
  void configure(framework::config::Parameters& parameters) override;

  /**
   *
   */

  void analyze(const framework::Event& event) override;

 private:
  std::string measurements_passname_;
  std::string output_measurements_passname_;
};
}  // namespace tracking::dqm
