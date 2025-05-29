
#include "SimCore/PhotoNuclearModels/BertiniAtLeastNProductsModel.h"
namespace simcore {

bool BertiniAtLeastNProductsProcess::acceptEvent() const {
  auto secondaries{theParticleChange.GetNumberOfSecondaries()};
  std::vector<int> counts(pdg_ids_.size(), 0);

  for (int i{0}; i < secondaries; ++i) {
    const auto* const secondary{
        theParticleChange.GetSecondary(i)->GetParticle()};
    const auto pdgCode{secondary->GetDefinition()->GetPDGEncoding()};
    const auto energy{secondary->GetKineticEnergy()};
    for (size_t j = 0; j < pdg_ids_.size(); ++j) {
      if (pdg_ids_[j] == pdgCode && energy > threshold_) {
        counts[j]++;
      }
    }
  }

  if (per_species_) {
    for (size_t j = 0; j < pdg_ids_.size(); ++j) {
      if (exact_count_) {
        if (counts[j] != n_products_vec_[j]) return false;
      } else {
        if (counts[j] < n_products_vec_[j]) return false;
      }
    }
    return true;
  }
  int total = 0;
  for (int count : counts) {
    total += count;
  }
  if (exact_count_) {
    return total == n_products_vec_[0];
  }
  return total >= n_products_vec_[0];
}

void BertiniAtLeastNProductsModel::ConstructGammaProcess(
    G4ProcessManager* processManager) {
  auto* photoNuclearProcess{
      new G4HadronInelasticProcess("photonNuclear", G4Gamma::Definition())};
  auto model = new BertiniAtLeastNProductsProcess{
      threshold_,      Zmin_,        Emin_,       pdg_ids_,
      n_products_vec_, per_species_, exact_count_};
  model->SetMaxEnergy(15 * CLHEP::GeV);
  addPNCrossSectionData(photoNuclearProcess);
  photoNuclearProcess->RegisterMe(model);
  processManager->AddDiscreteProcess(photoNuclearProcess);
}
}  // namespace simcore

DECLARE_PHOTONUCLEAR_MODEL(simcore::BertiniAtLeastNProductsModel)
