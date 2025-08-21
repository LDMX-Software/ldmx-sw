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
                                 std::vector<int> pdg_ids, int min_products)
      : BertiniEventTopologyProcess{},
        threshold_{threshold},
        zmin_{Zmin},
        emin_{Emin},
        pdg_ids_{pdg_ids},
        min_products_{min_products} {}
  virtual ~BertiniAtLeastNProductsProcess() = default;
  bool acceptProjectile(const G4HadProjectile& projectile) const override {
    return projectile.GetKineticEnergy() >= emin_;
  }
  bool acceptTarget(const G4Nucleus& targetNucleus) const override {
    return targetNucleus.GetZ_asInt() >= zmin_;
  }
  bool acceptEvent() const override;

 private:
  double threshold_;
  int zmin_;
  double emin_;
  std::vector<int> pdg_ids_;
  int min_products_;
};

class BertiniAtLeastNProductsModel : public PhotoNuclearModel {
 public:
  BertiniAtLeastNProductsModel(const std::string& name,
                               const framework::config::Parameters& parameters)
      : PhotoNuclearModel{name, parameters},
        threshold_{parameters.getParameter<double>("hard_particle_threshold")},
        zmin_{parameters.getParameter<int>("zmin")},
        emin_{parameters.getParameter<double>("emin")},
        pdg_ids_{parameters.getParameter<std::vector<int>>("pdg_ids")},
        min_products_{parameters.getParameter<int>("min_products")} {}
  virtual ~BertiniAtLeastNProductsModel() = default;
  void constructGammaProcess(G4ProcessManager* processManager) ;

 private:
  double threshold_;
  int zmin_;
  double emin_;
  std::vector<int> pdg_ids_;
  int min_products_;
};

}  // namespace simcore
#endif /* SIMCORE_BERTINI_AT_LEAST_N_PRODUCTS_MODEL_H */
