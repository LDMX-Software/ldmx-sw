#ifndef SIMCORE_PHOTONUCLEAR_MODEL_H
#define SIMCORE_PHOTONUCLEAR_MODEL_H
#include <G4ProcessManager.hh>
#include <string>
#include <utility>

#include "Framework/Configure/Parameters.h"
#include "SimCore/Factory.h"
/*
** Dynamically loadable photonuclear models either from SimCore or external
** libraries implementing this interface. For example implementations, see the
** BertiniNothingHard model in SimCore.
**
** Allows for replacing the default Bertini model from Geant4 with any other
** G4HadronicInteraction process. The library is used from within the
** GammaPhysics module in SimCore which ensures that the removeExistingModel and
** ConstructModel functions are called in the right order and that the
** photonNuclear process is located in the right part of the G4Gamma process
** list.
*/
namespace simcore {
class PhotonuclearModel {
 public:
  /**
   * Base class does not take any parameters or do anything in particular, but
   * any derived class may.
   *
   * @param[in] name unique instance name for this model
   * @param[in] parameters python configuration
   */
  PhotonuclearModel(const std::string& name,
                    const framework::config::Parameters& parameters){};
  virtual ~PhotonuclearModel() = default;

  /**
   * The primary part of the model interface, responsible for adding the desired
   * G4HadronicInteraction to the process manager for the G4Gamma class.
   *
   * @param[in] processManager the process manager for the G4Gamma class, passed
   * in automatically by the GammaPhysics module.
   */
  virtual void ConstructModel(G4ProcessManager* processManager) = 0;

  /**
   * The factory for PhotonuclearModels.
   */
  using Factory =
      ::simcore::Factory<PhotonuclearModel, std::shared_ptr<PhotonuclearModel>,
                         const std::string&,
                         const framework::config::Parameters&>;

  /**
   * Removes any existing photonNuclear process from the process manager of the
   * G4Gamma class. Should in general not be overridden for most models other
   * than the default Bertini model (which just retains the default
   * interaction).
   *
   * @param[in] processManager the process manager for the G4Gamma class, passed
   * in automatically by the GammaPhysics module.
   */
  virtual void removeExistingModel(G4ProcessManager* processManager);
};
}  // namespace simcore

/**
 * @macro DECLARE_PHOTONUCLEAR_MODEL
 *
 * Defines a builder for the declared derived photonuclear model class and
 * registers it as a possible photonuclear model. If you are implementing your
 * own photonuclear model class, make sure to invoke this macro in your
 * implementation file.
 */
#define DECLARE_PHOTONUCLEAR_MODEL(CLASS)                                 \
  namespace {                                                             \
  auto v = ::simcore::PhotonuclearModel::Factory::get().declare<CLASS>(); \
  }
#endif /* SIMCORE_PHOTONUCLEAR_MODEL_H */
