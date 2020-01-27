/**
 * @file Simulator.h
 * @brief Run the G4 simulation inside of ldmx-app
 * @author Tom Eichlersmith, University of Minnesota
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _SIMAPPLICATION_SIMULATOR_H_
#define _SIMAPPLICATION_SIMULATOR_H_

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <memory>

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Event/EventDef.h"

/*~~~~~~~~~~~*/
/*   Event   */
/*~~~~~~~~~~~*/
#include "Framework/EventProcessor.h"

class G4UImanager;
class G4RunManager;
class G4GDMLParser; 
class G4GDMLMessenger; 
class G4CascadeParameters;

namespace ldmx {

    class EventFile;  
    class ParameterSet; 
    class RootPersistencyManager; 
    class RunManager;
    class DetectorConstruction;

    /**
     * @class Simulator
     * @brief Producer that runs Geant4 simulation inside of ldmx-app
     *
     * Most (if not all) of the heavy lifting is done in the classes in the 
     * Sim* modules.  This producer is mainly focused on calling appropriate
     * functions at the right time in the processing chain.
     */
    class Simulator : public ldmx::Producer {

        public:

            /**
             * Constructor.
             *
             * Blank Producer constructor
             * Constructs object that are non-configurable.
             *
             * @param name Name for this instance of the class.
             * @param process The Process class assocaited with EventProcessor, 
             *                provided by the Framework. 
             */
            Simulator(const std::string& name, ldmx::Process& process);

            /**
             * Destructor.
             *
             * Deletes hanging pointers
             */
            ~Simulator();

            /**
             * Configure the simulation.
             *
             * This is called before run is begun, so all parameters/options 
             * for simulation must be set here.
             * This function runs the pre init setup commands.
             *
             * @param ps ParameterSet for the configuration. 
             */
            virtual void configure(const ldmx::ParameterSet& ps);

            /**
             * Run simulation and export results to output event.
             *
             * @param event The event to process. 
             */
            virtual void produce(ldmx::Event &event);

            /**
             *  Callback for the EventProcessor to take any necessary action 
             *  when a new file is opened.
             *
             *  @param eventFile  The input/output file.  
             */
            void onFileOpen(EventFile& eventFile);

            /**
             * Callback for the EventProcessor to take any necessary action
             * when a file is closed.
             *
             * @param eventFile The intput/output file. 
             */
            void onFileClose(EventFile& eventFile);  

            /**
             * Initialization of simulation
             *
             * This uses the parameters set in the configure method to 
             * construct and initialize the simulation objects.
             *
             * This function runs the post init setup commands.
             */
            virtual void onProcessStart(); 

            /**
             * Tying up of loose ends
             */
            virtual void onProcessEnd();

        private:

            /**
             * Check if the input command is allowed to be run.
             *
             * Looks for sub-strings matching the ones listed as an invalid command.
             * These invalid commands are mostly commands where control has been handed over to Simulator.
             */
            bool allowed(const std::string& command) const;

        private:

            /// Manager controlling G4 simulation run 
            std::unique_ptr<G4RunManager> runManager_;

            /// User interface handle
            G4UImanager* uiManager_{nullptr};

            /// GDML parser 
            std::unique_ptr<G4GDMLParser> parser_;

            /// Messenger that allows passing commands to the parser
            std::unique_ptr<G4GDMLMessenger> gdmlMessenger_; 

            /// Geant4 Class Instance holding cascade parameters
            ///     Can't be a unique_ptr
            const G4CascadeParameters* cascadeParameters_{nullptr};

            /// LDMX Object that constructs our detector
            std::unique_ptr<DetectorConstruction> detectorConstruction_;

            /// PersistencyManager 
            std::unique_ptr<RootPersistencyManager> persistencyManager_;  

            /// Commands not allowed to be passed from python config file
            ///     This is because Simulator already runs them.
            static const std::vector< std::string > invalidCommands_;

            /*********************************************************
             * Python Configuration Parameters
             *********************************************************/

            /// Short Description of Simulation for Run Header
            std::string description_;
            
            /// Path to detector description
            std::string detectorPath_{""};

            /// Run Number for this Sim Run
            int runNumber_;

            /// Vebosity for the simulation
            int verbosity_{1};

            /// Collections to drop from simulation (usually scoring plane collections)
            std::vector< std::string > dropCollections_;
            
            /// Path to scoring planes description
            std::string scoringPlanesPath_{""};

            /// Vector of Random Seeds to use for this run.
            std::vector< int > randomSeeds_;

            /// Vector to use as beamspot smearing
            std::vector< double > beamspotSmear_;
            
            /// Vector of Geant4 Commands to Run before /run/initialize
            std::vector< std::string > preInitCommands_;
            
            /// Vector of Geant4 Commands to Run after /run/initialize
            std::vector< std::string > postInitCommands_;

    };
}

#endif /* SIMAPPLICATION_SIMULATOR_H */
