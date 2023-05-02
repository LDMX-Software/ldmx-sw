#include "SimCore/PhotoNuclearModel.h"

namespace simcore {
void PhotoNuclearModel::removeExistingModel(G4ProcessManager* processManager) {
  const auto processes{processManager->GetProcessList()};
  for (int i{0}; i < processes->size(); ++i) {
    const auto process{(*processes)[i]};
    const auto processName{process->GetProcessName()};
    if (processName == "photonNuclear") {
      processManager->RemoveProcess(i);
    }
  }
}

void PhotoNuclearModel::addPNCrossSectionData(
    G4HadronInelasticProcess* process) const {
  auto crossSectionRegistry{G4CrossSectionDataSetRegistry::Instance()};
  auto crossSection{
      crossSectionRegistry->GetCrossSectionDataSet("PhotoNuclearXS")};
  if (!crossSection) {
    crossSection = new G4PhotoNuclearCrossSection{};
  }
  process->AddDataSet(crossSection);
}
}  // namespace simcore
