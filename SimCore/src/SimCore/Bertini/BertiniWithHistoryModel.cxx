/**
 * @file BertiniWithHistoryModel.cxx
 */

#include "SimCore/Bertini/BertiniWithHistoryModel.h"

#include <G4Gamma.hh>
#include <G4HadronInelasticProcess.hh>
#include <G4ProcessManager.hh>

#include "SimCore/Bertini/LDMXCascadeInterface.h"

namespace simcore {
namespace bertini {

BertiniWithHistoryModel::BertiniWithHistoryModel(
    const std::string& name, const framework::config::Parameters& parameters)
    : PhotoNuclearModel{name, parameters} {
  max_energy_ = parameters.getParameter<double>("max_energy", 15000.0);
  energy_threshold_ =
      parameters.getParameter<double>("energy_threshold", 5000.0);
}

void BertiniWithHistoryModel::constructGammaProcess(
    G4ProcessManager* processManager) {
  ldmx_log(info) << "BertiniWithHistoryModel::constructGammaProcess";
  ldmx_log(info) << "  Max energy: " << max_energy_ << " MeV";
  ldmx_log(info) << "  Energy threshold: " << energy_threshold_ << " MeV";

  auto photo_nuclear_process =
      new G4HadronInelasticProcess("photonNuclear", G4Gamma::Definition());

  auto model = new LDMXCascadeInterface("LDMXBertiniWithHistory");
  model->setEnergyThreshold(energy_threshold_);
  model->SetMaxEnergy(max_energy_ * CLHEP::MeV);

  ldmx_log(info) << "  Created LDMXCascadeInterface model";

  addPNCrossSectionData(photo_nuclear_process);
  photo_nuclear_process->RegisterMe(model);
  processManager->AddDiscreteProcess(photo_nuclear_process);

  ldmx_log(info) << "  Photonuclear process added to gamma";
}

}  // namespace bertini
}  // namespace simcore

// Register the model with the factory
DECLARE_PHOTONUCLEAR_MODEL(simcore::bertini::BertiniWithHistoryModel);
