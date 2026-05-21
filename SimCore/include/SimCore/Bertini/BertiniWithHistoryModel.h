/**
 * @file BertiniWithHistoryModel.h
 * @brief Photonuclear model with cascade history capture
 */

#ifndef SIMCORE_BERTINI_BERTINIWITHHISTORYMODEL_H
#define SIMCORE_BERTINI_BERTINIWITHHISTORYMODEL_H

#include "Framework/Configure/Parameters.h"
#include "Framework/Logger.h"
#include "SimCore/PhotoNuclearModels/PhotoNuclearModel.h"

class G4ProcessManager;

namespace simcore {
namespace bertini {

class LDMXCascadeInterface;

/**
 * @class BertiniWithHistoryModel
 * @brief PhotoNuclear model that records Bertini cascade history
 *
 * Uses LDMXCascadeInterface to capture cascade history for each PN interaction.
 * Histories are stored in CascadeHistoryStore and retrieved during event
 * finalization.
 *
 * @code{.py}
 * sim.photonuclear_model = photonuclear_models.BertiniWithHistoryModel()
 * @endcode
 */
class BertiniWithHistoryModel : public PhotoNuclearModel {
 public:
  BertiniWithHistoryModel(const std::string& name,
                          const framework::config::Parameters& parameters);
  virtual ~BertiniWithHistoryModel() = default;

  void constructGammaProcess(G4ProcessManager* processManager) override;

 private:
  /** Maximum energy for the model [MeV] */
  double max_energy_{15000.0};

  /** Minimum photon energy to record history [MeV] */
  double energy_threshold_{5000.0};

  enableLogging("BertiniWithHistoryModel")
};

}  // namespace bertini
}  // namespace simcore

#endif  // SIMCORE_BERTINI_BERTINIWITHHISTORYMODEL_H
