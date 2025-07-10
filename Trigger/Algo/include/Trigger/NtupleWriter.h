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
  TFile* outFile_{nullptr};
  std::string tag_{"Events"};
  std::string outPath_{"./ntuple.root"};

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

  bool writeTruth_{true};
  bool writeEle_{true};
  bool writeEcalSums_{true};
  bool writeHcalSums_{true};
  bool writeEcalTrigMIPs_{true};
  bool writeHcalTrigMIPs_{true};
};
}  // namespace trigger

#endif  // TRIGGER_NTUPLEWRITER_H_
