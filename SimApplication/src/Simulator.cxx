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
#include "G4CascadeParameters.hh"
#include "G4GDMLMessenger.hh"
#include "G4GDMLParser.hh"

namespace ldmx {

    Simulator::Simulator(const std::string& name, ldmx::Process& process) : Producer( name , process ) {

        // Instantiate the run manager.  
        runManager_ = std::make_unique<RunManager>();

        // Instantiate the GDML parser and corresponding messenger
        parser_ = std::make_unique<G4GDMLParser>();
        gdmlMessenger_ = std::make_unique<G4GDMLMessenger>(parser_.get()); 

        // Instantiate the class so cascade parameters can be set.
        G4CascadeParameters::Instance();  

        // Supply the default user initialization and actions
        runManager_->SetUserInitialization(new DetectorConstruction( parser_.get() ) );

        // Store the random numbers used to generate an event. 
        runManager_->SetRandomNumberStore( true );
        
    }

    Simulator::~Simulator() {
    }


    void Simulator::configure(const ldmx::ParameterSet& ps) {
      
        // Get the path to the detector description file
        detectorPath_ = ps.getString("detector");

        // Get the path to the macro used to configure the sim
        macroPath_ = ps.getString("macro"); 
    }

    void Simulator::onFileOpen(EventFile &file) {
      
        // Instantiate the persistency manager  
        persistencyManager_ = new RootPersistencyManager(file); 
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
        
        // Terminate the event.  This checks if an event is to be stored or 
        // stacked for later. 
        runManager_->TerminateOneEvent();
    
        return;
    }
    
    void Simulator::onProcessStart() {
        
        // Parse the detector geometry
        uiManager_->ApplyCommand("/persistency/gdml/read " + detectorPath_);

        // Execute the macro
        uiManager_->ApplyCommand("/control/execute " + macroPath_); 
         
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
    }

}

DECLARE_PRODUCER_NS(ldmx, Simulator)
