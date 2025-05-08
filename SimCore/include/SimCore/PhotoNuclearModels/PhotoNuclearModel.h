#ifndef SIMCORE_PHOTONUCLEAR_MODEL_H
#define SIMCORE_PHOTONUCLEAR_MODEL_H
#include <G4CrossSectionDataSetRegistry.hh>
#include <G4HadronInelasticProcess.hh>
#include <G4HadronicInteraction.hh>
#include <G4PhotoNuclearCrossSection.hh>
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

class PhotoNuclearModel {
 public:
  /**
   * Base class does not take any parameters or do anything in particular, but
   * any derived class may.
   *
   * @param[in] name unique instance name for this model
   * @param[in] parameters python configuration
   */
  PhotoNuclearModel(const std::string& name,
                    const framework::config::Parameters& parameters){};
  virtual ~PhotoNuclearModel() = default;

  /**
   * The primary part of the model interface, responsible for adding the desired
   * G4HadronicInteraction to the process manager for the G4Gamma class.
   *
   * @param[in] processManager the process manager for the G4Gamma class, passed
   * in automatically by the GammaPhysics module.
   */
  virtual void ConstructGammaProcess(G4ProcessManager* processManager) = 0;

  /**
   * The factory for PhotoNuclearModels.
   */
  using Factory =
      ::simcore::Factory<PhotoNuclearModel, std::shared_ptr<PhotoNuclearModel>,
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

  /**
   * Default implementation for adding XS data for the process.
   *
   * The default implementation is adapted from G4PhotoNuclearProcess.hh but can
   * be overridden to add cross sections in another way (e.g. from FLUKA). If no
   * cross-section is added to the process, the simulation process will halt
   * when attempting to calculate the mean free path so there is no way of
   * accidentally forgetting to enable the XS data.
   *
   * Typically called during `ConstructGammaProcess`.
   *
   */
  virtual void addPNCrossSectionData(G4HadronInelasticProcess* process) const;
};
}  // namespace simcore

/**
 * @macro DECLARE_PHOTONUCLEAR_MODEL
 *
 * Defines a builder for the declared derived photonuclear model class and
 * registers it as a possible photonuclear model. If you are implementing your
 * own photonuclear model class, make sure to invoke this macro in your
 * implementation file.
 *
 * See e.g. SimCore/src/SimCore/PhotoNuclearModels/BertiniModel.cxx
 */
#define DECLARE_PHOTONUCLEAR_MODEL(CLASS)                                 \
  namespace {                                                             \
  auto v = ::simcore::PhotoNuclearModel::Factory::get().declare<CLASS>(); \
  }
#endif /* SIMCORE_PHOTONUCLEAR_MODEL_H */
