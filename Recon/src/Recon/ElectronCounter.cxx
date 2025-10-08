#include "Recon/ElectronCounter.h"

namespace recon {

ElectronCounter::ElectronCounter(const std::string& name,
                                 framework::Process& process)
    : framework::Producer(name, process) {}

ElectronCounter::~ElectronCounter() {}

void ElectronCounter::configure(framework::config::Parameters& parameters) {
  input_coll_ = parameters.get<std::string>("input_collection");
  input_pass_name_ = parameters.get<std::string>("input_pass_name");
  output_coll_ = parameters.get<std::string>("output_collection");
  n_electrons_sim_ = parameters.get<int>("simulated_electron_number");
  use_sim_electron_count_ =
      parameters.get<bool>("use_simulated_electron_number");

  /*  // can rehash this for cluster vs track counting
  if (mode_ == 0) {
    algo_name_ = "LayerSumTrig";
  } else if (mode_ == 1) {
    algo_name_ = "CenterTower";
  }
  */
  ldmx_log(debug) << "ElectronCounter is using parameters: "
                  << " \n\tinput_collection = " << input_coll_
                  << " \n\tinput_pass_name = " << input_pass_name_
                  << " \n\toutput_collection = " << output_coll_
                  << " \n\tsimulated_electron_number = " << n_electrons_sim_
                  << " \n\tuse_simulated_electron_number = "
                  << use_sim_electron_count_;
}

void ElectronCounter::produce(framework::Event& event) {
  int n_electrons = -1;

  if (use_sim_electron_count_) {
    if (n_electrons_sim_ < 0) {
      ldmx_log(fatal)
          << "Can't use unset number of simulated electrons as electron count! "
             "Set with 'simulated_electron_number' ";
      return;
    }
    // then we just set it equal to simulated number and we're done
    n_electrons = n_electrons_sim_;
  }
  // Check if the collection of trig scint tracks exist.  If not,
  // don't bother processing the event.
  else {
    if (!event.exists(input_coll_, input_pass_name_)) {
      ldmx_log(fatal) << "Attemping to use non-existing input collection "
                      << input_coll_ << "_" << input_pass_name_
                      << " to count electrons! Exiting.";
      return;
    }
    // TODO, if cluster counting is needed: have two functions, one with tracks,
    // one with clusters, and just call one or the other.

    // Get the collection of TS tracks
    const std::vector<ldmx::TrigScintTrack> tracks =
        event.getCollection<ldmx::TrigScintTrack>(input_coll_,
                                                  input_pass_name_);

    n_electrons = tracks.size();
    ldmx_log(info) << "Found " << tracks.size()
                   << " electrons (tracks) using input collection "
                   << input_coll_ << "_" << input_pass_name_;
  }
  // add number of electrons to event header. allow for it to be unset (-1)
  event.getEventHeader().setIntParameter("nElectrons", n_electrons);
  event.setElectronCount(n_electrons);
}
}  // namespace recon

DECLARE_PRODUCER(recon::ElectronCounter)
