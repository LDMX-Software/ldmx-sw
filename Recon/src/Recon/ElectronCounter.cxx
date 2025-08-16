#include "Recon/ElectronCounter.h"

namespace recon {

ElectronCounter::ElectronCounter(const std::string& name,
                                 framework::Process& process)
    : framework::Producer(name, process) {}

ElectronCounter::~ElectronCounter() {}

void ElectronCounter::configure(framework::config::Parameters& parameters) {
  inputColl_ = parameters.get<std::string>("input_collection");
  input_pass_name_ = parameters.get<std::string>("input_pass_name");
  outputColl_ = parameters.get<std::string>("output_collection");
  nElectronsSim_ = parameters.get<int>("simulated_electron_number");
  useSimElectronCount_ = parameters.get<bool>("use_simulated_electron_number");

  /*  // can rehash this for cluster vs track counting
  if (mode_ == 0) {
    algoName_ = "LayerSumTrig";
  } else if (mode_ == 1) {
    algoName_ = "CenterTower";
  }
  */
  ldmx_log(debug) << "ElectronCounter is using parameters: "
                  << " \n\tinput_collection = " << inputColl_
                  << " \n\tinput_pass_name = " << input_pass_name_
                  << " \n\toutput_collection = " << outputColl_
                  << " \n\tsimulated_electron_number = " << nElectronsSim_
                  << " \n\tuse_simulated_electron_number = "
                  << useSimElectronCount_;
}

void ElectronCounter::produce(framework::Event& event) {
  int nElectrons = -1;

  if (useSimElectronCount_) {
    if (nElectronsSim_ < 0) {
      ldmx_log(fatal)
          << "Can't use unset number of simulated electrons as electron count! "
             "Set with 'simulated_electron_number' ";
      return;
    }
    // then we just set it equal to simulated number and we're done
    nElectrons = nElectronsSim_;
  }
  // Check if the collection of trig scint tracks exist.  If not,
  // don't bother processing the event.
  else {
    if (!event.exists(inputColl_, input_pass_name_)) {
      ldmx_log(fatal) << "Attemping to use non-existing input collection "
                      << inputColl_ << "_" << input_pass_name_
                      << " to count electrons! Exiting.";
      return;
    }
    // TODO, if cluster counting is needed: have two functions, one with tracks,
    // one with clusters, and just call one or the other.

    // Get the collection of TS tracks
    const std::vector<ldmx::TrigScintTrack> tracks =
        event.getCollection<ldmx::TrigScintTrack>(inputColl_, input_pass_name_);

    nElectrons = tracks.size();
    ldmx_log(info) << "Found " << tracks.size()
                   << " electrons (tracks) using input collection "
                   << inputColl_ << "_" << input_pass_name_;
  }
  // add number of electrons to event header. allow for it to be unset (-1)
  event.getEventHeader().setIntParameter("nElectrons", nElectrons);
  event.setElectronCount(nElectrons);
}
}  // namespace recon

DECLARE_PRODUCER(recon::ElectronCounter)
