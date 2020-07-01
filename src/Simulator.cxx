/**
 * @file Simulator.cxx
 * @brief Producer that runs Geant4 simulation inside of ldmx-app
 * @author Tom Eichlersmith, University of Minnesota
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimCore/Simulator.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Process.h"
#include "Event/Version.h" //for LDMX_INSTALL path

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/DetectorConstruction.h"
#include "SimCore/RootPersistencyManager.h" 
#include "SimCore/RunManager.h"
#include "SimCore/G4Session.h"

/*~~~~~~~~~~~~~~*/
/*    Geant4    */
/*~~~~~~~~~~~~~~*/
#include "G4UIsession.hh"
#include "G4UImanager.hh"
#include "G4CascadeParameters.hh"
#include "G4GeometryManager.hh"
#include "G4GDMLParser.hh"
#include "Randomize.hh" 

namespace ldmx {

    const std::vector<std::string> Simulator::invalidCommands_ = {
            "/run/initialize", //hard coded at the right time
            "/run/beamOn", //passed commands should only be sim setup
            "/random/setSeeds", //handled by own config parameter (if passed)
            "ldmx", //all ldmx messengers have been removed
            "/persistency/gdml/read" //detector description is read after passed a path to the detector description (required)
        };

    Simulator::Simulator(const std::string& name, ldmx::Process& process) : Producer( name , process ) {

        // Get the ui manager from geant
        //      This pointer is handled by Geant4
        uiManager_ = G4UImanager::GetUIpointer();

    }

    Simulator::~Simulator() { }

    void Simulator::configure(Parameters& parameters) {
    
        // parameters used to configure the simulation
        parameters_ = parameters; 
        
        // Set the run number. If not specified, the run number is set to 0. 
        int runNumber = parameters_.getParameter< int >( "runNumber" );
        process_.setRunNumber( runNumber );

        // Set the verbosity level.  The default level  is 0.
        verbosity_ = parameters_.getParameter< int >("verbosity");

        // If the verbosity level is set to 0, 
        // If the verbosity level is > 1, log everything to a file. Otherwise,
        // dump the output. If a prefix has been specified, append it ot the 
        // log message. 
        auto loggingPrefix = parameters_.getParameter< std::string >("logging_prefix");
        if ( verbosity_ == 0 ) sessionHandle_ = std::make_unique<BatchSession>();
        else if ( verbosity_ > 1 ) {
            
            if ( loggingPrefix.empty() ) sessionHandle_ = std::make_unique<LoggedSession>();
            else sessionHandle_ = std::make_unique<LoggedSession>( loggingPrefix + "_G4cout.log" , loggingPrefix + "_G4cerr.log" );

        }
        if (sessionHandle_ != nullptr) uiManager_->SetCoutDestination( sessionHandle_.get() ); 

        // Instantiate the run manager.  
        runManager_ = std::make_unique<RunManager>(parameters);

        // Instantiate the GDML parser and corresponding messenger owned and
        // managed by DetectorConstruction
        G4GDMLParser *parser = new G4GDMLParser;

        // Instantiate the class so cascade parameters can be set.
        G4CascadeParameters::Instance();

        // Set the DetectorConstruction instance used to build the detector 
        // from the GDML description. 
        runManager_->SetUserInitialization( new DetectorConstruction( parser , parameters ) );

        // Parse the detector geometry and validate if specified.
        auto detectorPath{parameters_.getParameter< std::string >("detector")};
        auto validateGeometry{parameters_.getParameter< bool >("validate_detector")}; 
        if ( verbosity_ > 0 ) {
            std::cout << "[ Simulator ] : Reading in geometry from '" << detectorPath << "'... " << std::flush;
        }
        G4GeometryManager::GetInstance()->OpenGeometry();
        parser->Read( detectorPath, validateGeometry );
        runManager_->DefineWorldVolume( parser->GetWorldVolume() );

        auto preInitCommands = parameters_.getParameter< std::vector< std::string > >("preInitCommands" ,{} ); 
        for ( const std::string& cmd : preInitCommands ) {
            if ( allowed(cmd) ) {
                int g4Ret = uiManager_->ApplyCommand( cmd );
                if ( g4Ret > 0 ) {
                    EXCEPTION_RAISE(
                            "PreInitCmd",
                            "Pre Initialization command '" + cmd + "' returned a failue status from Geant4: " + std::to_string(g4Ret)
                            );
                }
            } else {
                EXCEPTION_RAISE(
                        "PreInitCmd",
                        "Pre Initialization command '" + cmd + "' is not allowed because another part of Simulator handles it."
                        );
            }
        }
    }

    void Simulator::onFileOpen(EventFile &file) {
        // Initialize persistency manager and connect it to the current EventFile
        persistencyManager_ = std::make_unique<RootPersistencyManager>(file, parameters_); 
        persistencyManager_->Initialize(); 
    }

