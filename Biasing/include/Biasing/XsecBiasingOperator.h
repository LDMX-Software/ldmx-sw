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
#include "Biasing/XsecBiasingOperatorMessenger.h"

namespace ldmx { 

    class XsecBiasingOperatorMessenger; 

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


            /** Bias all particles of the given type. */
            void biasAll() { biasAll_ = true; };

            /** Bias only the incident particle. */
            void biasIncident() { biasIncident_ = true; };

            /** Set the particle type to bias. */
            void setParticleType(std::string particleType) { particleType_ = particleType; };

            /** Set the process to bias. */
            void setProcess(std::string process) { process_ = process; };

            /** Set the minimum energy required to bias the particle. */
            void setThreshold(double threshold) { threshold_ = threshold; };

            /** Set the factor by which the xsec will be enhanced. */
            void setXsecFactor(double xsecFactor) { xsecFactor_ = xsecFactor; };

        private: 

            /** Cross-section biasing operation */
            G4BOptnChangeCrossSection* xsecOperation{nullptr};

            /** Messenger used to pass arguments to this operator. */
            XsecBiasingOperatorMessenger* messenger_{nullptr}; 

            /** Flag indicating whether all particles should be biased. */
            bool biasAll_{false};

            /** Flag indicating whether to bias only the incident particle. */
            bool biasIncident_{false};

            /** The particle type to bias. */
            std::string particleType_{""}; 

            /** The process that the biasing will be applied to. */
            std::string process_{""}; 

            /** The minimum energy required to apply the biasing operation. */
            double threshold_{0};

            /** Factor to multiply the xsec by. */
            double xsecFactor_{0};

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
