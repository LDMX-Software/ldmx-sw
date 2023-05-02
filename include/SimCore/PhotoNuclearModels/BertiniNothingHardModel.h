#ifndef SIMCORE_BERTINI_NOTHING_HARD_MODEL_H
#define SIMCORE_BERTINI_NOTHING_HARD_MODEL_H
#include <G4CrossSectionDataSetRegistry.hh>
#include <G4Gamma.hh>
#include <G4HadProjectile.hh>
#include <G4HadronInelasticProcess.hh>
#include <G4Nucleus.hh>
#include <G4PhotoNuclearCrossSection.hh>
#include <G4ProcessManager.hh>

#include "Framework/Configure/Parameters.h"
#include "SimCore/PhotoNuclearModel.h"
#include "SimCore/PhotoNuclearModels/BertiniEventTopologyProcess.h"
namespace simcore {

class BertiniNothingHardProcess : public BertiniEventTopologyProcess {
 public:
  BertiniNothingHardProcess(double threshold, int Zmin, double Emin,
                            bool count_light_ions)
      : BertiniEventTopologyProcess{count_light_ions},
        threshold_{threshold},
        Zmin_{Zmin},
        Emin_{Emin} {}
  virtual ~BertiniNothingHardProcess() = default;
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
};

class BertiniNothingHardModel : public PhotoNuclearModel {
 public:
  BertiniNothingHardModel(const std::string& name,
                          const framework::config::Parameters& parameters)
      : PhotoNuclearModel{name, parameters},
        threshold_{parameters.getParameter<double>("hard_particle_threshold")},
        Zmin_{parameters.getParameter<int>("zmin")},
        Emin_{parameters.getParameter<double>("emin")},
        count_light_ions_{parameters.getParameter<bool>("count_light_ions")} {}
  virtual ~BertiniNothingHardModel() = default;
  void ConstructGammaProcess(G4ProcessManager* processManager) override;

 private:
  double threshold_;
  int Zmin_;
  double Emin_;
  bool count_light_ions_;
};
}  // namespace simcore
#endif /* SIMCORE_BERTINI_NOTHING_HARD_MODEL_H */
