#ifndef TRIGGER_PROPAGATIONMAPWRITER_H_
#define TRIGGER_PROPAGATIONMAPWRITER_H_

#include "Framework/EventProcessor.h"
#include "TFile.h"
#include "TProfile2D.h"

namespace trigger {

/**
 * @class PropagationMapWriter
 * @brief Null algorithm test
 */

class PropagationMapWriter : public framework::Producer {
 public:
  PropagationMapWriter(const std::string& name, framework::Process& process);
  virtual void configure(framework::config::Parameters&);
  virtual void produce(framework::Event& event);
  virtual void onProcessStart();
  virtual void onProcessEnd();

 private:
  TFile* out_file_{nullptr};
  std::string out_path_{"./propagationMap.root"};
  std::string ecal_scoring_plane_passname_;
  std::string target_scoring_plane_passname_;
  std::string target_sp_hits_events_passname_;
  std::string ecal_sp_hits_events_passname_;
  TProfile2D* profx_{nullptr};
  TProfile2D* profy_{nullptr};
};
}  // namespace trigger

#endif  // TRIGGER_PROPAGATIONMAPWRITER_H_
