/**
 * @file TrackerVetoProcessor.h
 * @brief Class that flags events if they pass the tracker veto
 * @author Tamas Almos Vami, UCSB
 */

#ifndef TRACKING_TRACKERVETOPROCESSOR_H_
#define TRACKING_TRACKERVETOPROCESSOR_H_

#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Tracking/Event/Track.h"
#include "Tracking/Event/TrackerVetoResult.h"

namespace tracking {

/**
 * @class TrackerVetoProcessor
 * @brief Flags events that pass the tracker veto by applying specific selection
 * criteria.
 *
 * This processor evaluates tracker events based on recoil and tagger track
 * properties, applying configurable selection criteria to determine whether an
 * event should be flagged.
 */
class TrackerVetoProcessor : public framework::Producer {
 public:
  /**
   * @brief Class constructor.
   * @param name Name of the processor.
   * @param process Reference to the framework process.
   */
  TrackerVetoProcessor(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}

  /**
   * @brief Class destructor.
   */
  virtual ~TrackerVetoProcessor() = default;

  /**
   * @brief Configure the processor using the given user-specified parameters.
   *
   * This function allows the user to set parameters such as track collection
   * names, selection thresholds, and filtering options.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters& parameters) override;

  /**
   * @brief Process an event and apply tracker veto selection criteria.
   *
   * This function checks the properties of tracks in the event and determines
   * whether it passes or fails the tracker veto conditions.
   *
   * @param event The event to run the checks on.
   */
  void produce(framework::Event& event) override;

 private:
  /** Maximum allowed transverse impact parameter (d0) for tracks. */
  float max_d0_;

  /** Maximum allowed longitudinal impact parameter (z0) for tracks. */
  float max_z0_;

  /** Max chi2/ndf required for tracks. */
  float max_chi2_per_ndf_;

  /** Minimum required momentum for tagger tracks. */
  float min_tagger_momentum_;

  /** Minimum number of recoil tracks required. */
  int min_recoil_n_;

  /** Maximum number of recoil tracks allowed. */
  int max_recoil_n_;

  /** Min number of hits for tagger tracks required. */
  int min_tagger_hits_;

  /** Min number of hits for recoil tracks required. */
  int min_recoil_hits_;

  /** The name of the tagger track collection. */
  std::string tagger_track_collection_name_;

  /** The name of the recoil track collection. */
  std::string recoil_track_collection_name_;

  /** The pass name of the input tagger collections. */
  std::string input_tagger_pass_name_;

  /** The pass name of the input recoil collections. */
  std::string input_recoil_pass_name_;

  /** Boolean flag to invert the selection criteria for skimming purposes. */
  bool inverse_skim_{false};

  //** Output collection name */
  std::string output_collection_;
};

}  // namespace tracking

#endif  // TRACKING_TRACKERVETOPROCESSOR_H_
