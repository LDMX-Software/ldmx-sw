/**
 * @file CascadeHistory.h
 * @brief Container for the complete history of a Bertini intranuclear cascade
 */

#ifndef SIMCORE_EVENT_CASCADEHISTORY_H
#define SIMCORE_EVENT_CASCADEHISTORY_H

#include <vector>

#include "SimCore/Event/CascadeStep.h"
#include "TObject.h"

namespace ldmx {

/**
 * @class CascadeHistory
 * @brief All CascadeSteps from a single photonuclear interaction
 *
 * Keyed by the initiating photon's track ID for correlation with SimParticle.
 * Parent-daughter relationships are encoded via history IDs (parentId = -1
 * for incident particle).
 */
class CascadeHistory {
 public:
  CascadeHistory() = default;
  virtual ~CascadeHistory() = default;

  void clear();

  void setIncidentTrackId(int trackId) { incident_track_id_ = trackId; }
  void setTargetNucleus(int a, int z) {
    target_a_ = a;
    target_z_ = z;
  }
  void setIncidentEnergy(double energy) { incident_energy_ = energy; }
  void setExcitationEnergy(double energy) { excitation_energy_ = energy; }
  void setResidualNucleus(int a, int z) {
    residual_a_ = a;
    residual_z_ = z;
  }

  void addStep(const CascadeStep& step) { steps_.push_back(step); }
  void addStep(CascadeStep&& step) { steps_.push_back(std::move(step)); }
  void reserve(size_t n) { steps_.reserve(n); }

  int getIncidentTrackId() const { return incident_track_id_; }
  int getTargetA() const { return target_a_; }
  int getTargetZ() const { return target_z_; }
  double getIncidentEnergy() const { return incident_energy_; }
  double getExcitationEnergy() const { return excitation_energy_; }
  int getResidualA() const { return residual_a_; }
  int getResidualZ() const { return residual_z_; }

  size_t getNumSteps() const { return steps_.size(); }
  bool empty() const { return steps_.empty(); }
  const std::vector<CascadeStep>& getSteps() const { return steps_; }
  std::vector<CascadeStep>& getSteps() { return steps_; }
  const CascadeStep& getStep(size_t i) const { return steps_.at(i); }
  const CascadeStep* getStepByHistoryId(int historyId) const;

  // Analysis helpers
  const CascadeStep* getIncidentStep() const;
  std::vector<const CascadeStep*> getStepsAtGeneration(int generation) const;
  std::vector<const CascadeStep*> getInteractingSteps() const;
  std::vector<const CascadeStep*> getEscapedSteps() const;
  int getMaxGeneration() const;
  int getNumInteractions() const;
  void print() const;

 private:
  int incident_track_id_{-1};
  int target_a_{0};
  int target_z_{0};
  double incident_energy_{0.0};    // [MeV]
  double excitation_energy_{0.0};  // [MeV]
  int residual_a_{0};
  int residual_z_{0};
  std::vector<CascadeStep> steps_;

  ClassDef(CascadeHistory, 1);
};

}  // namespace ldmx

#endif  // SIMCORE_EVENT_CASCADEHISTORY_H
