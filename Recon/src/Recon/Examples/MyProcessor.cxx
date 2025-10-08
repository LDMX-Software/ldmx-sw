
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

  int my_parameter = parameters.get<int>("my_parameter");

  ldmx_log(info) << "MyProcessor configured with my_parameter = "
                 << my_parameter;

  ecal_rechits_passname_ = parameters.get<std::string>("ecal_rechits_passname");
  ecal_rec_hits_event_passname_ =
      parameters.get<std::string>("ecal_rec_hits_event_passname");
}

void MyProcessor::produce(framework::Event &event) {
  // Check if the collection of reconstructed ECal hits_ exist.  If not,
  // don't bother processing the event.
  if (!event.exists("EcalRecHits", ecal_rec_hits_event_passname_)) return;

  // Get the collection of digitized ECal hits_ from the event
  const std::vector<ldmx::EcalHit> hits =
      event.getCollection<ldmx::EcalHit>("EcalRecHits", ecal_rechits_passname_);

  // Loop over the collection of hits_ and print the hit details
  for (const auto &hit : hits) {
    // Print the hit
    ldmx_log(info) << hit;
  }
}
}  // namespace recon

DECLARE_PRODUCER(recon::MyProcessor)
