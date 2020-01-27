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
#include "Framework/ParameterSet.h" 

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimApplication/DetectorConstruction.h"
#include "SimApplication/RootPersistencyManager.h" 
#include "SimApplication/RunManager.h"

/*~~~~~~~~~~~~~~*/
/*    Geant4    */
/*~~~~~~~~~~~~~~*/
#include "G4UImanager.hh"
#include "G4CascadeParameters.hh"
#include "G4GDMLMessenger.hh"
#include "G4GDMLParser.hh"

namespace ldmx {

    const std::vector<std::string> Simulator::invalidCommands_ = {
            "/run/initialize", //hard coded at the right time
            "/run/beamOn", //passed commands should only be sim setup
            "/ldmx/pw", //parallel world scoring planes is handled here (if passed a path to the scoring plane description)
            "/random/setSeeds", //handled by own config parameter (if passed)
            "EventPrintPlugin", //tied to process log frequency
            "/ldmx/persistency/root", //persistency messenger not operational right now (I believe)
            "/ldmx/generators/beamspot", //handled by own config parameter (if passed)
            "/persistency/gdml/read" //detector description is read after passed a path to the detector description (required)
        };

    Simulator::Simulator(const std::string& name, ldmx::Process& process) : Producer( name , process ) {

        // Get the ui manager from geant
        uiManager_ = G4UImanager::GetUIpointer();

        // Instantiate the run manager.  
        runManager_ = std::make_unique<RunManager>();

        // Instantiate the GDML parser and corresponding messenger
        parser_ = std::make_unique<G4GDMLParser>();
        gdmlMessenger_ = std::make_unique<G4GDMLMessenger>(parser_.get()); 

        // Instantiate the class so cascade parameters can be set.
        cascadeParameters_ = G4CascadeParameters::Instance();  

        // Supply the default user initialization and actions
        detectorConstruction_ = std::make_unique<DetectorConstruction>( parser_.get() );
        runManager_->SetUserInitialization( detectorConstruction_.get() );

        // Store the random numbers used to generate an event. 
        runManager_->SetRandomNumberStore( true );
        
    }

    Simulator::~Simulator() {
        if ( cascadeParameters_ ) delete cascadeParameters_;
        if ( uiManager_         ) delete uiManager_;
    }


    void Simulator::configure(const ldmx::ParameterSet& ps) {
      
        description_ = ps.getString( "description" );

        // Get the path to the detector description file
        detectorPath_ = ps.getString( "detector" );

        runNumber_ = ps.getInteger( "runNumber" );

        // Get the path to the scoring planes
        scoringPlanesPath_ = ps.getString( "scoringPlanes" , "" );

        randomSeeds_ = ps.getVInteger( "randomSeeds" , { } );
        beamspotSmear_ = ps.getVDouble( "beamspotSmear" , { } );

        // Get the simulation configuring commands
        preInitCommands_  = ps.getVString( "preInitCommands"  , { } );
        postInitCommands_ = ps.getVString( "postInitCommands" , { } );

        // Parse the detector geometry
        uiManager_->ApplyCommand( "/persistency/gdml/read " + detectorPath_ );

        if ( scoringPlanesPath_ != "" ) {
            //path was given, enable and read scoring planes into parallel world
            uiManager_->ApplyCommand( "/ldmx/pw/enable" );
            uiManager_->ApplyCommand( "/ldmx/pw/read " + scoringPlanesPath_ );
        }

        for ( const std::string& cmd : preInitCommands_ ) {
            if ( allowed(cmd) ) {
                uiManager_->ApplyCommand( cmd );
            } else {
                EXCEPTION_RAISE(
                        "PreInitCmd",
                        "Pre Initialization command '" + cmd + "' is not allowed because another part of Simulator handles it."
                        );
            }
        }
    }

    void Simulator::onFileOpen(EventFile &file) {
       
        persistencyManager_ = std::make_unique<RootPersistencyManager>(file); 
        persistencyManager_->Initialize(); 
        persistencyManager_->setRunNumber( runNumber_ );
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
        
        // Terminate the event.  This checks if an event is to be stored or 
        // stacked for later. 
        runManager_->TerminateOneEvent();
    
        return;
    }
    
    void Simulator::onProcessStart() {
        
        // Parse the detector geometry
        uiManager_->ApplyCommand("/persistency/gdml/read " + detectorPath_);

        //initialize run
        uiManager_->ApplyCommand( "/run/initialize" );

        for ( const std::string& cmd : postInitCommands_ ) {
            if ( allowed(cmd) ) {
                uiManager_->ApplyCommand( cmd );
            } else {
                EXCEPTION_RAISE(
                        "PostInitCmd",
                        "Post Initialization command '" + cmd + "' is not allowed because another part of Simulator handles it."
                        );
            }
        }

        if ( beamspotSmear_.size() == 2 ) {
            uiManager_->ApplyCommand( "/ldmx/generators/beamspot/enable" );
            uiManager_->ApplyCommand( "/ldmx/generators/beamspot/sizeX " + std::to_string(beamspotSmear_.at(0)) );
            uiManager_->ApplyCommand( "/ldmx/generators/beamspot/sizeY " + std::to_string(beamspotSmear_.at(1)) );
        }

        if ( process_.getLogFrequency() > 0 ) {
            uiManager_->ApplyCommand( "/ldmx/plugins/load EventPrintPlugin" );
            uiManager_->ApplyCommand( "/ldmx/plugins/EventPrintPlugin/modulus " + std::to_string(process_.getLogFrequency()) );
            uiManager_->ApplyCommand( "/ldmx/plugins/EventPrintPlugin/prepend \"[ Simulator ] : \"" );
            uiManager_->ApplyCommand( "/ldmx/plugins/EventPrintPlugin/append \"\"" );
        }

        if ( randomSeeds_.size() == 2 ) {
            uiManager_->ApplyCommand( "/random/setSeeds " + std::to_string(randomSeeds_.at(0)) 
                    + " " + std::to_string(randomSeeds_.at(1)) );
        }

        // Instantiate the scoring worlds including any parallel worlds. 
        runManager_->ConstructScoringWorlds();

        // Initialize the current run
        runManager_->RunInitialization();

        // Initialize the event processing
        runManager_->InitializeEventLoop( 1 );
        
        return;
    }

    void Simulator::onFileClose(EventFile& eventFile) { 
       
        // End the current run and print out some basic statistics if verbose 
        // level > 0.  
        runManager_->TerminateEventLoop();

        // Persist any remaining events, call the end of run action and 
        // terminate the Geant4 kernel. 
        runManager_->RunTermination();
    }

    void Simulator::onProcessEnd() {
        /*TODO some annoying warnings about deleting things when geometry is/isn't open at end of run
         * WARNING - Attempt to delete the physical volume store while geometry closed !
         * WARNING - Attempt to delete the logical volume store while geometry closed !
         * WARNING - Attempt to delete the solid store while geometry closed !
         * WARNING - Attempt to delete the region store while geometry closed !
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

}

DECLARE_PRODUCER_NS(ldmx, Simulator)
