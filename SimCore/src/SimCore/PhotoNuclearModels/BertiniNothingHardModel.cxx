#include "SimCore/PhotoNuclearModels/BertiniNothingHardModel.h"
namespace simcore {

bool BertiniNothingHardProcess::acceptEvent() const {
  int secondaries{theParticleChange.GetNumberOfSecondaries()};
  for (int i{0}; i < secondaries; ++i) {
    const auto secondary{theParticleChange.GetSecondary(i)->GetParticle()};
    const auto pdg_code{secondary->GetDefinition()->GetPDGEncoding()};
    if (skipCountingParticle(pdg_code)) {
      continue;
    }
    const auto energy{secondary->GetKineticEnergy()};
    if (energy > threshold_) {
      return false;
    }
  }
  return true;
}

void BertiniNothingHardModel::constructGammaProcess(
    G4ProcessManager* processManager) {
  auto photo_nuclear_process{
      new G4HadronInelasticProcess("photonNuclear", G4Gamma::Definition())};
  auto model{new BertiniNothingHardProcess{threshold_, Zmin_, Emin_,
                                           count_light_ions_}};
  model->SetMaxEnergy(15 * CLHEP::GeV);
  addPNCrossSectionData(photo_nuclear_process);
  photo_nuclear_process->RegisterMe(model);
  processManager->AddDiscreteProcess(photo_nuclear_process);
}
}  // namespace simcore

DECLARE_PHOTONUCLEAR_MODEL(simcore::BertiniNothingHardModel)
