/**
 * @file TargetProcessFilter.h
 * @brief Class defining a UserActionPlugin that biases Geant4 to only process events which
 *        involve a photonuclear reaction in the target
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMPLUGINS_TARGETPROCESSFILTER_H_
#define SIMPLUGINS_TARGETPROCESSFILTER_H_

// Geant4
#include "G4RunManager.hh"

// LDMX
#include "SimPlugins/UserActionPlugin.h"
#include "Biasing/BiasingMessenger.h"

namespace ldmx {

/**
 * @class TargetProcessFilter
 * @brief Biases Geant4 to only process events where PN reaction occurred in the target
 */
class TargetProcessFilter : public UserActionPlugin {

    public:

        /**
         * Class constructor.
         */
        TargetProcessFilter();

        /**
         * Class destructor.
         */
        ~TargetProcessFilter();

        /**
         * Get the name of the plugin.
         * @return The name of the plugin.
         */
        virtual std::string getName() {
            return "TargetProcessFilter";
        }

        /**
         * Get whether this plugin implements the stepping action.
         * @return True to indicate this plugin implements the stepping action.
         */
        bool hasSteppingAction() {
            return true;
        }

        /**
         * Implementmthe stepping action which performs the target volume biasing.
         * @param step The Geant4 step.
         */
        void stepping(const G4Step* step);

    private:

        /** The volume name of the LDMX target. */
        G4String volumeName_{"target_PV"};

        /** Brem photon energy threshold. */
        double photonEnergyThreshold_{2500}; // MeV
};

}

#endif // SIMPLUGINS_TARGETPROCESSFILTER_H__
