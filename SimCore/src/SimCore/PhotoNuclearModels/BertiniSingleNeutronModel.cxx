
#include "SimCore/PhotoNuclearModels/BertiniSingleNeutronModel.h"
namespace simcore {

bool BertiniSingleNeutronProcess::acceptEvent() const {
  int nhard{0};
  int nhard_neutron{0};
  int secondaries{theParticleChange.GetNumberOfSecondaries()};
  for (int i{0}; i < secondaries; ++i) {
    const auto secondary{theParticleChange.GetSecondary(i)->GetParticle()};
    const auto pdg_code{secondary->GetDefinition()->GetPDGEncoding()};
    if (skipCountingParticle(pdg_code)) {
      continue;
    }
    const auto energy{secondary->GetKineticEnergy()};
    if (energy > threshold_) {
      nhard++;
      if (pdg_code == 2112) {
        nhard_neutron++;
      }
    }
  }
  return nhard == 1 && nhard_neutron == 1;
}

void BertiniSingleNeutronModel::constructGammaProcess(
    G4ProcessManager* processManager) {
  auto photo_nuclear_process{
      new G4HadronInelasticProcess("photonNuclear", G4Gamma::Definition())};
  auto model{new BertiniSingleNeutronProcess{threshold_, Zmin_, Emin_,
                                             count_light_ions_}};
  model->SetMaxEnergy(15 * CLHEP::GeV);
  addPNCrossSectionData(photo_nuclear_process);
  photo_nuclear_process->RegisterMe(model);
  processManager->AddDiscreteProcess(photo_nuclear_process);
}
}  // namespace simcore

DECLARE_PHOTONUCLEAR_MODEL(simcore::BertiniSingleNeutronModel)
