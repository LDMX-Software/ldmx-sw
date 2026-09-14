/**
 * @file TrackerVetoResult.h
 * @brief Class that holds reco-level tracking veto decision
 * @author Tamas Almos Vami, UCSB
 */

#ifndef TRACKER_EVENT_TRACKERVETORESULTS_H_
#define TRACKER_EVENT_TRACKERVETORESULTS_H_

// ROOT
#include <iostream>

#include "TObject.h"  //ClassDef

namespace ldmx {

/**
 * @class TrackerVetoResult
 * @brief Class that holds reco-level tracking veto decision
 */
class TrackerVetoResult {
 public:
  /**
   * Class constructor.
   */
  TrackerVetoResult() = default;

  /**
   * Class destructor.
   */
  virtual ~TrackerVetoResult() = default;

  /**
   * Reset the TrackerVetoResult object.
   */
  void clear();

  /**
   * Print the TrackerVetoResult object.
   */
  friend std::ostream& operator<<(std::ostream& o, const TrackerVetoResult& d);

  /** Checks if the event passes the Tracker veto. */
  bool passesVeto() const { return passes_veto_; };

  /** Checks if the event passes the Tagger Tracker veto. */
  bool passesTaggerVeto() const { return passes_tagger_veto_; };

  /** Checks if the event passes the Recoil Tracker veto. */
  bool passesRecoilVeto() const { return passes_recoil_veto_; };

  /**
   * Sets whether the Tracker veto was passed or not.
   *
   * @param passes_veto Veto result.
   */
  void setVetoResult(bool passes_veto) { passes_veto_ = passes_veto; }

  /**
   * Sets whether the Tagger Tracker veto was passed or not.
   *
   * @param passes_tagger_veto Tagger Veto result.
   */
  void setTaggerVetoResult(bool passes_tagger_veto) {
    passes_tagger_veto_ = passes_tagger_veto;
  }

  /**
   * Sets whether the Recoil Tracker veto was passed or not.
   *
   * @param passes_recoil_veto Recoil Veto result.
   */
  void setRecoilVetoResult(bool passes_recoil_veto) {
    passes_recoil_veto_ = passes_recoil_veto;
  }

 private:
  /* Boolean to store if the veto is passed */
  bool passes_veto_{false};

  bool passes_tagger_veto_{false};

  bool passes_recoil_veto_{false};

  ClassDef(TrackerVetoResult, 2);
};
}  // namespace ldmx

#endif
