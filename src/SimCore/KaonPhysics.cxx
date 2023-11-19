#include "SimCore/KaonPhysics.h"

namespace simcore {
KaonPhysics::KaonPhysics(const G4String& name,
                         const framework::config::Parameters& parameters)
    : G4VPhysicsConstructor(name) {
  kplus_branching_ratios =
      parameters.getParameter<std::vector<double>>("kplus_branching_ratios");
  kminus_branching_ratios =
      parameters.getParameter<std::vector<double>>("kminus_branching_ratios");
  kplus_lifetime_factor =
      parameters.getParameter<double>("kplus_lifetime_factor");
  kminus_lifetime_factor =
      parameters.getParameter<double>("kminus_lifetime_factor");
}

}  // namespace simcore
