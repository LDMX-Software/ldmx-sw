/**
 * @file LCIOPersistencyManager.h 
 * @brief Class used to manage LCIO based persistency. 
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_LCIOPERSISTENCYMANAGER_H
#define SIMCORE_LCIOPERSISTENCYMANAGER_H

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4PersistencyManager.hh" 

// Forward Declarations
class G4Event; 
class G4Run; 

namespace ldmx { 

    class LCIOPersistencyManager : public G4PersistencyManager { 
    
        public: 

            /**
             *  Class constructor.
             *
             *  @param eventFile 
             *  @param parameters Parameters used to configure the manager
             */
            LCIOPersistencyManager(EventFile &file, Parameters& parameters);

            /// Destructor 
            ~LCIOPersistencyManager() {}

            /**
             * Builds the output LCIO event. 
             *
             * @param g4Event A Geant4 event. 
             */
            G4bool Store(const G4Event* g4Event) final override; 

            /**
             *  
             */
            G4bool Store(const G4Run* g4Run) final override; 

        private: 

    };

} // ldmx

#endif // SIMCORE_LCIOPERSISTENCYMANAGER_H
