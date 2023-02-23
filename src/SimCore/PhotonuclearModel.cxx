#include "SimCore/PhotonuclearModel.h"

namespace simcore {
void PhotonuclearModel::removeExistingModel(G4ProcessManager* processManager) {
  const auto processes{processManager->GetProcessList()};
  for (int i{0}; i < processes->size(); ++i) {
    const auto process{(*processes)[i]};
    const auto processName{process->GetProcessName()};
    if (processName == "photonNuclear") {
      processManager->RemoveProcess(i);
    }
  }
}
}  // namespace simcore
