#ifndef RECON_OVERLAYPRODUCER_H
#define RECON_OVERLAYPRODUCER_H

//---< C++ StdLib >---//
#include <algorithm>
#include <bitset>
#include <map>
#include <memory>
#include <string>
#include <vector>

//---< ROOT >---//
#include "TFile.h"
#include "TRandom2.h"

//---< Framework >---//
#include "Framework/Configure/Parameters.h"
#include "Framework/EventFile.h"
#include "Framework/EventProcessor.h"
#include "Framework/Exception/Exception.h"
#include "Framework/RandomNumberSeedService.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "SimCore/Event/SimTrackerHit.h"

namespace recon {

/**
 * Class to overlay in-time pile-up events from an overlay file
 */
class OverlayProducer : public framework::Producer {
 public:
  OverlayProducer(const std::string &name, framework::Process &process)
      : framework::Producer(name, process), overlay_event_{"overlay"} {}

  // Destructor
  virtual ~OverlayProducer() = default;

  /**
   * Configure the processor with input parameters from the python cofig
   */
  void configure(framework::config::Parameters &parameters) override;

  /**
   * At the start of the run, the pileup overlay file is set up, and the
   * starting event number is chosen, using the RNSS.
   */
  void onNewRun(const ldmx::RunHeader &) override;  // );    //

  /**
   * Based on the list of collections to overlay, and the desired number of
   * events, loop through all relevant collections and copy the sim event
   * (once), and then add the corresponding collection from the pileup overlay
   * file. The event loop is the outer loop, and the inner loop is over the list
   * of collections.
   *
   * The collections have to be specified separately as a list of
   * SimCalorimeterHit collections and a list of SimTrackerHit collections.
   *
   * The collection name is parsed for "Ecal" to be flagged as a collection
   * which needs overlay hits to be added as contribs. This is currently
   * hardwired.
   *
   * The resulting collections inherit the input collection name, with an
   * appended string "Overlay". This name is also currently hardwired.
   */
  void produce(framework::Event &event) override;

  /**
   * Encode track ID with overlay event information.
   *
   * Performs bitwise encoding on track ID integer with version and event
   * index.
   *
   * VERSION DESCRIPTION
   * - 0 : no encoding, leaves track_id alone
   * - 1 : four bit version (27-31) plus three bit event index (24-26)
   */
  int encodeTrack(int track_id, const unsigned int encoding_version,
                  const unsigned int event_index = 0);

  /**
   * At the start of processing, the pileup overlay file is set up.
   */
  void onProcessStart() override;

 private:
  /// The parameters used to configure this producer
  framework::config::Parameters params_;

  /**
   * Pileup overlay events input file name
   */
  std::string overlay_filename_;

  /**
   * Pileup overlay events input file
   */
  std::unique_ptr<framework::EventFile> overlay_file_;

  /**
   * The overlay ldmx event bus
   */
  framework::Event overlay_event_;

  /**
   * List of SimCalorimeterHit collection(s) to loop over and add hits from,
   * combining sim and pileup
   */
  std::vector<std::string> calo_collections_;

  /**
   * List of SimTrackerHit collection(s) to loop over and add hits from,
   * combining sim and pileup
   */
  std::vector<std::string> tracker_collections_;

  /**
   * List of SimParticle collection(s) to loop over and add hits from,
   * combining sim and pileup
   */
  std::vector<std::string> particle_collections_;

  /**
   * List of SimCalorimeterHit collections which keep track of hit contribs.
   */
  std::vector<std::string> contrib_collections_;

  /**
   * Pileup overlay events input pass name
   */
  std::string overlay_passname_;

  /**
   * To use for finding the sim event bus passengers, mostly a disambiguation
   */
  std::string sim_passname_;

  /**
   * Postfix to add to the collection name of the overlayed collections.
   * This is currently set to "Overlay".
   */
  std::string out_coll_postfix_;

  /**
   * Let the total number of in-time events be poisson distributed, or fix at
   * the chosen value, poissonMu_
   */
  bool do_poisson_in_time_{false};

  /**
   * Let the total number of out-of-time events be poisson distributed, or fix
   *  at the chosen value, poissonMu_
   */
  bool do_poisson_out_of_time_{false};

  /**
   * (average) total number of events
   */
  double poisson_mu_{0.};

  /**
   * Random number generator for number of overlaid events.
   * TRandom2 slightly (~10%) faster than TRandom3; shorter period but our input
   * files will have way shorter period anyway.
   */
  std::unique_ptr<TRandom2> rndm_;

  /**
   * Random number generator for pileup event time offset.
   * TRandom2 slightly (~10%) faster than TRandom3; shorter period but our input
   * files will have way shorter period anyway.
   */
  std::unique_ptr<TRandom2> rndm_time_;

  /**
   * Width of pileup bunch spread in time (in [ns]), specified as a sigma of a
   * Gaussian distribution
   */
  double time_sigma_{0.};

  /**
   * Average position in time (in [ns]) of pileup bunches, relative to the sim
   * event. Should realistically be 0. Using a non-zero mean and sigma = 0 is
   * however useful for validation.
   */
  double time_mean_{0.};

  /**
   * Spacing in time (in [ns]) between electron bunches.
   */
  double bunch_spacing_{0.};

  /**
   * Number of bunches before the sim event to pull pileup events
   * from. Defaults to 0 --> all events occur in the same bunch as the sim
   * event.
   */
  int n_earlier_{0};

  /**
   * Number of bunches after the sim event to pull pileup events
   * from. Defaults to 0 --> all events occur in the same bunch as the sim
   * event.
   */
  int n_later_{0};

  /**
   * Minimum event number to start overlaying from.
   * Inclusive.
   */
  int start_event_min_{1};

  /**
   * Maximum event number to start overlaying from.
   * Inclusive.
   */
  int start_event_max_{10000};

  /**
   * Track ID encoding scheme version. Version is stored
   * in first 4 bits after the sign bit in the track ID int
   * variables (i.e. bits 28-31)
   *
   * If this variable is left as the hardcoded default 0, then
   * the OverlayProducer will default to not managing track IDs,
   * instead setting all equal to nonsense values as was the case
   * before the introduction of this track ID management.
   */
  unsigned track_id_encoding_{0};
};
}  // namespace recon

#endif /* RECON_OVERLAY_OVERLAYPRODUCER_H */
