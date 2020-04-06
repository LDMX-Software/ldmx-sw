/**
 * @file G4Session.h
 * @brief Classes which redirect the output of G4cout and G4cerr
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef SIMAPPLICATION_G4SESSION_H_
#define SIMAPPLICATION_G4SESSION_H_

#include "Exception/Logger.h"

// Geant4
#include "G4UIsession.hh"

namespace ldmx {

    /**
     * @class LoggedSession
     *
     * Log the output of Geant4 to files in current directory.
     */
    class LoggedSession : public G4UIsession {
        
        public:

            /**
             * Constructor
             *
             * Sets up output file streams for the cout and cerr paths.
             */
            LoggedSession() { }

            /**
             * Destructor
             *
             * Closes the output files streams
             */
            ~LoggedSession() { }

            /**
             * Required hook for Geant4
             *
             * Does nothing
             */
            G4UIsession* SessionStart() { return nullptr; }

            /**
             * Redirects cout to file
             */
            G4int ReceiveG4cout(const G4String& message);

            /**
             * Redirects cerr to file
             */
            G4int ReceiveG4cerr(const G4String& message);

        private:

            /** enable logging macro */
            enableLogging( "Geant4" )

    }; //LoggedSession

} //ldmx

#endif
