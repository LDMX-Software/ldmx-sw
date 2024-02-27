#include "Recon/BeamElectronLocator.h"

namespace recon {

BeamElectronLocator::BeamElectronLocator(const std::string &name,
                                         framework::Process &process)
    : framework::Producer(name, process) {}

BeamElectronLocator::~BeamElectronLocator() {}

void BeamElectronLocator::configure(framework::config::Parameters &parameters) {
  inputColl_ = parameters.getParameter<std::string>("input_collection");
  inputPassName_ = parameters.getParameter<std::string>("input_pass_name");
  outputColl_ = parameters.getParameter<std::string>("output_collection");
  granularityXmm_ = parameters.getParameter<double>("granularity_X_mm");
  granularityYmm_ = parameters.getParameter<double>("granularity_Y_mm");
  tolerance_ = parameters.getParameter<double>("min_granularity_mm");
  minXmm_ = parameters.getParameter<double>("min_X_mm");
  maxXmm_ = parameters.getParameter<double>("max_X_mm");
  minYmm_ = parameters.getParameter<double>("min_Y_mm");
  maxYmm_ = parameters.getParameter<double>("max_Y_mm");
  verbose_ = parameters.getParameter<bool>("verbose");
}
void BeamElectronLocator::onProcessStart() {
  ldmx_log(debug) << "BeamElectronLocator is using parameters: "
                  << " \n\tinput_collection = " << inputColl_
                  << " \n\tinput_pass_name = " << inputPassName_
                  << " \n\toutput_collection = " << outputColl_
                  << " \n\tgranularity_X_mm = " << granularityXmm_
                  << " \n\tgranularity_Y_mm = " << granularityYmm_
                  << " \n\tmin_granularity_mm = " << tolerance_
                  << " \n\tmin_X_mm = " << minXmm_
                  << " \n\tmax_X_mm = " << maxXmm_
                  << " \n\tmin_Y_mm = " << minYmm_
                  << " \n\tmax_Y_mm = " << maxYmm_
                  << " \n\tverbose = " << verbose_;
}

void BeamElectronLocator::produce(framework::Event &event) {
  // Check if the input collection exists. If not,
  // don't bother processing the event.
  if (!event.exists(inputColl_, inputPassName_)) {
    ldmx_log(fatal) << "Attemping to use non-existing input collection "
                    << inputColl_ << "_" << inputPassName_
                    << " to locate electrons! Exiting.";
    return;
  }

  std::vector<ldmx::BeamElectronTruth> beamElectronInfo;
  const auto simHits{
      event.getCollection<ldmx::SimCalorimeterHit>(inputColl_, inputPassName_)};

  if (verbose_) {
    ldmx_log(info) << "Looping through simhits in event "
                   << event.getEventNumber() << ".";
  }

  for (const auto &simHit : simHits) {
    // check if we already caught this position, else, add it
    bool isMatched = false;
    std::vector<float> pos = simHit.getPosition();
    for (auto foundElectrons : beamElectronInfo) {
      // this check makes it square rather than a dR circle
      if (fabs(pos[0] - foundElectrons.getX()) < tolerance_ &&
          fabs(pos[1] - foundElectrons.getY()) < tolerance_) {
        if (verbose_) {
          ldmx_log(debug) << "\tHit at (x = " << pos[0] << ", y = " << pos[1]
                          << " matches electron found at (x = "
                          << foundElectrons.getX()
                          << ", y = " << foundElectrons.getY()
                          << "); skip this simhit";
		}
        isMatched = true;
        break;  // finding a match means Move on
      }         // if coordinates match something we already found
    }           // over found electrons
    if (!isMatched) {
      if (verbose_) {
        ldmx_log(info) << "\tHit at (x = " << pos[0] << ", y = " << pos[1]
                       << " not formerly matched. Adding to collection.";
      }
      ldmx::BeamElectronTruth electronInfo;
      electronInfo.setXYZ(pos[0], pos[1], pos[2]);
      // find a way to do this later
      // electronInfo.setThreeMomentum(simHit.getPx(), simHit.getPy(),
      // simHit.getPz());

      electronInfo.setBarX(bin(pos[0], granularityXmm_, minXmm_, maxXmm_));
      electronInfo.setBarY(bin(pos[1], granularityYmm_, minYmm_, maxYmm_));
      // set coordinates to bin center
      electronInfo.setBinnedX(minXmm_ + (electronInfo.getBarX() + 0.5) *
                                            granularityXmm_);
      electronInfo.setBinnedY(minYmm_ + (electronInfo.getBarY() + 0.5) *
                                            granularityYmm_);

      beamElectronInfo.push_back(electronInfo);
    }
  }  // over simhits in the collection

  event.add(outputColl_, beamElectronInfo);
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

DECLARE_PRODUCER_NS(recon, BeamElectronLocator)
