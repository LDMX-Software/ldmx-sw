/**
 * @file PhotonuclearXsecBiasingPlugin.h
 * @brief Run action plugin that biases the Geant4 photonuclear xsec by a user
 *        specified value. 
 * @author Omar Moreno
 *         SLAC National Accelerator Laboratory
 */

#ifndef SIMPLUGINS_PHOTONUCLEARXSECBIASINGPLUGIN_H_
#define SIMPLUGINS_PHOTONUCLEARXSECBIASINGPLUGIN_H_

//------------//
//   Geant4   //
//------------//
#include "G4Gamma.hh"
#include "G4HadronicProcess.hh"
#include "G4RunManager.hh"
#include "G4ProcessManager.hh"


//----------//
//   LDMX   //
//----------//
#include "SimPlugins/UserActionPlugin.h"

namespace sim {

class PhotonuclearXsecBiasingPlugin : public UserActionPlugin {

    public:

        /** Default Ctor */
        PhotonuclearXsecBiasingPlugin();

        /** Destructor */
        ~PhotonuclearXsecBiasingPlugin();

        /** @return A std::string descriptor of the class. */
        virtual std::string getName() {
            return "PhotonuclearXsecBiasingPlugin";
        }

        bool hasRunAction() {
            return true;
        }

        void beginRun(const G4Run*);
            
    private:

        /** Photonuclear cross-section multiplicative factor. */
        double xsecBiasingFactor_{100};

};

}

#endif // SIMPLUGINS_PHOTONUCLEARXSECBIASINGPLUGIN_H__
