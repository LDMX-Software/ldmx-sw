#include "SimCore/PhotoNuclearModels/BertiniExactlyNProductsModel.h"

#include <numeric>

namespace simcore {

bool BertiniExactlyNProductsProcess::acceptEvent() const {
  int secondaries{theParticleChange.GetNumberOfSecondaries()};
  int n_hard{0};
  std::vector<int> matching_count(pdg_ids_.size(), 0);

  for (int i{0}; i < secondaries; ++i) {
    const auto secondary{theParticleChange.GetSecondary(i)->GetParticle()};
    const auto pdgCode{secondary->GetDefinition()->GetPDGEncoding()};
    const auto energy{secondary->GetKineticEnergy()};

    if (energy > threshold_) {
      ++n_hard;
      for (size_t j{0}; j < matching_count.size(); ++j) {
        if (pdgCode == pdg_ids_[j]) ++matching_count[j];
      }
    }
  }

  int total_matched =
      std::accumulate(matching_count.begin(), matching_count.end(), 0);

  bool all_matched =
      std::find(std::begin(matching_count), std::end(matching_count), 0) ==
      std::end(matching_count);

  if (total_matched == n_products_ && n_hard == total_matched &&
      (!check_allmatch_ || all_matched)) {
    return true;
  }

  return false;
}

void BertiniExactlyNProductsModel::ConstructGammaProcess(
    G4ProcessManager* process_manager) {
  auto photo_nuclear_process{
      new G4HadronInelasticProcess("photonNuclear", G4Gamma::Definition())};

  auto model{new BertiniExactlyNProductsProcess{
      threshold_, zmin_, emin_, pdg_ids_, check_allmatch_, n_products_}};
  model->SetMaxEnergy(15 * CLHEP::GeV);
  addPNCrossSectionData(photo_nuclear_process);
  photo_nuclear_process->RegisterMe(model);
  process_manager->AddDiscreteProcess(photo_nuclear_process);
}
}  // namespace simcore

DECLARE_PHOTONUCLEAR_MODEL(simcore::BertiniExactlyNProductsModel)
