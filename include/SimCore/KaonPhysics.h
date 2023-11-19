#ifndef SIMCORE_KAON_PHYSICS_H
#define SIMCORE_KAON_PHYSICS_H

#include <G4DecayTable.hh>
#include <G4KaonMinus.hh>
#include <G4KaonPlus.hh>
#include <G4ParticleDefinition.hh>
#include <G4VDecayChannel.hh>
#include <G4VPhysicsConstructor.hh>
#include <numeric>
#include <vector>

#include "Framework/Configure/Parameters.h"
#include "Framework/Exception/Exception.h"

namespace simcore {

/**
 * @class KaonPhysics
 *
 * @brief Allows the configuration of properties of kaons
 * produced in the simulation, in particular setting their lifetime and
 * branching ratios
 *
 * @note Only affects charged kaons, but similar changes could be added for the
 * neutral ones in a rather straight-forward manner.
 */

class KaonPhysics : public G4VPhysicsConstructor {
};
}  // namespace simcore

#endif /* SIMCORE_KAON_PHYSICS_H */
