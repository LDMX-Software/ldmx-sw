#ifndef SIMCORE_PHOTONUCLEAR_MODEL_H
#define SIMCORE_PHOTONUCLEAR_MODEL_H
#include <G4ProcessManager.hh>
#include <string>
#include <utility>

#include "Framework/Configure/Parameters.h"
#include "SimCore/Factory.h"
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
};
}  // namespace simcore

#define DECLARE_PHOTONUCLEAR_MODEL(CLASS)                                 \
  namespace {                                                             \
  auto v = ::simcore::PhotonuclearModel::Factory::get().declare<CLASS>(); \
  }
#endif /* SIMCORE_PHOTONUCLEAR_MODEL_H */
