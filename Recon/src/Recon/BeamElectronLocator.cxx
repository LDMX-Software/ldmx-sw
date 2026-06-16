#include "Recon/BeamElectronLocator.h"

namespace recon {

BeamElectronLocator::BeamElectronLocator(const std::string& name,
                                         framework::Process& process)
    : framework::Producer(name, process) {}

BeamElectronLocator::~BeamElectronLocator() {}

void BeamElectronLocator::configure(framework::config::Parameters& parameters) {
  input_coll_ = parameters.get<std::string>("input_collection");
  input_pass_name_ = parameters.get<std::string>("input_pass_name");
  output_coll_ = parameters.get<std::string>("output_collection");
  granularity_xmm_ = parameters.get<double>("granularity_x_mm");
  granularity_ymm_ = parameters.get<double>("granularity_y_mm");
  tolerance_ = parameters.get<double>("min_granularity_mm");
  min_xmm_ = parameters.get<double>("min_x_mm");
  max_xmm_ = parameters.get<double>("max_x_mm");
  min_ymm_ = parameters.get<double>("min_y_mm");
  max_ymm_ = parameters.get<double>("max_y_mm");
  verbose_ = parameters.get<bool>("verbose");
}
void BeamElectronLocator::onProcessStart() {
  ldmx_log(debug) << "BeamElectronLocator is using parameters: "
                  << " \n\tinput_collection = " << input_coll_
                  << " \n\tinput_pass_name = " << input_pass_name_
                  << " \n\toutput_collection = " << output_coll_
                  << " \n\tgranularity_X_mm = " << granularity_xmm_
                  << " \n\tgranularity_Y_mm = " << granularity_ymm_
                  << " \n\tmin_granularity_mm = " << tolerance_
                  << " \n\tmin_X_mm = " << min_xmm_
                  << " \n\tmax_X_mm = " << max_xmm_
                  << " \n\tmin_Y_mm = " << min_ymm_
                  << " \n\tmax_Y_mm = " << max_ymm_
                  << " \n\tverbose = " << verbose_;
}

void BeamElectronLocator::produce(framework::Event& event) {
  // Check if the input collection exists. If not,
  // don't bother processing the event.
  if (!event.exists(input_coll_, input_pass_name_)) {
    ldmx_log(fatal) << "Attemping to use non-existing input collection "
                    << input_coll_ << "_" << input_pass_name_
                    << " to locate electrons! Exiting.";
    return;
  }

  std::vector<ldmx::BeamElectronTruth> beam_electron_info;
  const auto sim_hits{event.getCollection<ldmx::SimCalorimeterHit>(
      input_coll_, input_pass_name_)};

  if (verbose_) {
    ldmx_log(info) << "Looping through simhits in event "
                   << event.getEventNumber() << ".";
  }

  for (const auto& sim_hit : sim_hits) {
    // check if we already caught this position, else, add it
    bool is_matched = false;
    std::vector<float> pos = sim_hit.getPosition();
    for (auto found_electrons : beam_electron_info) {
      // this check makes it square rather than a dR circle
      if (fabs(pos[0] - found_electrons.getX()) < tolerance_ &&
          fabs(pos[1] - found_electrons.getY()) < tolerance_) {
        if (verbose_) {
          ldmx_log(debug) << "\tHit at (x_ = " << pos[0] << ", y_ = " << pos[1]
                          << " matches electron found at (x_ = "
                          << found_electrons.getX()
                          << ", y_ = " << found_electrons.getY()
                          << "); skip this simhit";
        }
        is_matched = true;
        break;  // finding a match means Move on
      }  // if coordinates match something we already found
    }  // over found electrons
    if (!is_matched) {
      if (verbose_) {
        ldmx_log(info) << "\tHit at (x_ = " << pos[0] << ", y_ = " << pos[1]
                       << " not formerly matched. Adding to collection.";
      }
      ldmx::BeamElectronTruth electron_info;
      electron_info.setXYZ(pos[0], pos[1], pos[2]);
      // find a way to do this later
      // electronInfo.setThreeMomentum(simHit.getPx(), simHit.getPy(),
      // simHit.getPz());

      electron_info.setBarX(bin(pos[0], granularity_xmm_, min_xmm_, max_xmm_));
      electron_info.setBarY(bin(pos[1], granularity_ymm_, min_ymm_, max_ymm_));
      // set coordinates to bin center
      electron_info.setBinnedX(min_xmm_ + (electron_info.getBarX() + 0.5) *
                                              granularity_xmm_);
      electron_info.setBinnedY(min_ymm_ + (electron_info.getBarY() + 0.5) *
                                              granularity_ymm_);

      beam_electron_info.push_back(electron_info);
    }
  }  // over simhits in the collection

  event.add(output_coll_, beam_electron_info);
}

int BeamElectronLocator::bin(float coordinate, double binWidth, double min,
                             double max) {
  int n = 0;
  while (coordinate > min + n * binWidth) {
    n++;
    if (min + n * binWidth > max) {
      // don't go out of bounds, but, still indicate overflow by increasing n
      // before breaking
      break;
    }
  }
  // the n we have now is the first bin beyond our coordinate.
  // better aligned with conventions to return the lower edge.
  return n - 1;
}

}  // namespace recon

DECLARE_PRODUCER(recon::BeamElectronLocator)
