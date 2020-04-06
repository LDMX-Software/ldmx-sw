/**
 * @file Simulator.cxx
 * @brief Producer that runs Geant4 simulation inside of ldmx-app
 * @author Tom Eichlersmith, University of Minnesota
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimApplication/Simulator.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Process.h"
#include "Event/Version.h" //for LDMX_INSTALL path

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimApplication/DetectorConstruction.h"
#include "SimApplication/RootPersistencyManager.h" 
#include "SimApplication/RunManager.h"
#include "SimApplication/G4Session.h"

/*~~~~~~~~~~~~~~*/
/*    Geant4    */
/*~~~~~~~~~~~~~~*/
#include "G4UIsession.hh"
#include "G4UImanager.hh"
#include "G4CascadeParameters.hh"
#include "G4GeometryManager.hh"
#include "G4GDMLParser.hh"

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
        
        /*************************************************
         * Pass Run Number to Process
         *************************************************/

        int runNumber = parameters_.getParameter< int >( "runNumber" );
        //make sure Process uses this run number when creating the event headers
        process_.setRunNumber( runNumber );

        /*************************************************
         * Verbosity and Logging
         *************************************************/

        verbosity_ = parameters_.getParameter< int >("verbosity");
        auto loggingPrefix = parameters_.getParameter< std::string >("loggingPrefix");
        if ( verbosity_ > 0 or verbosity_ == std::numeric_limits<int>::min() ) {
            // non-zero verbosity ==> log geant4 comments in files
            //  can input different log file names into this constructor
            if ( loggingPrefix.empty() )
                sessionHandle_ = std::make_unique<LoggedSession>();
            else
                sessionHandle_ = std::make_unique<LoggedSession>( loggingPrefix + "_G4cout.log" , loggingPrefix + "_G4cerr.log" );
        } else {
            // zero verbosity ==> batch run
            sessionHandle_ = std::make_unique<BatchSession>();
        }
        uiManager_->SetCoutDestination( sessionHandle_.get() ); //re-direct the G4 messaging service

        /*************************************************
         * Start Configuration of Simulation
         *************************************************/

        // Instantiate the run manager.  
        runManager_ = std::make_unique<RunManager>(parameters);

        // Instantiate the GDML parser and corresponding messenger
        //      owned and managed by DetectorConstruction
        G4GDMLParser *parser = new G4GDMLParser;

        // Instantiate the class so cascade parameters can be set.
        //      This pointer is handled by Geant4
        G4CascadeParameters::Instance();

        // Supply the default user initialization and actions
        //  detector construction owned and managed by RunManager
        runManager_->SetUserInitialization( new DetectorConstruction( parser , parameters ) );

        // Store the random numbers used to generate an event. 
        runManager_->SetRandomNumberStore( true );

        /*************************************************
         * Do Pre /run/initialize commands
         *************************************************/
        
        // Parse the detector geometry
        std::string detectorPath = parameters_.getParameter<std::string>("detector");
        if ( verbosity_ > 0 ) {
            std::cout << "[ Simulator ] : Reading in geometry from '" << detectorPath << "'... " << std::flush;
        }
        G4GeometryManager::GetInstance()->OpenGeometry();
        parser->Read( detectorPath );
        runManager_->DefineWorldVolume( parser->GetWorldVolume() );
        if ( verbosity_ > 0 ) {
            std::cout << "done" << std::endl;
        }

        auto preInitCommands = parameters_.getParameter< std::vector< std::string > >("preInitCommands" ); 
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
        runManager_->ProcessOneEvent( event.getEventHeader().getEventNumber() );

        // If a Geant4 event has been aborted, skip the rest of the processing
        // sequence. This will immediately force the simulation to move on to 
        // the next event. 
        if ( runManager_->GetCurrentEvent()->IsAborted() ) { this->abortEvent(); }
        
        if ( process_.getLogFrequency() > 0 and event.getEventHeader().getEventNumber() % process_.getLogFrequency() == 0 ) {
            //print according to log frequency and verbosity
            if ( verbosity_ > 1 ) std::cout << "[ Simulator ] : Printing event contents:" << std::endl;
            event.Print();
        }

        // Terminate the event.  This checks if an event is to be stored or 
        // stacked for later. 
        runManager_->TerminateOneEvent();
    
        return;
    }
    
    void Simulator::onProcessStart() {
        
        //initialize run
        runManager_->Initialize();

        auto randomSeeds = parameters_.getParameter<std::vector<int>>("randomSeeds");
        if ( randomSeeds.size() > 1 ) {
            //Geant4 allows for random seeds from 2 to 100
            std::string cmd( "/random/setSeeds " );
            for ( const int &seed : randomSeeds ) {
                cmd += std::to_string(seed) + " ";
            }
            uiManager_->ApplyCommand( cmd );
        }

        // Get the extra simulation configuring commands
        auto postInitCommands = parameters_.getParameter< std::vector< std::string > >("postInitCommands");
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

}

DECLARE_PRODUCER_NS(ldmx, Simulator)
