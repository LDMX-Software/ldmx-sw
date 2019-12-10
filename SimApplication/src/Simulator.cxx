/**
 * @file Simulator.cxx
 * @brief Producer that runs Geant4 simulation inside of ldmx-app
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimApplication/Simulator.h"

namespace ldmx {

    Simulator::Simulator(const std::string& name, ldmx::Process& process) : Producer( name , process ) {

        runManager_ = new RunManager();

        parser_ = new G4GDMLParser();
        gdmlMessenger_ = new G4GDMLMessenger(parser_);

        new SimApplicationMessenger();

        // Instantiate the class so cascade parameters can be set.
        G4CascadeParameters::Instance();  

        runManager_->SetUserInitialization(new DetectorConstruction( parser_ ) );
        runManager_->SetRandomNumberStore( true );

        uiManager_ = G4UImanager::GetUIpointer();

        persistencyManager_ = new RootPersistencyManager();
        persistencyManager_->SetVerboseLevel( 3 );
    }

    Simulator::~Simulator() {

        if ( runManager_ ) delete runManager_;
        if ( parser_     ) delete parser_;
        if ( persistencyManager_ ) delete persistencyManager_;

        //Producer destructor called automatically
    }


    void Simulator::configure(const ldmx::ParameterSet& ps) {
        return;
    }

    void Simulator::produce(ldmx::Event& event) {

        runManager_->ProcessOneEvent( iEvent_++ );
        runManager_->GetCurrentEvent()->Print();
//        persistencyManager_->buildEvent( runManager_->GetCurrentEvent() , &event );
        persistencyManager_->writeHitsCollections( runManager_->GetCurrentEvent() , &event );
        runManager_->TerminateOneEvent();

        return;
    }
    
    void Simulator::onProcessStart() {

        iEvent_ = 0;

        //read in detector from gdml
        uiManager_->ApplyCommand( "/persistency/gdml/read detector.gdml" );
        //biasing would happen here

        uiManager_->ApplyCommand( "/run/initialize" );

        //load plugins here

        //setup basic particle gun
        uiManager_->ApplyCommand("/gun/particle e-");
        uiManager_->ApplyCommand("/gun/energy 4.0 GeV");
        uiManager_->ApplyCommand("/gun/position 0 0 -.55 mm");
        uiManager_->ApplyCommand("/gun/direction 0 0 4 GeV");

        //turn off writing to output file

        runManager_->ConstructScoringWorlds();
        runManager_->RunInitialization();
        runManager_->InitializeEventLoop( 1 );
        runManager_->Initialize();

        persistencyManager_->setupHitsCollectionMap();

        return;
    }

    void Simulator::onProcessEnd() {

        runManager_->TerminateEventLoop();
        runManager_->RunTermination();

        return;
    }

}

DECLARE_PRODUCER_NS(ldmx, Simulator);
