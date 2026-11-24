#include "SimCore/PhotoNuclearModels/BertiniModel.h"

#include "G4CascadeInterface.hh"
#include "G4HadronicProcess.hh"
#include "G4SystemOfUnits.hh"
#include "G4TheoFSGenerator.hh"

namespace simcore {

void BertiniModel::constructGammaProcess(G4ProcessManager* process_manager) {
  const auto processes{process_manager->GetProcessList()};
  for (int i{0}; i < processes->size(); ++i) {
    auto process{dynamic_cast<G4HadronicProcess*>((*processes)[i])};
    if (process == nullptr or process->GetProcessName() != "photonNuclear") {
      continue;
    }
    for (auto model : process->GetHadronicInteractionList()) {
      // Bertini up to 10 GeV, QGS from 8 GeV
      if (dynamic_cast<G4CascadeInterface*>(model)) {
        model->SetMaxEnergy(10. * GeV);
      } else if (dynamic_cast<G4TheoFSGenerator*>(model)) {
        model->SetMinEnergy(8. * GeV);
      }
    }
  }
}

}  // namespace simcore

DECLARE_PHOTONUCLEAR_MODEL(simcore::BertiniModel)