    void Simulator::produce(ldmx::Event& event) {

        // Pass the current LDMX event object to the persistency manager.  This
        // is needed by the persistency manager to fill the current event. 
        persistencyManager_->setCurrentEvent( &event ); 

        // Generate and process a Geant4 event. 
        numEventsBegan_++;
        runManager_->ProcessOneEvent( event.getEventHeader().getEventNumber() );

        // If a Geant4 event has been aborted, skip the rest of the processing
        // sequence. This will immediately force the simulation to move on to 
        // the next event. 
        if ( runManager_->GetCurrentEvent()->IsAborted() ) { 
            runManager_->TerminateOneEvent(); //clean up event objects
            this->abortEvent();  //get out of processors loop
        }
        
        if ( process_.getLogFrequency() > 0 and event.getEventHeader().getEventNumber() % process_.getLogFrequency() == 0 ) {
            //print according to log frequency and verbosity
            if ( verbosity_ > 1 ) std::cout << "[ Simulator ] : Printing event contents:" << std::endl;
            event.Print( verbosity_ );
        }

        // Terminate the event.  This checks if an event is to be stored or 
        // stacked for later. 
        numEventsCompleted_++;
        runManager_->TerminateOneEvent();
    
        return;
    }
    
    void Simulator::onProcessStart() {
        
        //initialize run
        runManager_->Initialize();

        // If specified, set the seeds.  The seed vector must contain at 
        // least two seeds otherwise nothing will get set.
        auto seeds{parameters_.getParameter< std::vector< int > >("randomSeeds",{})};
        setSeeds(seeds); 
        
        // Get the extra simulation configuring commands
        auto postInitCommands = parameters_.getParameter< std::vector< std::string > >("postInitCommands",{});
        for ( const std::string& cmd : postInitCommands ) {
            if ( allowed(cmd) ) {
                int g4Ret = uiManager_->ApplyCommand( cmd );
                if ( g4Ret > 0 ) {
                    EXCEPTION_RAISE(
                            "PostInitCmd",
                            "Post Initialization command '" + cmd + "' returned a failue status from Geant4: " + std::to_string(g4Ret)
                            );
                }
            } else {
                EXCEPTION_RAISE(
                        "PostInitCmd",
                        "Post Initialization command '" + cmd + "' is not allowed because another part of Simulator handles it."
                        );
            }
        }

        // Instantiate the scoring worlds including any parallel worlds. 
        runManager_->ConstructScoringWorlds();

        // Initialize the current run
        runManager_->RunInitialization();

        // Initialize the event processing
        runManager_->InitializeEventLoop( 1 );
        
        return;
    }

    void Simulator::onFileClose(EventFile&) { 
       
        // End the current run and print out some basic statistics if verbose 
        // level > 0.  
        runManager_->TerminateEventLoop();

        // Pass the **real** number of events to the persistency manager
        persistencyManager_->setNumEvents( numEventsBegan_ , numEventsCompleted_ );

        // Persist any remaining events, call the end of run action and 
        // terminate the Geant4 kernel. 
        runManager_->RunTermination();

        // Cleanup persistency manager
        //  Geant4 expects us to handle the persistency manager
        //  In order to avoid segfaulting nonsense, I delete it here
        //  so that it is deleted before the EventFile it references
        //  is deleted
        persistencyManager_.reset( nullptr );
    }

    void Simulator::onProcessEnd() {

        std::cout << "[ Simulator ] : "
            << "Started " << numEventsBegan_ << " events to produce "
            << numEventsCompleted_ << " events."
            << std::endl;
        
        // Delete Run Manager
        // From Geant4 Basic Example B01:
        //      Job termination
        //      Free the store: user actions, physics list and detector descriptions are
        //      owned and deleted by the run manager, so they should not be deleted 
        //      in the main() program 
        // This needs to happen here because otherwise, Geant4 objects are deleted twice:
        //  1. When the histogram file is closed (all ROOT objects created during processing are put there because ROOT)
        //  2. When Simulator is deleted because runManager_ is a unique_ptr
        runManager_.reset( nullptr );

        // Delete the G4UIsession
        // I don't think this needs to happen here, but since we are cleaning up loose ends...
        sessionHandle_.reset( nullptr );
    }

    bool Simulator::allowed(const std::string &command) const {
        for ( const std::string &invalidSubstring : invalidCommands_ ) {
            if ( command.find( invalidSubstring ) != std::string::npos ) {
                //found invalid substring in this command ==> NOT ALLOWED
                return false;
            }
        }
        //checked all invalid commands ==> ALLOWED
        return true;
    }

    void Simulator::setSeeds(std::vector< int > seeds) {

        // If no seeds have been specified then return immediately.
        if (seeds.empty()) return;

        // If seeds are specified, make sure that the container has at least 
        // two seeds.  If not, throw an exception.  
        if (seeds.size() == 1) { 
            EXCEPTION_RAISE("ConfigurationException", "At least two seeds need to be specified.");
        }

        // Create the array of seeds and pass them to G4Random.  Currently, 
        // only 100 seeds can be specified at a time.  If less than 100 
        // seeds are specified, the remaining slots are set to 0. 
        std::vector< long > seedVec(100, 0); 
        for (std::size_t index{0}; index < seeds.size(); ++index) seedVec[index] = static_cast<long>(seeds[index]);

        // Pass the array of seeds to the random engine.
        G4Random::setTheSeeds(&seedVec[0]);  

    }

}

DECLARE_PRODUCER_NS(ldmx, Simulator)
