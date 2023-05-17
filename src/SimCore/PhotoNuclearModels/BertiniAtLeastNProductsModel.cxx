
#include "SimCore/PhotoNuclearModels/BertiniAtLeastNProductsModel.h"
namespace simcore {

bool BertiniAtLeastNProductsProcess::acceptEvent() const {
  int secondaries{theParticleChange.GetNumberOfSecondaries()};
  int matchingCount{0};
  for (int i{0}; i < secondaries; ++i) {
    const auto secondary{theParticleChange.GetSecondary(i)->GetParticle()};
    const auto pdgCode{secondary->GetDefinition()->GetPDGEncoding()};
    const auto energy{secondary->GetKineticEnergy()};
    if (std::find(std::begin(pdg_ids_), std::end(pdg_ids_), pdgCode) !=
        std::end(pdg_ids_)) {
      if (energy > threshold_) {
        ++matchingCount;
      }
    }
    if (matchingCount >= min_products_) {
      return true;
    }
  }
  return false;
}

void BertiniAtLeastNProductsModel::ConstructGammaProcess(
    G4ProcessManager* processManager) {
  auto photoNuclearProcess{
      new G4HadronInelasticProcess("photonNuclear", G4Gamma::Definition())};
  auto model{new BertiniAtLeastNProductsProcess{threshold_, Zmin_, Emin_,
                                                pdg_ids_, min_products_}};
  model->SetMaxEnergy(15 * CLHEP::GeV);
  addPNCrossSectionData(photoNuclearProcess);
  photoNuclearProcess->RegisterMe(model);
  processManager->AddDiscreteProcess(photoNuclearProcess);
}
}  // namespace simcore

DECLARE_PHOTONUCLEAR_MODEL(simcore::BertiniAtLeastNProductsModel)
