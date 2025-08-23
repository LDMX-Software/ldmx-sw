#ifndef TRIGGER_NTUPLEWRITER_H_
#define TRIGGER_NTUPLEWRITER_H_

#include "Framework/EventProcessor.h"
#include "Framework/NtupleManager.h"
#include "TFile.h"
// #include "TTree.h"
using std::vector;

namespace trigger {

/**
 * @class NtupleWriter
 * @brief Null algorithm test
 */
class NtupleWriter : public framework::Producer {
 public:
  NtupleWriter(const std::string& name, framework::Process& process);
  virtual void configure(framework::config::Parameters&);
  virtual void produce(framework::Event& event);
  virtual void onProcessStart();
  virtual void onProcessEnd();

 private:
  TFile* out_file_{nullptr};
  std::string tag_{"Events"};
  std::string out_path_{"./ntuple.root"};

  std::string target_sp_hits_event_passname_;
  std::string target_sp_passname_;

  std::string ecal_sp_hits_events_passname_;
  std::string ecal_sp_passname_;

  std::string ecal_trig_sums_event_passname_;
  std::string ecal_trig_sums_passname_;

  std::string hcal_trig_quads_events_passname_;
  std::string hcal_trig_quads_passname_;

  std::string trig_electrons_event_passname_;
  std::string trig_electrons_passname_;

  bool write_truth_{true};
  bool write_ele_{true};
  bool write_ecal_sums_{true};
  bool write_hcal_sums_{true};
  bool write_ecal_trig_mi_ps_{true};
  bool write_hcal_trig_mi_ps_{true};
};
}  // namespace trigger

#endif  // TRIGGER_NTUPLEWRITER_H_
