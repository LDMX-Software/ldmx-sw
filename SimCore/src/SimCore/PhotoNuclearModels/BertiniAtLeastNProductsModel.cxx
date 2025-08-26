
#include "SimCore/PhotoNuclearModels/BertiniAtLeastNProductsModel.h"
namespace simcore {

bool BertiniAtLeastNProductsProcess::acceptEvent() const {
  int secondaries{theParticleChange.GetNumberOfSecondaries()};
  int matching_count{0};
  for (int i{0}; i < secondaries; ++i) {
    const auto secondary{theParticleChange.GetSecondary(i)->GetParticle()};
    const auto pdg_code{secondary->GetDefinition()->GetPDGEncoding()};
    const auto energy{secondary->GetKineticEnergy()};
    if (std::find(std::begin(pdg_ids_), std::end(pdg_ids_), pdg_code) !=
        std::end(pdg_ids_)) {
      if (energy > threshold_) {
        ++matching_count;
      }
    }
    if (matching_count >= min_products_) {
      return true;
    }
  }
  return false;
}

void BertiniAtLeastNProductsModel::constructGammaProcess(
    G4ProcessManager* processManager) {
  auto photo_nuclear_process{
      new G4HadronInelasticProcess("photonNuclear", G4Gamma::Definition())};
  auto model{new BertiniAtLeastNProductsProcess{threshold_, zmin_, emin_,
                                                pdg_ids_, min_products_}};
  model->SetMaxEnergy(15 * CLHEP::GeV);
  addPNCrossSectionData(photo_nuclear_process);
  photo_nuclear_process->RegisterMe(model);
  processManager->AddDiscreteProcess(photo_nuclear_process);
}
}  // namespace simcore

DECLARE_PHOTONUCLEAR_MODEL(simcore::BertiniAtLeastNProductsModel)
