#ifndef RECON_OVERLAYPRODUCER_H
#define RECON_OVERLAYPRODUCER_H

//---< C++ StdLib >---// 
#include <map>
#include <string>
#include <vector>

//---< ROOT >---//
#include "TFile.h"
#include "TRandom2.h"

//---< Framework >---//
#include "Framework/Configure/Parameters.h"
#include "Framework/EventFile.h"
#include "Framework/EventProcessor.h"

namespace recon {

/**
 * Class to overlay in-time pile-up events from an overlay file
 */
class OverlayProducer : public framework::Producer {
 public:
  OverlayProducer(const std::string &name, framework::Process &process)
      : framework::Producer(name, process), overlayEvent_{"overlay"} {}

  // Destructor
  ~OverlayProducer() = default;

  /**
   * Configure the processor with input parameters from the python cofig
   */
  void configure(framework::config::Parameters &parameters) final override;

  /**
   * At the start of the run, the pileup overlay file is set up, and the
   * starting event number is chosen, using the RNSS.
   */
  void onNewRun(const ldmx::RunHeader &);  // );    //

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
  void produce(framework::Event &event) final override;

  /**
   * At the start of processing, the pileup overlay file is set up.
   */
  void onProcessStart() final override;

 private:
  /// The parameters used to configure this producer
  framework::config::Parameters params_;

  /**
   * Pileup overlay events input file name
   */
  std::string overlayFileName_;

  /**
   * Pileup overlay events input file
   */
  std::unique_ptr<framework::EventFile> overlayFile_;

  /**
   * The overlay ldmx event bus
   */
  framework::Event overlayEvent_;

  /**
   * List of SimCalorimeterHit collection(s) to loop over and add hits from,
   * combining sim and pileup
   */
  std::vector<std::string> caloCollections_;

  /**
   * List of SimTrackerHit collection(s) to loop over and add hits from,
   * combining sim and pileup
   */
  std::vector<std::string> trackerCollections_;

  /**
   * Pileup overlay events input pass name
   */
  std::string overlayPassName_;

  /**
   * To use for finding the sim event bus passengers, mostly a disambiguation
   */
  std::string simPassName_;

  /**
   * Let the total number of in-time events be poisson distributed, or fix at
   * the chosen value, poissonMu_
   */
  bool doPoissonIT_{false};

  /**
   * Let the total number of out-of-time events be poisson distributed, or fix
   *  at the chosen value, poissonMu_
   */
  bool doPoissonOOT_{false};

  /**
   * (average) total number of events
   */
  double poissonMu_{0.};

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
  std::unique_ptr<TRandom2> rndmTime_;

  /**
   * Width of pileup bunch spread in time (in [ns]), specified as a sigma of a
   * Gaussian distribution
   */
  double timeSigma_{0.};

  /**
   * Average position in time (in [ns]) of pileup bunches, relative to the sim
   * event. Should realistically be 0. Using a non-zero mean and sigma = 0 is
   * however useful for validation.
   */
  double timeMean_{0.};

  /**
   * Spacing in time (in [ns]) between electron bunches.
   */
  double bunchSpacing_{0.};

  /**
   * Number of bunches before the sim event to pull pileup events
   * from. Defaults to 0 --> all events occur in the same bunch as the sim
   * event.
   */
  int nEarlier_{0};

  /**
   * Number of bunches after the sim event to pull pileup events
   * from. Defaults to 0 --> all events occur in the same bunch as the sim
   * event.
   */
  int nLater_{0};

  /**
   * Local control of processor verbosity
   */
  int verbosity_;

  /**
   * For Ecal, overlay hits should be added as contribs.
   * But these are required to be unique, by the Ecal rconstruction code.
   * So assign a nonsensical trackID, incidentID, and PDG ID to the contribs
   * from overlay. These are hardwired right here.
   */
  int overlayIncidentID_{-1000};
  int overlayTrackID_{-1000};
  int overlayPdgCode_{0};
};
}  // namespace recon

#endif /* RECON_OVERLAY_OVERLAYPRODUCER_H */
