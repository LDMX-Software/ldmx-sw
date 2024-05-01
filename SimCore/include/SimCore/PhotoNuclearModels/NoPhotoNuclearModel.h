#ifndef NOPHOTONUCLEARMODEL_H
#define NOPHOTONUCLEARMODEL_H

#include <G4ProcessManager.hh>

#include "Framework/Configure/Parameters.h"
#include "SimCore/PhotoNuclearModel.h"

namespace simcore {

/**
 *
 * A photonuclear model that removes the photonNuclear process entirely. Be
 * careful when using that you aren't using any other part of ldmx-sw that
 * relies on the photonNuclear process being defined such as a biasing
 * operator for photonuclear reactions.
 **/
class NoPhotoNuclearModel : public PhotoNuclearModel {
 public:
  NoPhotoNuclearModel(const std::string& name,
                      const framework::config::Parameters& parameters)
      : PhotoNuclearModel{name, parameters} {}
  virtual ~NoPhotoNuclearModel() = default;
  void ConstructGammaProcess(G4ProcessManager* processManager) override;
};

}  // namespace simcore

#endif /* NOPHOTONUCLEARMODEL_H */
