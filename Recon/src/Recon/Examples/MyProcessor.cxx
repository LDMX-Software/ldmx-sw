
#include "Recon/Examples/MyProcessor.h"

namespace recon {

MyProcessor::MyProcessor(const std::string &name, framework::Process &process)
    : framework::Producer(name, process) {}

MyProcessor::~MyProcessor() {}

void MyProcessor::configure(framework::config::Parameters &parameters) {
  /**
   * You access configuration parameters set in the python
   * by asking for the parameter with the same name as the
   * python member variable.
   */

  int my_parameter = parameters.getParameter<int>("my_parameter");

  std::cout << "MyProcessor has my_parameter = " << my_parameter << std::endl;
}

void MyProcessor::produce(framework::Event &event) {
  // Check if the collection of reconstructed ECal hits exist.  If not,
  // don't bother processing the event.
  if (!event.exists("EcalRecHits")) return;

  // Get the collection of digitized ECal hits from the event
  const std::vector<ldmx::EcalHit> hits =
      event.getCollection<ldmx::EcalHit>("EcalRecHits");

  // Loop over the collection of hits and print the hit details
  for (const ldmx::EcalHit &hit : hits) {
    // Print the hit
    hit.Print();
  }
}
}  // namespace recon

DECLARE_PRODUCER_NS(recon, MyProcessor)
