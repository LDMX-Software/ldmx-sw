/**
 * 
 */

#ifndef SIMAPPLICATION_PARALLELWORLD_H_
#define SIMAPPLICATION_PARALLELWORLD_H_

//---------------//
//   C++StdLib   //
//---------------//
#include <string>

//------------//
//   Geant4   //
//------------//
#include "G4GDMLParser.hh"
#include "G4String.hh"
#include "G4VUserParallelWorld.hh"

//-------------//
//   ldmx-sw   //
//-------------//
#include "Exception/Logger.h"
#include "SimApplication/AuxInfoReader.h"

namespace ldmx { 

    class ParallelWorld : public G4VUserParallelWorld { 

        public: 

            /** Constructor */
            ParallelWorld(G4GDMLParser* parser, G4String worldName);

            /** Destructor */
            ~ParallelWorld();

            /** */
            void Construct();

            /** */
            void ConstructSD();

        private:

            /** GDML parser */
            G4GDMLParser* parser_{nullptr};
            
            /** The auxiliary GDML info reader. */
            AuxInfoReader* auxInfoReader_{nullptr};

            /** enable logging macro */
            enableLogging("ParallelWorld")

    }; // ParallelWorld
}

#endif // SIMAPPLICATION_PARALLELWORLD_H_

