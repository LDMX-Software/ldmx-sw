#include "SimCore/PhotoNuclearModels/NoPhotoNuclearModel.h"

namespace simcore {

void NoPhotoNuclearModel::ConstructGammaProcess(
    G4ProcessManager* processManager) {
  // Do nothing
}
}  // namespace simcore

DECLARE_PHOTONUCLEAR_MODEL(simcore::NoPhotoNuclearModel);
