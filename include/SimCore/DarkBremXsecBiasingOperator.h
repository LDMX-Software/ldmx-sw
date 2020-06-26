/**
 * @file DarkBremXsecBiasingOperator.h
 * @brief Geant4 Biasing Operator used to bias the occurence of dark brem
 *        events by modifying the cross-section.
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef SIMCORE_DARKBREMXSECBIASINGOPERATOR_H
#define SIMCORE_DARKBREMXSECBIASINGOPERATOR_H

//----------//
//   LDMX   //
//----------//
#include "SimCore/XsecBiasingOperator.h"

class G4Track;
class G4BiasingProcessInterface;
class G4VBiasingOperation;

namespace ldmx { 

    class DarkBremXsecBiasingOperator : public XsecBiasingOperator { 

        public: 

            /** 
             * Constructor 
             *
             * Calls base class constructor.
             */
            DarkBremXsecBiasingOperator(std::string name);

            /** 
             * Destructor 
             *
             * Blank right now
             */
            ~DarkBremXsecBiasingOperator();

            /** 
             * Method called at the beginning of a run. 
             *
             * @sa XsecBiasingOperator::StartRun()
             */
            void StartRun();

            /** 
             * @return Method that returns the biasing operation that will be used
             *         to bias the occurence of events.
             */
            G4VBiasingOperation* ProposeOccurenceBiasingOperation(const G4Track* track,
                    const G4BiasingProcessInterface* callingProcess);

        
        protected:

            virtual std::string getProcessToBias() { return DARKBREM_PROCESS; }

        private: 

            /** Geant4 photonuclear process name. */
            static const std::string DARKBREM_PROCESS;

            /** Unbiased darkbrem xsec. */
            double dbXsecUnbiased_{0};

            /** Biased darkbrem xsec. */
            double dbXsecBiased_{0};  

    };  // DarkBremXsecBiasingOperator
}

#endif // SIMCORE_DARKBREMXSECBIASINGOPERATOR_H
