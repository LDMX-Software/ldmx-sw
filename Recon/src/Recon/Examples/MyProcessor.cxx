
#include "Recon/Examples/MyProcessor.h"

namespace recon {

MyProcessor::MyProcessor(const std::string &name, framework::Process &process)
    : framework::Producer(name, process) {}

void MyProcessor::configure(framework::config::Parameters &parameters) {
  /**
   * You access configuration parameters set in the python
   * by asking for the parameter with the same name as the
   * python member variable.
   */

  int my_parameter = parameters.getParameter<int>("my_parameter");
  
  ecal_rechits_passname_ = parameters.getParameter<std::string>("ecal_rechits_passname");
  ecal_rec_hits_event_passname_ = parameters.getParameter<std::string>("ecal_rec_hits_event_passname"); 

  std::cout << "MyProcessor has my_parameter = " << my_parameter << std::endl;
}

void MyProcessor::produce(framework::Event &event) {
  // Check if the collection of reconstructed ECal hits exist.  If not,
  // don't bother processing the event.
  if (!event.exists("EcalRecHits", ecal_rec_hits_event_passname_)) return;

  // Get the collection of digitized ECal hits from the event
  const std::vector<ldmx::EcalHit> hits =
      event.getCollection<ldmx::EcalHit>("EcalRecHits", ecal_rechits_passname_);

  // Loop over the collection of hits and print the hit details
  for (const ldmx::EcalHit &hit : hits) {
    // Print the hit
    hit.Print();
  }
}
}  // namespace recon

DECLARE_PRODUCER(recon::MyProcessor)
