#ifndef TRACKING_RECO_PEDESTALSUBTRACTOR_H_
#define TRACKING_RECO_PEDESTALSUBTRACTOR_H_

#include <set>
#include <string>

#include "Framework/EventProcessor.h"

namespace tracking::reco {

/**
 * Subtract per-channel, per-sample pedestals from a RawSiStripHit collection.
 *
 * The pedestal means are obtained from the TrackerPedestals conditions object
 * (a service, provided by TrackerPedestalProvider) rather than read from a file
 * here.  On every event the per-sample mean is subtracted from each
 * RawSiStripHit and a new (pedestal-subtracted) RawSiStripHit collection is
 * written out; the raw and subtracted collections are distinguished only by
 * name.  No noise or other calibration constant is stored on the hits —
 * downstream consumers query the same conditions object when they need it.
 */
class PedestalSubtractor : public framework::Producer {
 public:
  PedestalSubtractor(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}

  void configure(framework::config::Parameters& ps) override;
  void produce(framework::Event& event) override;

 private:
  std::string input_collection_{"RawSiStripHits"};
  std::string input_pass_name_{""};
  std::string output_collection_{"TrackerHits"};

  std::set<std::string> warned_channels_;
};

}  // namespace tracking::reco

#endif  // TRACKING_RECO_PEDESTALSUBTRACTOR_H_
