/**
 * @file PFTrackProducer.h
 * @brief Track selection skeleton for PFlow Reco
 * @author Christian Herwig, Fermilab
 */

#ifndef PFTRACKPRODUCER_H
#define PFTRACKPRODUCER_H

// LDMX Framework
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor

namespace recon {

/**
 * @class PFTrackProducer
 * @brief
 */
class PFTrackProducer : public framework::Producer {
 public:
  PFTrackProducer(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}

  virtual void configure(framework::config::Parameters& ps);

  virtual void produce(framework::Event& event);

 private:
  bool truthTracking_{true};

  // name of collection for track inputs to be passed
  std::string inputTrackCollName_;
  // pass name for the input collection
  std::string input_pass_name_;
  // name of collection for pfTracks to be output
  std::string outputTrackCollName_;
};
}  // namespace recon

#endif /* PFTRACKPRODUCER_H */
