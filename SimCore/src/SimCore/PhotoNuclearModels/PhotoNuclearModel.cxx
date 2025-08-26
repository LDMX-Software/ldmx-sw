#include "SimCore/PhotoNuclearModels/PhotoNuclearModel.h"

namespace simcore {
void PhotoNuclearModel::removeExistingModel(G4ProcessManager* processManager) {
  const auto processes{processManager->GetProcessList()};
  for (int i{0}; i < processes->size(); ++i) {
    const auto process{(*processes)[i]};
    const auto process_name{process->GetProcessName()};
    if (process_name == "photonNuclear") {
      processManager->RemoveProcess(i);
    }
  }
}

void PhotoNuclearModel::addPNCrossSectionData(
    G4HadronInelasticProcess* process) const {
  auto cross_section_registry{G4CrossSectionDataSetRegistry::Instance()};
  auto cross_section{
      cross_section_registry->GetCrossSectionDataSet("PhotoNuclearXS")};
  if (!cross_section) {
    cross_section = new G4PhotoNuclearCrossSection{};
  }
  process->AddDataSet(cross_section);
}

DEFINE_FACTORY(PhotoNuclearModel);

}  // namespace simcore
