/**
 * @file Simulator.h
 * @brief Run the G4 simulation inside of ldmx-app
 * @author Tom Eichlersmith, University of Minnesota
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_SIMULATOR_H
#define SIMAPPLICATION_SIMULATOR_H

//~~~~~~~~~~~~//
//   StdLib   //
//~~~~~~~~~~~~//
#include <memory>

//~~~~~~~~~~~~~//
//   ldmx-sw   //
//~~~~~~~~~~~~~//
#include "Event/EventDef.h"
#include "Framework/EventProcessor.h"
#include "Framework/ParameterSet.h" 

//~~~~~~~~~~~~//
//   Geant4   //
//~~~~~~~~~~~~//
#include "G4UImanager.hh"

class G4RunManager;
class G4GDMLParser; 
class G4GDMLMessenger; 

namespace ldmx {

    class RunManager; 

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
             * This is called before run is begun, so all parameters/options 
             * for simulation must be set here.
             */
            virtual void configure(const ldmx::ParameterSet& ps);

            /**
             * Run simulation and export results to output event
             */
            virtual void produce(ldmx::Event& event);

            /**
             * Initialization of simulation
             *
             * This uses the parameters set in the configure method to 
             * construct and initialize the simulation objects.
             */
            virtual void onProcessStart(); 

            /**
             * Tying up of loose ends
             */
            virtual void onProcessEnd();

        private:

            /// Manager controlling G4 simulation run 
            std::unique_ptr<G4RunManager> runManager_;

            /// User interface handle
            G4UImanager* uiManager_{G4UImanager::GetUIpointer()};

            /// GDML parser 
            std::unique_ptr<G4GDMLParser> parser_;

            /// Messenger that allows passing commands to the parser
            std::unique_ptr<G4GDMLMessenger> gdmlMessenger_; 

            /// Index of current event
            unsigned int iEvent_{0};

            /// Path to detector description
            std::string detectorPath_{""};

            /// Macro path
            std::string macroPath_{""};  
    };
}

#endif /* SIMAPPLICATION_SIMULATOR_H */
