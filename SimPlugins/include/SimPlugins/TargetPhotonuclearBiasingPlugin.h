/**
 * @file TargetPhotonuclearBiasing.cxx
 * @brief User action plugin that biases Geant4 to only process events which
 *        involve a photonuclear reaction in the target.
 * @author Omar Moreno
 *         SLAC National Accelerator Laboratory
 */

#ifndef SIMPLUGINS_TARGETPHOTONUCLEARBIASINGPLUGIN_H_
#define SIMPLUGINS_TARGETPHOTONUCLEARBIASINGPLUGIN_H_

//------------//
//   Geant4   //
//------------//
#include "G4RunManager.hh"

//----------//
//   LDMX   //
//----------//
#include "SimPlugins/UserActionPlugin.h"

namespace sim {

class TargetPhotonuclearBiasingPlugin : public UserActionPlugin {

    public:

        /** Default Ctor */
        TargetPhotonuclearBiasingPlugin();

        /** Destructor */
        ~TargetPhotonuclearBiasingPlugin();

        /** @return A std::string descriptor of the class. */
        virtual std::string getName() {
            return "TargetPhotonuclearBiasingPlugin";
        }

        bool hasSteppingAction() {
            return true;
        }

        bool hasStackingAction() { 
            return true;
        }

        void stepping(const G4Step* step);

        G4ClassificationOfNewTrack stackingClassifyNewTrack(const G4Track*, const G4ClassificationOfNewTrack&);

    private:

        /** The volume name of the LDMX target. */
        G4String volumeName_{"target_PV"};

        /** Brem photon energy threshold */
        double photonEnergyThreshold_{2500}; // MeV
};

}

#endif // SIMPLUGINS_TARGETPHOTONUCLEARBIASINGPLUGIN_H__
