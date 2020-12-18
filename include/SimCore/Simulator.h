/**
 * @file Simulator.h
 * @brief Run the G4 simulation inside of ldmx-app
 * @author Tom Eichlersmith, University of Minnesota
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _SIMCORE_SIMULATOR_H_
#define _SIMCORE_SIMULATOR_H_

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <any>
#include <map>
#include <memory>
#include <string> 

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/EventDef.h"
#include "Framework/EventProcessor.h"
#include "Framework/Configure/Parameters.h" 

#include "SimCore/ConditionsInterface.h"

class G4UImanager;
class G4UIsession;
class G4RunManager;
class G4GDMLParser; 
class G4GDMLMessenger; 
class G4CascadeParameters;

namespace simcore {
namespace persist { 
class RootPersistencyManager; 
}
}

namespace ldmx {

    class EventFile;  
    class ParameterSet; 
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
             * Callback for the processor to configure itself from the given set
             * of parameters.
             * 
             * @param parameters ParameterSet for configuration.
             */
            void configure(Parameters& parameters) final override; 

            /**
             * Given a non-const reference to the new RunHeader,
             * we can add parameters from the simulation here
             * before the run starts.
             *
             * @param header of new run
             */
            void beforeNewRun(RunHeader& header) final override;

            /**
             * Before the run starts (but after the conditions are configured)
             * set up the random seeds for this run.
             *
             * @param[in] header RunHeader for this run, unused
             */
            void onNewRun(const RunHeader& header) final override;

            /**
             * Run simulation and export results to output event.
             *
             * @param event The event to process. 
             */
            virtual void produce(ldmx::Event &event) final override;

            /**
             *  Callback for the EventProcessor to take any necessary action 
             *  when a new file is opened.
             *
             *  @param eventFile  The input/output file.  
             */
            void onFileOpen(EventFile& eventFile) final override;

            /**
             * Callback for the EventProcessor to take any necessary action
             * when a file is closed.
             *
             * @param eventFile The intput/output file. 
             */
            void onFileClose(EventFile& eventFile) final override;  

            /**
             * Initialization of simulation
             *
             * This uses the parameters set in the configure method to 
             * construct and initialize the simulation objects.
             *
             * This function runs the post init setup commands.
             */
            void onProcessStart() final override; 

            
            /// Callback called once processing is complete. 
            void onProcessEnd() final override;

        private:

            /**
             * Check if the input command is allowed to be run.
             *
             * Looks for sub-strings matching the ones listed as an invalid command.
             * These invalid commands are mostly commands where control has been handed over to Simulator.
             */
            bool allowed(const std::string& command) const;


            /**
             * Set the seeds to be used by the Geant4 random engine. 
             *
             * @param[in] seeds A vector of seeds to pass to the G4 random 
             *      engine.  The vector must contain at least 2 seeds otherwise 
             *      an exception is thrown. 
             */
            void setSeeds(std::vector< int > seeds); 

        private:

            /// Manager controlling G4 simulation run 
            std::unique_ptr<RunManager> runManager_;

            /// User interface handle
            G4UImanager* uiManager_{nullptr};

            /// PersistencyManager 
            std::unique_ptr<simcore::persist::RootPersistencyManager> persistencyManager_;

            /// Handle to the G4Session -> how to deal with G4cout and G4cerr
            std::unique_ptr<G4UIsession> sessionHandle_;

            /// Commands not allowed to be passed from python config file
            ///     This is because Simulator already runs them.
            static const std::vector< std::string > invalidCommands_;

            /// Number of events started
            int numEventsBegan_{0};

            /// Number of events completed
            int numEventsCompleted_{0};

            ///  Conditions interface
            ConditionsInterface conditionsIntf_;
      
            /*********************************************************
             * Python Configuration Parameters
             *********************************************************/

            /// The parameters used to configure the simulation
            Parameters parameters_;   

            /// Vebosity for the simulation
            int verbosity_{1};

    };
}

#endif /* SIMCORE_SIMULATOR_H */
