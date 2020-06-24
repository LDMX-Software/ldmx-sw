/**
 * @file ElectroNulcearXsecBiasingOperator.h
 * @brief Geant4 Biasing Operator used to bias the occurrence of electronuclear 
 *        events by modifying the cross-section.
 * @author Omar Moreno
 *         SLAC National Accelerator Laboratory
 */

#ifndef BIASING_ELECTRONUCLEARXSECBIASINGOPERATOR_H_
#define BIASING_ELECTRONUCLEARXSECBIASINGOPERATOR_H_

//------------//
//   Geant4   //
//------------//
#include "G4BiasingProcessInterface.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4Track.hh"
#include "G4VBiasingOperator.hh"

//----------//
//   LDMX   //
//----------//
#include "XsecBiasingOperator.h"

namespace ldmx { 

    class ElectroNuclearXsecBiasingOperator : public XsecBiasingOperator { 
    
        public: 
        
            /** Constructor */
            ElectroNuclearXsecBiasingOperator(std::string name); 

            /** Destructor */
            ~ElectroNuclearXsecBiasingOperator(); 

            /** Method called at the beginning of a run. */
            void StartRun();

            /** 
             * @return Method that returns the biasing operation that will be used
             *         to bias the occurence of photonuclear events.
             */
            G4VBiasingOperation* ProposeOccurenceBiasingOperation(const G4Track* track,
                    const G4BiasingProcessInterface* callingProcess);

        
        protected: 

            /** Return the process to bias. */
            virtual std::string getProcessToBias() { return ELECTRONUCLEAR_PROCESS; }

        private: 

            /** Geant4 electronuclear process name. */
            static const std::string ELECTRONUCLEAR_PROCESS; 

    }; // ElectroNuclearXsecBiasingOperator
}

#endif // BIASING_ELECTRONUCLEARXSECBIASINGOPERATOR_H_
