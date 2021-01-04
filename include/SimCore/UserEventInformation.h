#ifndef SIMCORE_USEREVENTINFORMATION_H
#define SIMCORE_USEREVENTINFORMATION_H

#include "G4VUserEventInformation.hh"

namespace ldmx {

/**
 * Encapsulates user defined information associated with a Geant4 event.
 */
class UserEventInformation : public G4VUserEventInformation {
 public:
  /// Constructor
  UserEventInformation();

  /// Destructor
  ~UserEventInformation();

  /// Print the information associated with the track
  void Print() const final override;

  /// Increment the number of brem candidates in an event.
  void incBremCandidateCount() { bremCandidateCount_ += 1; }

  /// Decrease the number of brem candidates in an event.
  void decBremCandidateCount() { bremCandidateCount_ -= 1; }

  /**
   * Set the event weight.
   *
   * @param[in] weight the event weight
   */
  void setWeight(double weight) { weight_ = weight; }

  /**
   * @return The event weight
   */
  double getWeight() { return weight_; }

  /**
   * @return The total number of brem candidates that this event
   *      contains.
   */
  int bremCandidateCount() { return bremCandidateCount_; }

 private:
  /// Total number of brem candidates in the event
  int bremCandidateCount_{0};

  /// The event weight
  double weight_{1.};
};
}  // namespace ldmx

#endif  // SIMCORE_USEREVENTINFORMATION_H
