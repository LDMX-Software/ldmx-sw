#ifndef SIMCORE_BERTINI_NOTHING_HARD_MODEL_H
#define SIMCORE_BERTINI_NOTHING_HARD_MODEL_H
#include "Framework/Configure/Parameters.h"
#include "G4CrossSectionDataSetRegistry.hh"
#include "G4Gamma.hh"
#include "G4HadProjectile.hh"
#include "G4HadronInelasticProcess.hh"
#include "G4Nucleus.hh"
#include "G4PhotoNuclearCrossSection.hh"
#include "G4ProcessManager.hh"
#include "SimCore/PhotonuclearModel.h"
#include "SimCore/PhotonuclearModels/BertiniEventTopologyProcess.h"
namespace simcore {

class BertiniNothingHardProcess : public BertiniEventTopologyProcess {
 public:
  BertiniNothingHardProcess(double threshold, int Zmin, double Emin)
      : BertiniEventTopologyProcess{},
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

class BertiniNothingHardModel : public PhotonuclearModel {
 public:
  BertiniNothingHardModel(const std::string& name,
                          const framework::config::Parameters& parameters)
      : PhotonuclearModel{name, parameters},
        threshold_{parameters.getParameter<double>("hard_particle_threshold")},
        Zmin_{parameters.getParameter<int>("zmin")},
        Emin_{parameters.getParameter<double>("emin")} {}
  virtual ~BertiniNothingHardModel() = default;
  void ConstructGammaProcess(G4ProcessManager* processManager) override;

 private:
  double threshold_;
  int Zmin_;
  double Emin_;
};
}  // namespace simcore
#endif /* SIMCORE_BERTINI_NOTHING_HARD_MODEL_H */
