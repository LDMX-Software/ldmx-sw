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
  TFile* outFile_{nullptr};
  std::string outPath_{"./propagationMap.root"};
  TProfile2D* profx_{nullptr};
  TProfile2D* profy_{nullptr};
};
}  // namespace trigger

#endif  // TRIGGER_PROPAGATIONMAPWRITER_H_
