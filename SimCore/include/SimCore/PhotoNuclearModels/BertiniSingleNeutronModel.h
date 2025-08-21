#ifndef SIMCORE_BERTINI_SINGLE_NEUTRON_MODEL_H
#define SIMCORE_BERTINI_SINGLE_NEUTRON_MODEL_H
#include <G4CrossSectionDataSetRegistry.hh>
#include <G4Gamma.hh>
#include <G4HadProjectile.hh>
#include <G4HadronInelasticProcess.hh>
#include <G4Nucleus.hh>
#include <G4ProcessManager.hh>

#include "Framework/Configure/Parameters.h"
#include "SimCore/PhotoNuclearModels/BertiniEventTopologyProcess.h"
#include "SimCore/PhotoNuclearModels/PhotoNuclearModel.h"
namespace simcore {
class BertiniSingleNeutronProcess : public BertiniEventTopologyProcess {
 public:
  BertiniSingleNeutronProcess(double threshold, int Zmin, double Emin,
                              bool count_light_ions)
      : BertiniEventTopologyProcess{count_light_ions},
        threshold_{threshold},
        zmin_{Zmin},
        emin_{Emin} {}
  virtual ~BertiniSingleNeutronProcess() = default;
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
};

class BertiniSingleNeutronModel : public PhotoNuclearModel {
 public:
  BertiniSingleNeutronModel(const std::string& name,
                            const framework::config::Parameters& parameters)
      : PhotoNuclearModel{name, parameters},
        threshold_{parameters.getParameter<double>("hard_particle_threshold")},
        zmin_{parameters.getParameter<int>("zmin")},
        emin_{parameters.getParameter<double>("emin")},
        count_light_ions_{parameters.getParameter<bool>("count_light_ions")} {}
  virtual ~BertiniSingleNeutronModel() = default;
  void constructGammaProcess(G4ProcessManager* processManager) ;

 private:
  double threshold_;
  int zmin_;
  double emin_;
  bool count_light_ions_;
};

}  // namespace simcore
#endif /* SIMCORE_BERTINI_SINGLE_NEUTRON_MODEL_H */
