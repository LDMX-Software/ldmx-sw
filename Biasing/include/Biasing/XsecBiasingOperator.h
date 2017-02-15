/**
 * @file XsecBiasingPlugin.h
 * @brief Geant4 Biasing Operator used to bias the occurence of photonuclear 
 *        events by modifying the cross-section.
 * @author Omar Moreno
 *         SLAC National Accelerator Laboratory
 */

#ifndef BIASING_XSECBIASINGOPERATOR_H_
#define BIASING_XSECBIASINGOPERATOR_H_

//------------//
//   Geant4   //
//------------//
#include "G4BiasingProcessInterface.hh"
#include "G4BiasingProcessSharedData.hh"
#include "G4BOptnChangeCrossSection.hh"
#include "G4Gamma.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"
#include "G4ProcessManager.hh"
#include "G4RunManager.hh"
#include "G4Track.hh"
#include "G4VBiasingOperator.hh"

//----------//
//   LDMX   //
//----------//
#include "Biasing/BiasingMessenger.h"

namespace ldmx { 

    class XsecBiasingOperator : public G4VBiasingOperator { 

        public: 

            /** Constructor */
            XsecBiasingOperator(std::string name);

            /** Destructor */
            ~XsecBiasingOperator();

            /** Method called at the beginning of a run. */
            void StartRun();

            /** 
             * @return Method that returns the biasing operation that will be used
             *         to bias the occurence of photonuclear events.
             */
            G4VBiasingOperation* ProposeOccurenceBiasingOperation(const G4Track* track,
                    const G4BiasingProcessInterface* callingProcess);


        private: 

            /** Cross-section biasing operation */
            G4BOptnChangeCrossSection* xsecOperation{nullptr};

            /** The process that the biasing will be applied to. */
            std::string process_; 

            /** Factor to multiply the xsec by. */
            double xsecTrans_;

            //--------//
            // Unused //
            //--------//
            G4VBiasingOperation* ProposeFinalStateBiasingOperation(const G4Track*,
                    const G4BiasingProcessInterface*) { return nullptr; }

            G4VBiasingOperation* ProposeNonPhysicsBiasingOperation(const G4Track*,
                    const G4BiasingProcessInterface*) { return nullptr; }

    };  // XsecBiasingOperator

}

#endif // SIMPLUGINS_XSECBIASINGOPERATOR_H_ 
