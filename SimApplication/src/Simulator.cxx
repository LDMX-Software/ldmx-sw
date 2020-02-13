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
#include "Framework/Process.h"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimApplication/DetectorConstruction.h"
#include "SimApplication/RootPersistencyManager.h" 
#include "SimApplication/PrimaryGeneratorAction.h"
#include "SimApplication/GeneralParticleSource.h"
#include "SimApplication/LHEPrimaryGenerator.h"
#include "SimApplication/MultiParticleGunPrimaryGenerator.h"
#include "SimApplication/RootPrimaryGenerator.h"
#include "SimApplication/RunManager.h"

/*~~~~~~~~~~~~~~*/
/*    Geant4    */
/*~~~~~~~~~~~~~~*/
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

    Simulator::~Simulator() { }

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
        detectorConstruction_ = std::make_unique<DetectorConstruction>( parser_.get() );
        runManager_->SetUserInitialization( detectorConstruction_.get() );

        // Store the random numbers used to generate an event. 
        runManager_->SetRandomNumberStore( true );

        /*************************************************
         * Necessary Parameters
         *************************************************/
        
        detectorPath_ = parameters.getParameter< std::string >("detector");
         
        /*************************************************
         * Optional Parameters
         *************************************************/
        verbosity_ = parameters.getParameter< int >("verbosity");

        enableHitContribs_   = parameters.getParameter< int >("enableHitContribs"); 
        compressHitContribs_ = parameters.getParameter< int >("compressHitContribs");

        dropCollections_ = parameters.getParameter< std::vector< std::string > >("dropCollections");

        // Get the path to the scoring planes
        scoringPlanesPath_ = parameters.getParameter< std::string >("scoringPlanes"); 

        randomSeeds_ = parameters.getParameter< std::vector< int > >("randomSeeds");

        //LHE Generator
        lheFilePath_ = parameters.getParameter< std::string >("lheFilePath"); 

        //ROOT Generator
        rootReSimPath_         = parameters.getParameter< std::string >("rootReSimPath"); 
        rootPrimaryGenRunMode_ = parameters.getParameter< int >("rootPrimaryGenRunMode");
        rootPrimaryGenUseSeed_ = parameters.getParameter< int >("rootPrimaryGenUseSeed"); 

        //Multi Particle Gun
        mpgNparticles_    = parameters.getParameter< int >("mpgNparticles"); 
        mpgEnablePoisson_ = parameters.getParameter< int >("mpgEnablePoisson"); 
        mpgPdgID_         = parameters.getParameter< int >("mpgPdgID"); 
        mpgVertex_        = parameters.getParameter< std::vector< double > >("mpgVertex");    
        mpgMomentum_      = parameters.getParameter< std::vector< double > >("mpgMomentum");

        //Beamspot (all generators)
        beamspotSmear_ = parameters.getParameter< std::vector< double > >("beamspotSmear" );

        // Get the extra simulation configuring commands
        preInitCommands_  = parameters.getParameter< std::vector< std::string > >("preInitCommands" ); 
        postInitCommands_ = parameters.getParameter< std::vector< std::string > >("postInitCommands");
        /*************************************************
         * Do Pre /run/initialize commands
         *************************************************/
        
        // Parse the detector geometry
        G4GeometryManager::GetInstance()->OpenGeometry();
        parser_->Read( detectorPath_ );
        runManager_->DefineWorldVolume( parser_->GetWorldVolume() );

        if ( not scoringPlanesPath_.empty() ) {
            //path was given, enable and read scoring planes into parallel world
            runManager_->enableParallelWorld(true);
            runManager_->setParallelWorldPath(scoringPlanesPath_);
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

        // Initialize persistency manager and connect it to the current EventFile
        persistencyManager_ = std::make_unique<RootPersistencyManager>(file, parameters_); 
        persistencyManager_->Initialize(); 
        
        // pass on the description
        //persistencyManager_->setRunDescription( description_ );
        // pass on the collections to drop after sim (i.e. NOT save)
        // TODO remove this after functional dropping is merged in
        for ( const std::string &collName : dropCollections_ ) persistencyManager_->dropCollection( collName );
        // set how to deal with hit contributions in ECal
        persistencyManager_->setEnableHitContribs( enableHitContribs_ );
        persistencyManager_->setCompressHitContribs( compressHitContribs_ );
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

        //attach generator to runManager
        PrimaryGeneratorAction *primaryGeneratorAction = new PrimaryGeneratorAction;
        runManager_->SetUserAction( primaryGeneratorAction );
        primaryGeneratorAction->setPluginManager( runManager_->getPluginManager() );

        /*************************************************
         * Generator Setup Commands
         *************************************************/

        if ( not lheFilePath_.empty() ) {
            //lhe generator
            primaryGeneratorAction->setPrimaryGenerator(new LHEPrimaryGenerator(new LHEReader(lheFilePath_)));
        } 
        
        if ( not rootReSimPath_.empty() ) {
            //root generator
            RootPrimaryGenerator *rpg = new RootPrimaryGenerator(rootReSimPath_);
            primaryGeneratorAction->setPrimaryGenerator(rpg);
            runManager_->setUseRootSeed( rootPrimaryGenUseSeed_ );

            //TODO break up root resim into two different generators
            rpg->setRunMode( rootPrimaryGenRunMode_ ); //default 1
        } 
        
        if ( mpgNparticles_ > 0 ) {
            MultiParticleGunPrimaryGenerator *mpg = new MultiParticleGunPrimaryGenerator();
            primaryGeneratorAction->setPrimaryGenerator(mpg);

            mpg->setMpgNparticles( mpgNparticles_ ); //default 0 (mpg is off)
            if ( mpgEnablePoisson_ ) mpg->enablePoisson(); //default false
            mpg->setMpgPdgId( mpgPdgID_ ); //default 11
            if ( mpgVertex_.size() == 3 ) {
                mpg->setMpgVertex( 
                            G4ThreeVector( mpgVertex_.at(0)*mm , mpgVertex_.at(1)*mm , mpgVertex_.at(2)*mm )
                            ); //default ( 0. , 0. , 0. )*mm (in middle of target)
            }
            if ( mpgMomentum_.size() == 3 ) {
                mpg->setMpgMomentum( 
                            G4ThreeVector( mpgMomentum_.at(0)*MeV , mpgMomentum_.at(1)*MeV , mpgMomentum_.at(2)*MeV )
                            ); //default ( 0. , 0. , 4000. )*MeV
            }
        } 
        
        if ( enableGeneralParticleSource_ ) {
            primaryGeneratorAction->setPrimaryGenerator(new GeneralParticleSource());
            //other gps commands?
        }

        //beam spot smearing (should work with all generators)
        if ( beamspotSmear_.size() > 1 ) {
            primaryGeneratorAction->setUseBeamspot(true);
            primaryGeneratorAction->setBeamspotXSize( beamspotSmear_.at(0) );
            primaryGeneratorAction->setBeamspotYSize( beamspotSmear_.at(1) );
            if ( beamspotSmear_.size() > 2 ) {
                primaryGeneratorAction->setBeamspotZSize( beamspotSmear_.at(2) );
            }
        }

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
                uiManager_->ApplyCommand( cmd );
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
        std::cout << "onProcessEnd" << std::endl;
        /*TODO some annoying warnings about deleting things when geometry is/isn't open at end of run
         * Occur after Simulator::onProcessEnd
         * ~Simulator never called
         * WARNING - Attempt to delete the physical volume store while geometry closed !
         * WARNING - Attempt to delete the logical volume  store while geometry closed !
         * WARNING - Attempt to delete the solid           store while geometry closed !
         * WARNING - Attempt to delete the region          store while geometry closed !
         */
        std::cout << "Done onProcessEnd" << std::endl;
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
