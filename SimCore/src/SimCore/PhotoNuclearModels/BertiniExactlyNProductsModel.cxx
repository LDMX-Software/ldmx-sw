
#include "SimCore/PhotoNuclearModels/BertiniExactlyNProductsModel.h"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"

namespace simcore {

  bool BertiniExactlyNProductsProcess::acceptEvent() const {

    G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
    G4ParticleDefinition* neutrino = particleTable->FindParticle("nu_E"); 

    int secondaries{theParticleChange.GetNumberOfSecondaries()};
    int matching_count{0},n_hard{0};
    for (int i{0}; i < secondaries; ++i) {
      const auto secondary{theParticleChange.GetSecondary(i)->GetParticle()};
      const auto pdgCode{secondary->GetDefinition()->GetPDGEncoding()};
      const auto energy{secondary->GetKineticEnergy()};

      if (energy > threshold_) {
	++n_hard;
        if (std::find(std::begin(pdg_ids_), std::end(pdg_ids_), pdgCode) !=
            std::end(pdg_ids_)) {
            ++matching_count;
        }
      }
    }

    if (matching_count == n_products_ && n_hard == matching_count ) {
      return true;
    }

    return false;
  }

  void BertiniExactlyNProductsModel::ConstructGammaProcess(
      G4ProcessManager* processManager) {

    auto photoNuclearProcess{
	new G4HadronInelasticProcess("photonNuclear", G4Gamma::Definition())};

    auto model{new BertiniExactlyNProductsProcess{threshold_, Zmin_, Emin_,
                                                  pdg_ids_, n_products_}};
    model->SetMaxEnergy(15 * CLHEP::GeV);
    addPNCrossSectionData(photoNuclearProcess);
    photoNuclearProcess->RegisterMe(model);
    processManager->AddDiscreteProcess(photoNuclearProcess);
  }
}  // namespace simcore

DECLARE_PHOTONUCLEAR_MODEL(simcore::BertiniExactlyNProductsModel)
