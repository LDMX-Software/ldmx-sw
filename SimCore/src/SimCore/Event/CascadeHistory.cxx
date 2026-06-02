/**
 * @file CascadeHistory.cxx
 */

#include "SimCore/Event/CascadeHistory.h"

#include <iostream>

namespace ldmx {

void CascadeHistory::clear() {
  incident_track_id_ = -1;
  target_a_ = 0;
  target_z_ = 0;
  steps_.clear();
}

const CascadeStep* CascadeHistory::getStepByHistoryId(int historyId) const {
  for (const auto& step : steps_) {
    if (step.getHistoryId() == historyId) {
      return &step;
    }
  }
  return nullptr;
}

const CascadeStep* CascadeHistory::getIncidentStep() const {
  for (const auto& step : steps_) {
    if (step.getGeneration() == 0 && step.getParentId() == -1) {
      return &step;
    }
  }
  return steps_.empty() ? nullptr : &steps_.front();
}

std::vector<const CascadeStep*> CascadeHistory::getStepsAtGeneration(
    int generation) const {
  std::vector<const CascadeStep*> result;
  for (const auto& step : steps_) {
    if (step.getGeneration() == generation) {
      result.push_back(&step);
    }
  }
  return result;
}

std::vector<const CascadeStep*> CascadeHistory::getInteractingSteps() const {
  std::vector<const CascadeStep*> result;
  for (const auto& step : steps_) {
    if (step.didInteract()) {
      result.push_back(&step);
    }
  }
  return result;
}

std::vector<const CascadeStep*> CascadeHistory::getEscapedSteps() const {
  std::vector<const CascadeStep*> result;
  for (const auto& step : steps_) {
    if (step.didEscape()) {
      result.push_back(&step);
    }
  }
  return result;
}

int CascadeHistory::getMaxGeneration() const {
  int max_gen = -1;
  for (const auto& step : steps_) {
    if (step.getGeneration() > max_gen) {
      max_gen = step.getGeneration();
    }
  }
  return max_gen;
}

int CascadeHistory::getNumInteractions() const {
  int count = 0;
  for (const auto& step : steps_) {
    if (step.didInteract()) {
      ++count;
    }
  }
  return count;
}

void CascadeHistory::print() const {
  std::cout << "CascadeHistory: " << steps_.size() << " steps"
            << ", target A=" << target_a_ << " Z=" << target_z_
            << ", incident track=" << incident_track_id_ << "\n";

  for (const auto& step : steps_) {
    std::cout << "  [" << step.getHistoryId() << "] "
              << "PDG=" << step.getPdgId() << " gen=" << step.getGeneration()
              << " zone=" << step.getZone() << " E=" << step.getEnergy()
              << " MeV";
    if (step.getParentId() >= 0) {
      std::cout << " parent=" << step.getParentId();
    }
    if (step.didInteract()) {
      std::cout << " -> " << step.getNumDaughters() << " daughters";
    }
    if (step.didEscape()) {
      std::cout << " [escaped]";
    }
    std::cout << "\n";
  }
}

}  // namespace ldmx
