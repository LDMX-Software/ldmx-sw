#ifndef SIMCORE_BERTINI_MODEL_H
#define SIMCORE_BERTINI_MODEL_H

#include <G4ProcessManager.hh>

#include "Framework/Configure/Parameters.h"
#include "SimCore/PhotoNuclearModels/PhotoNuclearModel.h"
namespace simcore {

/**
 * The default photonuclear model for LDMX simulations. Since the Bertini
 * G4HadronicInteraction comes registered by default, this model does not
 * construct any new process but does override the removeExistingModel function
 * to ensure that the existing process is not removed.
 *
 * It moves the Bertini to QGS transition of the existing process to 8-10 GeV
 * (Geant4: 3-6 GeV), so the GeV photons LDMX cares about stay in Bertini.
 */
class BertiniModel : public PhotoNuclearModel {
 public:
  BertiniModel(const std::string& name,
               const framework::config::Parameters& parameters)
      : PhotoNuclearModel{name, parameters} {}
  virtual ~BertiniModel() = default;
  /**
   * The Bertini process is already present, only set its energy ranges
   */
  void constructGammaProcess(G4ProcessManager* process_manager) override;
  /**
   * Do nothing, we don't want to remove the existing process.
   */
  void removeExistingModel(G4ProcessManager*) override {}
};

}  // namespace simcore

#endif /* SIMCORE_BERTINI_MODEL_H */
