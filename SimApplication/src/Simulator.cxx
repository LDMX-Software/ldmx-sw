/**
 * @file Simulator.cxx
 * @brief Producer that runs Geant4 simulation inside of ldmx-app
 * @author Tom Eichlersmith, University of Minnesota
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimApplication/Simulator.h"

#include "Framework/ParameterSet.h" 
#include "SimApplication/DetectorConstruction.h"
#include "SimApplication/RootPersistencyManager.h" 
#include "SimApplication/RunManager.h"

#include "G4CascadeParameters.hh"
#include "G4GDMLParser.hh"
#include "G4GDMLMessenger.hh"

namespace ldmx {

    Simulator::Simulator(const std::string& name, ldmx::Process& process) : Producer( name , process ) {

        // Instantiate the run manager.  
        runManager_ = std::make_unique<RunManager>();

        // Instantiate the GDML parser and corresponding messenger
        parser_ = std::make_unique<G4GDMLParser>();
        gdmlMessenger_ = std::make_unique<G4GDMLMessenger>(parser_.get()); 

        // Instantiate the class so cascade parameters can be set.
        G4CascadeParameters::Instance();  

        runManager_->SetUserInitialization(new DetectorConstruction( parser_.get() ) );

        runManager_->SetRandomNumberStore( true );
        
        /*
        new SimApplicationMessenger();
        */
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
       
        persistencyManager_ = new RootPersistencyManager(file); 
        persistencyManager_->Initialize(); 
    }

    void Simulator::produce(ldmx::Event& event) {

        persistencyManager_->setCurrentEvent(&event); 
        runManager_->ProcessOneEvent( iEvent_++ );
        runManager_->TerminateOneEvent();
    
        return;
    }
    
    void Simulator::onProcessStart() {
        
        // Parse the detector geometry
        uiManager_->ApplyCommand("/persistency/gdml/read " + detectorPath_);

        // Execute the macro
        uiManager_->ApplyCommand("/control/execute " + macroPath_); 
         

        //runManager_->ConstructScoringWorlds();
        runManager_->RunInitialization();
        runManager_->InitializeEventLoop( 1 );


        return;
    }

    void Simulator::onFileClose(EventFile& eventFile) { 
        
        runManager_->TerminateEventLoop();
        runManager_->RunTermination();
    }

    void Simulator::onProcessEnd() {
    }

}

DECLARE_PRODUCER_NS(ldmx, Simulator)
