/**
 * @file Simulator.cxx
 * @brief Producer that runs Geant4 simulation inside of ldmx-app
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "EventProc/Simulator.h"

namespace ldmx {

    Simulator::Simulator(const std::string& name, ldmx::Process& process) : Producer( name , process ) {

        runManager_ = new RunManager();

        parser_ = new G4GDMLParser();

        runManager_->SetUserInitialization(new DetectorConstruction( parser_ ) );
        runManager_->SetRandomNumberStore( true );

    }

    Simulator::~Simulator() {

        if ( runManager_ ) delete runManager_;
        if ( parser_     ) delete parser_;

        //Producer destructor called automatically
    }


    void Simulator::configure(const ldmx::ParameterSet& ps) {

        return;
    }

    void Simulator::produce(ldmx::Event& event) {

        return;
    }
    
    void Simulator::onProcessStart() {

        runManager_ = new RunManager();

        parser_ = new G4GDMLParser();

        runManager_->SetUserInitialization(new DetectorConstruction( parser_ ) );
        runManager_->SetRandomNumberStore( true );

        return;
    }

    void Simulator::onProcessEnd() {

        return;
    }

}

DECLARE_PRODUCER_NS(ldmx, Simulator);
