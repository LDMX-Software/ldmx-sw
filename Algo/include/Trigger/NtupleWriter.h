#ifndef TRIGGER_NTUPLEWRITER_H_
#define TRIGGER_NTUPLEWRITER_H_

#include "Framework/EventProcessor.h"
#include "Framework/NtupleManager.h"

#include "TFile.h"
// #include "TTree.h"

namespace trigger {

/**
 * @class NtupleWriter
 * @brief Null algorithm test 
 */
class NtupleWriter : public framework::Producer {
 public:
  NtupleWriter(const std::string& name,
                           framework::Process& process);
  virtual void configure(framework::config::Parameters&);
  virtual void produce(framework::Event& event);
  virtual void onProcessStart();
  virtual void onProcessEnd();

 /* private: */
  TFile* outFile_{nullptr};
  /* TTree* eventTree_{nullptr}; */
  

};
}  // namespace trigger

#endif // TRIGGER_NTUPLEWRITER_H_
