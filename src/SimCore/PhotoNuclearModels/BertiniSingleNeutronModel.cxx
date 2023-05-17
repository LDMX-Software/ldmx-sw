
#include "SimCore/PhotoNuclearModels/BertiniSingleNeutronModel.h"
namespace simcore {

bool BertiniSingleNeutronProcess::acceptEvent() const {
  int Nhard{0};
  int Nhard_neutron{0};
  int secondaries{theParticleChange.GetNumberOfSecondaries()};
  for (int i{0}; i < secondaries; ++i) {
    const auto secondary{theParticleChange.GetSecondary(i)->GetParticle()};
    const auto pdgCode{secondary->GetDefinition()->GetPDGEncoding()};
    if (skipCountingParticle(pdgCode)) {
      continue;
    }
    const auto energy{secondary->GetKineticEnergy()};
    if (energy > threshold_) {
      Nhard++;
      if (pdgCode == 2112) {
        Nhard_neutron++;
      }
    }
  }
  return Nhard == 1 && Nhard_neutron == 1;
}

void BertiniSingleNeutronModel::ConstructGammaProcess(
    G4ProcessManager* processManager) {
  auto photoNuclearProcess{
      new G4HadronInelasticProcess("photonNuclear", G4Gamma::Definition())};
  auto model{new BertiniSingleNeutronProcess{threshold_, Zmin_, Emin_,
                                             count_light_ions_}};
  model->SetMaxEnergy(15 * CLHEP::GeV);
  addPNCrossSectionData(photoNuclearProcess);
  photoNuclearProcess->RegisterMe(model);
  processManager->AddDiscreteProcess(photoNuclearProcess);
}
}  // namespace simcore

DECLARE_PHOTONUCLEAR_MODEL(simcore::BertiniSingleNeutronModel)
