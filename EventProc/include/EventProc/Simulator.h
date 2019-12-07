/**
 * @file Simulator.h
 * @brief Run the G4 simulation inside of ldmx-app
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef EVENTPROC_SIMULATOR_H
#define EVENTPROC_SIMULATOR_H

//LDMX Framework
#include "Event/EventDef.h"
#include "Framework/EventProcessor.h" //Needed to declare processor
#include "Framework/ParameterSet.h" // Needed to import parameters from configuration file
#include "SimApplication/DetectorConstruction.h"
#include "SimApplication/RunManager.h"
#include "SimApplication/SimApplicationMessenger.h"

//Geant4
#include "G4RunManager.hh"
#include "G4GDMLParser.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisManager.hh"
#include "G4VisExecutive.hh"
#include "G4CascadeParameters.hh"


namespace ldmx {
    
    /**
     * @class Simulator
     * @brief Producer that runs Geant4 simulation inside of ldmx-app
     *
     * Most (if not all) of the heavy lifting is done in the classes in the Sim* modules.
     * This producer is mainly focused on calling appropriate functions at the right time in the processing chain.
     */
    class Simulator : public ldmx::Producer {
        public:

            /**
             * Constructor
             *
             * Blank Producer constructor
             * Constructs object that are non-configurable.
             */
            Simulator(const std::string& name, ldmx::Process& process);

            /**
             * Destructor
             *
             * Deletes hanging pointers
             */
            ~Simulator();

            /**
             * Configure the simulation
             *
             * This is called before run is begun, so all parameters/options for simulation must be set here.
             */
            virtual void configure(const ldmx::ParameterSet& ps);

            /**
             * Run simulation and export results to output event
             */
            virtual void produce(ldmx::Event& event);

            /**
             * Initialization of simulation
             *
             * This uses the parameters set in the configure method to construct and initialize the simulation objects.
             */
            virtual void onProcessStart(); 

            /**
             * Tying up of loose ends
             */
            virtual void onProcessEnd();

        private:

            /** Manager controlling G4 simulation run */
            G4RunManager *runManager_{nullptr};

            /** Class to parse GDML Detector Geometry */
            G4GDMLParser *parser_{nullptr};

    };
}

#endif /* EVENTPROC_SIMULATOR_H */
