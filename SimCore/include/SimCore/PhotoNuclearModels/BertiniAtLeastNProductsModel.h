#ifndef SIMCORE_BERTINI_AT_LEAST_N_PRODUCTS_MODEL_H
#define SIMCORE_BERTINI_AT_LEAST_N_PRODUCTS_MODEL_H
#include <G4CrossSectionDataSetRegistry.hh>
#include <G4Gamma.hh>
#include <G4HadProjectile.hh>
#include <G4HadronInelasticProcess.hh>
#include <G4Nucleus.hh>
#include <G4ProcessManager.hh>

#include "Framework/Configure/Parameters.h"
#include "SimCore/PhotoNuclearModels/BertiniEventTopologyProcess.h" /*  */
#include "SimCore/PhotoNuclearModels/PhotoNuclearModel.h"
namespace simcore {
class BertiniAtLeastNProductsProcess : public BertiniEventTopologyProcess {
 public:
  BertiniAtLeastNProductsProcess(double threshold, int Zmin, double Emin,
                                 const std::vector<int>& pdg_ids,
                                 const std::vector<int>& n_products_vec,
                                 bool per_species, bool exact_count)
      : BertiniEventTopologyProcess{},
        threshold_{threshold},
        Zmin_{Zmin},
        Emin_{Emin},
        pdg_ids_{pdg_ids},
        n_products_vec_{n_products_vec},
        per_species_{per_species},
        exact_count_{exact_count} {}

  virtual ~BertiniAtLeastNProductsProcess() = default;
  bool acceptProjectile(const G4HadProjectile& projectile) const override {
    return projectile.GetKineticEnergy() >= Emin_;
  }
  bool acceptTarget(const G4Nucleus& targetNucleus) const override {
    return targetNucleus.GetZ_asInt() >= Zmin_;
  }
  bool acceptEvent() const override;

 private:
  double threshold_;
  int Zmin_;
  double Emin_;
  std::vector<int> pdg_ids_;
  std::vector<int> n_products_vec_;
  bool per_species_;
  bool exact_count_;
};

class BertiniAtLeastNProductsModel : public PhotoNuclearModel {
 public:
  BertiniAtLeastNProductsModel(const std::string& name,
                               const framework::config::Parameters& parameters)
      : PhotoNuclearModel{name, parameters},
        threshold_{parameters.getParameter<double>("hard_particle_threshold")},
        Zmin_{parameters.getParameter<int>("zmin")},
        Emin_{parameters.getParameter<double>("emin")},
        pdg_ids_{parameters.getParameter<std::vector<int>>("pdg_ids")},
        per_species_{parameters.getParameter<bool>("per_species", false)} {
    if (per_species_) {
      n_products_vec_ =
          parameters.getParameter<std::vector<int>>("n_products_by_species");
    } else {
      int n = parameters.getParameter<int>("n_products", 1);
      n_products_vec_ = std::vector<int>{n};
    }
  }
  virtual ~BertiniAtLeastNProductsModel() = default;
  void ConstructGammaProcess(G4ProcessManager* processManager) override;

 private:
  double threshold_{200.0 * CLHEP::MeV};
  int Zmin_{0};
  double Emin_{0.0};
  std::vector<int> pdg_ids_{};
  std::vector<int> n_products_vec_{};
  bool per_species_{false};
  bool exact_count_{false};
};

}  // namespace simcore
#endif /* SIMCORE_BERTINI_AT_LEAST_N_PRODUCTS_MODEL_H */
