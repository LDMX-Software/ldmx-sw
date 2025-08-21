#include "SimCore/PhotoNuclearModels/NoPhotoNuclearModel.h"

namespace simcore {

void NoPhotoNuclearModel::constructGammaProcess(
    G4ProcessManager* processManager) {
  // Do nothing
}
}  // namespace simcore

DECLARE_PHOTONUCLEAR_MODEL(simcore::NoPhotoNuclearModel);
