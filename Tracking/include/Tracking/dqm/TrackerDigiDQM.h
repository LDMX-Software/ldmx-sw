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

  /**
   *
   */
  void analyze(const framework::Event& event) override;
};
}  // namespace tracking::dqm
