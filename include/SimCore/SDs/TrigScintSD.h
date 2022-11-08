#ifndef SIMCORE_TRIGSD_H
#define SIMCORE_TRIGSD_H

#include "SimCore/SensitiveDetector.h"
#include "SimCore/Event/SimCalorimeterHit.h"

namespace simcore {

/**
 * Class defining a sensitive detector of type trigger scintillator.
 */
class TrigScintSD : public SensitiveDetector {
 public:
  /**
   * Class constructor.
   *
   * @param[in] name The name of the sensitive detector.
   * @param[in] ci interface to conditions objects
   * @param[in] p python configuration parameters
   */
  TrigScintSD(const std::string& name,
              simcore::ConditionsInterface& ci,
              const framework::config::Parameters& p);

  /// Destructor
  ~TrigScintSD();

  /**
   * Should the input logical volume be included
   * in this sensitive detector?
   *
   * @note Depends on names in GDML!
   *
   * In order to avoid attaching to both 'target' and 'sp_target',
   * we intentionally exclude volumes with the string 'sp_' in 
   * their names.
   */
  virtual bool isSensDet(G4LogicalVolume* vol) const final override {
    return vol->GetName().contains(vol_name_) and not vol->GetName().contains("sp_");
  }

  /**
   * Process steps to create hits.
   *
   * @param[in] step The step information.
   * @param[in] history The readout history.
   */
  G4bool ProcessHits(G4Step* step, G4TouchableHistory* history) final override;

  /**
   * Save our hits collection into the event bus and reset it.
   */
  virtual void saveHits(framework::Event& event) final override {
    event.add(collection_name_, hits_);
  }

  virtual void EndOfEvent() final override {
    hits_.clear();
  }

 private:
  /// our collection of hits in this SD
  std::vector<ldmx::SimCalorimeterHit> hits_;
  /// name of the hit collection for this SD
  std::string collection_name_;
  /// name of trigger pad volume this SD is capturing
  std::string vol_name_;
  /// the ID number for the module we are gathering hits from
  int module_id_;

};

}  // namespace simcore

#endif
