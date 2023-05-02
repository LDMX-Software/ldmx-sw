#ifndef SIMCORE_BERTINI_MODEL_H
#define SIMCORE_BERTINI_MODEL_H

#include <G4ProcessManager.hh>

#include "Framework/Configure/Parameters.h"
#include "SimCore/PhotoNuclearModel.h"
namespace simcore {

/**
 * The default photonuclear model for LDMX simulations. Since the Bertini
 * G4HadronicInteraction comes registered by default, this model does not
 * construct any new process but does override the removeExistingModel function
 * to ensure that the existing process is not removed.
 */
class BertiniModel : public PhotoNuclearModel {
 public:
  BertiniModel(const std::string& name,
               const framework::config::Parameters& parameters)
      : PhotoNuclearModel{name, parameters} {}
  virtual ~BertiniModel() = default;
  /**
   * Do nothing, the Bertini process is already present.
   */
  void ConstructGammaProcess(G4ProcessManager*) override {}
  /**
   * Do nothing, we don't want to remove the existing process.
   */
  void removeExistingModel(G4ProcessManager*) override {}
};

}  // namespace simcore

#endif /* SIMCORE_BERTINI_MODEL_H */
