/**
 * @file CascadeStep.h
 * @brief Data class representing a single step in the Bertini intranuclear
 * cascade
 */

#ifndef SIMCORE_EVENT_CASCADESTEP_H
#define SIMCORE_EVENT_CASCADESTEP_H

#include <vector>

#include "TObject.h"

namespace ldmx {

/**
 * @enum CascadeStage
 * @brief Classification of cascade particle stages
 */
enum class CascadeStage : int {
  UNKNOWN = 0,   ///< Unclassified
  INCIDENT = 1,  ///< The incident particle (generation 0)
  PRIMARY = 2,   ///< Direct products of initial photon-nucleon interaction
  CASCADE = 3,   ///< Products of subsequent intranuclear scattering
  PREEQUILIBRIUM = 4,  ///< Fast particles escaping before equilibration
  ABSORBED = 5,        ///< Particles absorbed by the nucleus
  SPECTATOR = 6,       ///< Knocked-out nucleons from quasi-deuteron breakup
  DEEXCITATION =
      7  ///< Products from nuclear de-excitation (evaporation, gamma)
};

/**
 * @class CascadeStep
 * @brief Single particle state in the Bertini intranuclear cascade
 *
 * Records PDG, 4-momentum, position, generation, zone, and parent/daughter IDs.
 * History IDs are unique within a cascade and encode the genealogy.
 */
class CascadeStep {
 public:
  CascadeStep() = default;
  virtual ~CascadeStep() = default;

  void clear();

  void setHistoryId(int id) { history_id_ = id; }
  void setParentId(int id) { parent_id_ = id; }
  void setPdgId(int id) { pdg_id_ = id; }
  void setMomentum(double px, double py, double pz, double e) {
    px_ = px;
    py_ = py;
    pz_ = pz;
    energy_ = e;
  }
  void setPosition(double x, double y, double z) {
    x_ = x;
    y_ = y;
    z_ = z;
  }
  void setGeneration(int gen) { generation_ = gen; }
  void setZone(int zone) { zone_ = zone; }
  void setDaughterIds(const std::vector<int>& ids) { daughter_ids_ = ids; }
  void addDaughterId(int id) { daughter_ids_.push_back(id); }
  void setTargetPdgId(int id) { target_pdg_id_ = id; }
  void setInteracted(bool interacted) { interacted_ = interacted; }
  void setEscaped(bool escaped) { escaped_ = escaped; }
  void setStage(CascadeStage stage) { stage_ = stage; }
  void setStage(int stage) { stage_ = static_cast<CascadeStage>(stage); }

  int getHistoryId() const { return history_id_; }
  int getParentId() const { return parent_id_; }
  int getPdgId() const { return pdg_id_; }
  double getPx() const { return px_; }
  double getPy() const { return py_; }
  double getPz() const { return pz_; }
  double getEnergy() const { return energy_; }
  double getX() const { return x_; }
  double getY() const { return y_; }
  double getZ() const { return z_; }
  int getGeneration() const { return generation_; }
  int getZone() const { return zone_; }
  const std::vector<int>& getDaughterIds() const { return daughter_ids_; }
  int getNumDaughters() const { return static_cast<int>(daughter_ids_.size()); }
  int getTargetPdgId() const { return target_pdg_id_; }
  bool didInteract() const { return interacted_; }
  bool didEscape() const { return escaped_; }
  CascadeStage getStage() const { return stage_; }
  int getStageInt() const { return static_cast<int>(stage_); }

  double getKineticEnergy() const;
  double getMass() const;

 private:
  int history_id_{-1};
  int parent_id_{-1};
  int pdg_id_{0};

  double px_{0};
  double py_{0};
  double pz_{0};
  double energy_{0};

  double x_{0};
  double y_{0};
  double z_{0};

  int generation_{0};
  int zone_{0};

  std::vector<int> daughter_ids_;
  int target_pdg_id_{0};

  bool interacted_{false};
  bool escaped_{false};

  CascadeStage stage_{CascadeStage::UNKNOWN};

  ClassDef(CascadeStep, 1);
};

}  // namespace ldmx

#endif  // SIMCORE_EVENT_CASCADESTEP_H
