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
  PhotonuclearModel(const std::string& name,
                    const framework::config::Parameters& parameters) = default;
  virtual ~PhotonuclearModel() = default;
  virtual void ConstructModel(G4ProcessManager* processManager) = 0;
  using Factory =
      ::simcore::Factory<PhotonuclearModel, std::shared_ptr<PhotonuclearModel>,
                         const std::string&,
                         const framework::config::Parameters&>;

  virtual void removeExistingModel(G4ProcessManager* processManager);
};
}  // namespace simcore

#define DECLARE_PHOTONUCLEAR_MODEL(CLASS)                                 \
  namespace {                                                             \
  auto v = ::simcore::PhotonuclearModel::Factory::get().declare<CLASS>(); \
  }
#endif /* SIMCORE_PHOTONUCLEAR_MODEL_H */
