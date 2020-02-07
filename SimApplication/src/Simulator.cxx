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
            "/ldmx/pw", //parallel world scoring planes is handled here (if passed a path to the scoring plane description)
            "/random/setSeeds", //handled by own config parameter (if passed)
            "EventPrintPlugin", //tied to process log frequency
            "/ldmx/persistency/root", //persistency manager handled directly with python config parameters
            "/ldmx/generators", //handled by own config parameters (if passed)
            "/persistency/gdml/read" //detector description is read after passed a path to the detector description (required)
        };

    Simulator::Simulator(const std::string& name, ldmx::Process& process) : Producer( name , process ) {

        // Get the ui manager from geant
        //      This pointer is handled by Geant4
        uiManager_ = G4UImanager::GetUIpointer();

        
    }

    Simulator::~Simulator() {
        if ( sessionHandle_ ) delete sessionHandle_;
    }

    void Simulator::configure(Parameters& parameters) {
    
        // parameters used to configure the simulation
        parameters_ = parameters; 

        // Instantiate the run manager.  
        runManager_ = std::make_unique<RunManager>(parameters);

        // Instantiate the GDML parser and corresponding messenger
        parser_ = std::make_unique<G4GDMLParser>();

        // Instantiate the class so cascade parameters can be set.
        //      This pointer is handled by Geant4
        G4CascadeParameters::Instance();

        // Supply the default user initialization and actions
        detectorConstruction_ = std::make_unique<DetectorConstruction>( parser_.get(), parameters );
        runManager_->SetUserInitialization( detectorConstruction_.get() );

        // Store the random numbers used to generate an event. 
        runManager_->SetRandomNumberStore( true );
        
        /*************************************************
         * Necessary Parameters
         *************************************************/
        
        detectorPath_ = parameters.getParameter< std::string >("detector");

        runNumber_ = ps.getInteger( "runNumber" );
        //make sure Process uses this run number when creating the event headers
        process_.setRunNumber( runNumber_ );

        /*************************************************
         * Optional Parameters
         *************************************************/
        verbosity_ = parameters.getParameter< int >("verbosity");
        if ( verbosity_ > 0 ) {
            // non-zero verbosity ==> log geant4 comments in files
            //  can input different log file names into this constructor
            sessionHandle_ = new LoggedSession();
        } else {
            // zero verbosity ==> batch run
            sessionHandle_ = new BatchSession();
        }
        uiManager_->SetCoutDestination( sessionHandle_ );

        // Get the path to the scoring planes
        scoringPlanesPath_ = parameters.getParameter< std::string >("scoringPlanes"); 

        randomSeeds_ = parameters.getParameter< std::vector< int > >("randomSeeds");

        // Get the extra simulation configuring commands
        preInitCommands_  = parameters.getParameter< std::vector< std::string > >("preInitCommands" ); 
        postInitCommands_ = parameters.getParameter< std::vector< std::string > >("postInitCommands");

        /*************************************************
         * Do Pre /run/initialize commands
         *************************************************/
        
        // Parse the detector geometry
        if ( verbosity_ > 0 ) {
            std::cout << "[ Simulator ] : Reading in geometry from '" << detectorPath_ << "'... " << std::flush;
        }
        G4GeometryManager::GetInstance()->OpenGeometry();
        parser_->Read( detectorPath_ );
        runManager_->DefineWorldVolume( parser_->GetWorldVolume() );
        if ( verbosity_ > 0 ) {
            std::cout << "done" << std::endl;
        }

        if (!scoringPlanesPath_.empty() ) {
            //path was given, enable and read scoring planes into parallel world
            runManager_->enableParallelWorld(true);
            runManager_->setParallelWorldPath(scoringPlanesPath_);
        }

        for ( const std::string& cmd : preInitCommands_ ) {
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
            event.Print( verbosity_ );
        }

        // Terminate the event.  This checks if an event is to be stored or 
        // stacked for later. 
        runManager_->TerminateOneEvent();
    
        return;
    }
    
    void Simulator::onProcessStart() {
        
        //initialize run
        runManager_->Initialize();

        if ( randomSeeds_.size() > 1 ) {
            //Geant4 allows for random seeds from 2 to 100
            std::string cmd( "/random/setSeeds " );
            for ( const int &seed : randomSeeds_ ) {
                cmd += std::to_string(seed) + " ";
            }
            uiManager_->ApplyCommand( cmd );
        }

        for ( const std::string& cmd : postInitCommands_ ) {
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
    }

    void Simulator::onProcessEnd() {
        /*TODO some annoying warnings about deleting things when geometry is/isn't open at end of run
         * Occur after Simulator::onProcessEnd
         * ~Simulator never called
         * WARNING - Attempt to delete the physical volume store while geometry closed !
         * WARNING - Attempt to delete the logical volume  store while geometry closed !
         * WARNING - Attempt to delete the solid           store while geometry closed !
         * WARNING - Attempt to delete the region          store while geometry closed !
         */
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

    std::string Simulator::getDetectorPath(int version) const {
        
        std::map< int , std::string > versionToName = {
            { 3 , "ldmx-det-full-v3-fieldmap-magnet" },
            { 4 , "ldmx-det-full-v4-fieldmap-magnet" },
            { 5 , "ldmx-det-full-v5-fieldmap-magnet" },
            { 9 , "ldmx-det-full-v9-fieldmap-magnet" },
            { 11 , "ldmx-det-full-v11-fieldmap-magnet" },
            { 12 , "ldmx-det-full-v12-fieldmap-magnet" }
        };

        if ( versionToName.find( version ) == versionToName.end() ) {
            EXCEPTION_RAISE(
                    "DetectorVersion",
                    "Detector Version " + std::to_string(version)
                    + " is not listed in the version to detector name map."
                    );
        }

        std::string detectorDirectory = LDMX_INSTALL;
        detectorDirectory += "/data/detectors/";

        return ( detectorDirectory + versionToName.at(version) + "/detector.gdml" );
    }

}

DECLARE_PRODUCER_NS(ldmx, Simulator)
