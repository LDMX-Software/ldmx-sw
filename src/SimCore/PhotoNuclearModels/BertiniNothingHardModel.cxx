#include "SimCore/PhotoNuclearModels/BertiniNothingHardModel.h"
namespace simcore {

bool BertiniNothingHardProcess::acceptEvent() const {
  int secondaries{theParticleChange.GetNumberOfSecondaries()};
  for (int i{0}; i < secondaries; ++i) {
    const auto secondary{theParticleChange.GetSecondary(i)->GetParticle()};
    const auto pdgCode{secondary->GetDefinition()->GetPDGEncoding()};
    if (skipCountingParticle(pdgCode)) {
      continue;
    }
    const auto energy{secondary->GetKineticEnergy()};
    if (energy > threshold_) {
      return false;
    }
  }
  return true;
}

void BertiniNothingHardModel::ConstructGammaProcess(
    G4ProcessManager* processManager) {
  auto photoNuclearProcess{
      new G4HadronInelasticProcess("photonNuclear", G4Gamma::Definition())};
  auto model{new BertiniNothingHardProcess{threshold_, Zmin_, Emin_,
                                           count_light_ions_}};
  model->SetMaxEnergy(15 * CLHEP::GeV);
  addPNCrossSectionData(photoNuclearProcess);
  photoNuclearProcess->RegisterMe(model);
  processManager->AddDiscreteProcess(photoNuclearProcess);
}
}  // namespace simcore

DECLARE_PHOTONUCLEAR_MODEL(simcore::BertiniNothingHardModel)
