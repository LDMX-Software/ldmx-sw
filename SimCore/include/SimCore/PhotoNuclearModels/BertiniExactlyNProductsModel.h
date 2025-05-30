#ifndef SIMCORE_BERTINI_EXACTLY_N_PRODUCTS_MODEL_H
#define SIMCORE_BERTINI_EXACTLY_N_PRODUCTS_MODEL_H
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
class BertiniExactlyNProductsProcess : public BertiniEventTopologyProcess {
 public:
  BertiniExactlyNProductsProcess(double threshold, int Zmin, double Emin,
                                 std::vector<int> pdg_ids, bool check_allmatch,
                                 int n_products)
      : BertiniEventTopologyProcess{},
        threshold_{threshold},
        Zmin_{Zmin},
        Emin_{Emin},
        pdg_ids_{pdg_ids},
        check_allmatch_{check_allmatch},
        n_products_{n_products} {}

  virtual ~BertiniExactlyNProductsProcess() = default;

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
  bool check_allmatch_;
  int n_products_;
};

class BertiniExactlyNProductsModel : public PhotoNuclearModel {
 public:
  BertiniExactlyNProductsModel(const std::string& name,
                               const framework::config::Parameters& parameters)
      : PhotoNuclearModel{name, parameters},
        threshold_{parameters.getParameter<double>("hard_particle_threshold")},
        Zmin_{parameters.getParameter<int>("zmin")},
        Emin_{parameters.getParameter<double>("emin")},
        pdg_ids_{parameters.getParameter<std::vector<int>>("pdg_ids")},
        check_allmatch_{parameters.getParameter<bool>("check_allmatch")},
        n_products_{parameters.getParameter<int>("n_products")} {}
  virtual ~BertiniExactlyNProductsModel() = default;
  void ConstructGammaProcess(G4ProcessManager* processManager) override;

 private:
  double threshold_;
  int Zmin_;
  double Emin_;
  std::vector<int> pdg_ids_;
  bool check_allmatch_;
  int n_products_;
};

}  // namespace simcore
#endif /* SIMCORE_BERTINI_AT_LEAST_N_PRODUCTS_MODEL_H */
