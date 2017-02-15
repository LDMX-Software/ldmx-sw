/**
 * @file EcalProcessFilter.h
 * @brief User action plugin that biases Geant4 to only process events which
 *        involve a photonuclear reaction in the ECal.
 * @author Omar Moreno
 *         SLAC National Accelerator Laboratory
 */

#ifndef SIMPLUGINS_ECALPROCESSFILTER_H_
#define SIMPLUGINS_ECALPROCESSFILTER_H_

//------------//
//   Geant4   //
//------------//
#include "G4RunManager.hh"

//----------//
//   LDMX   //
//----------//
#include "SimPlugins/UserActionPlugin.h"
#include "Biasing/BiasingMessenger.h"

namespace ldmx {

    class EcalProcessFilter : public UserActionPlugin {

        public:

            /** Default Ctor */
            EcalProcessFilter();

            /** Destructor */
            ~EcalProcessFilter();

            /** @return A std::string descriptor of the class. */
            virtual std::string getName() {
                return "EcalProcessFilter";
            }

            bool hasSteppingAction() {
                return true;
            }

            void stepping(const G4Step* step);

        private:

            /** Brem photon energy threshold */
            double photonEnergyThreshold_{2500}; // MeV
    
    }; // EcalProcessFilter 
}

#endif // SIMPLUGINS_ECALPROCESSFILTER_H__
