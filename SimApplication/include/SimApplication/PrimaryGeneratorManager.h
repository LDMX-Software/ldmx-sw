/**
 * @file PrimaryGeneratorManager.h 
 * @brief Class that manages the generators used to fire particles. 
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_PRIMARYGENERATORMANAGER_H
#define SIMCORE_PRIMARYGENERATORMANAGER_H

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Parameters.h" 

// Forward Declarations
class G4VPrimaryGenerator; 

namespace ldmx { 

    /**
     * @class PrimaryGeneratorManager
     * @brief Class that manages the generators used to fire particles. 
     */
    class PrimaryGeneratorManager {

        public: 

            /// Constructor
            PrimaryGeneratorManager(Parameters& parameters); 

            /// Destructor
            ~PrimaryGeneratorManager();

            /**
             * Get the collection of all enabled generators
             */
            std::vector< G4VPrimaryGenerator* > getGenerators() const { return generators_; }; 

        private:

            /**
             * Initialize and configure the primary generators.
             *
             * @param parameters The parameters used to determine which 
             *                   generators are initialized and how they are 
             *                   configured. 
             */ 
            void initialize(Parameters& parameters); 
            
            /// Cointainer for all generators to be used by the simulation
            std::vector< G4VPrimaryGenerator* > generators_;

    }; // PrimaryGeneratorManager

} // ldmx

#endif // SIMCORE_PRIMARYGENERATORMANAGER_H

