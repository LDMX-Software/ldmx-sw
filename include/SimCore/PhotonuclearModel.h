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
};
}  // namespace simcore

#define DECLARE_PHOTONUCLEAR_MODEL(CLASS)                                 \
  namespace {                                                             \
  auto v = ::simcore::PhotonuclearModel::Factory::get().declare<CLASS>(); \
  }
#endif /* SIMCORE_PHOTONUCLEAR_MODEL_H */
