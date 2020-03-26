/**
 * @file PrimaryGeneratorManager.cxx
 * @brief Class that manages the generators used to fire particles. 
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimApplication/PrimaryGeneratorManager.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <algorithm> 
#include <string>
#include <vector>

/*~~~~~~~~~~~~~~~*/
/*   Exception   */
/*~~~~~~~~~~~~~~~*/
#include "Exception/Exception.h" 

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimApplication/GeneralParticleSource.h"
#include "SimApplication/LHEPrimaryGenerator.h"
#include "SimApplication/LHEReader.h"
#include "SimApplication/MultiParticleGunPrimaryGenerator.h"
#include "SimApplication/RootPrimaryGenerator.h"
#include "SimApplication/ParticleGun.h"

namespace ldmx { 

    PrimaryGeneratorManager::PrimaryGeneratorManager(Parameters& parameters) {

        // Initialize the primary generators 
        initialize(parameters); 
    }

    PrimaryGeneratorManager::~PrimaryGeneratorManager() {}

    void PrimaryGeneratorManager::initialize(Parameters& parameters) {
    
        // Get the list of generators to initialize
        auto genList{parameters.getParameter< std::vector < std::string > >("generators")};
     
        // If the list of generators is empty, throw an exception.
        if (genList.empty()) { 
            EXCEPTION_RAISE("MissingGenerator", "A generator needs to be specified."); 
        }

        std::for_each( genList.begin(), genList.end(), 
                [&parameters, this] (auto gen) {
                    if (gen.compare("gun") == 0) {
                        generators_.push_back(new ParticleGun(parameters));  
                    } else if (gen.compare("lhe") == 0) {
                        auto lheFilePath{parameters.getParameter< std::string >("lheFilePath")}; 
                        generators_.push_back( new LHEPrimaryGenerator( new LHEReader(lheFilePath) )); 
                    } else if (gen.compare("root") == 0) {
                        
                        auto rootResimPath{parameters.getParameter< std::string >("rootResimPath")}; 
                        auto rpg{new RootPrimaryGenerator(rootResimPath)}; 
                        generators_.push_back(rpg);
            
                        //TODO break up root resim into two different generators
                        auto rootPrimaryGenRunMode{parameters.getParameter< int >("rootPrimaryGenRunMode")}; 
                        if (rootPrimaryGenRunMode < 0) rootPrimaryGenRunMode = 1; 
                        rpg->setRunMode( rootPrimaryGenRunMode ); //default 1

                    } else if (gen.compare("gps") == 0) {

                        // TODO: There are too many GPS commands to port into 
                        //       the framework.  For now, the config should 
                        //       be put into a macro.   
                        generators_.push_back(new GeneralParticleSource());

                    } else if (gen.compare("multi") == 0) {
                         
                        //Multi Particle Gun
                        if (auto mpgNparticles{parameters.getParameter< int >("mpg.nParticles")}; 
                                mpgNparticles > 0) {
                            auto mpg{new MultiParticleGunPrimaryGenerator()}; 
                            mpg->setMpgNparticles(mpgNparticles); 
                            
                            if (auto enablePoisson{parameters.getParameter< bool >("mpg.enablePoisson")}; 
                                    enablePoisson) mpg->enablePoisson(); 

                            if (auto mpgPdgID{parameters.getParameter< int >("mpg.pdgID")};
                                   mpgPdgID > 0) mpg->setMpgPdgId( mpgPdgID );

                            if (auto mpgVertex{parameters.getParameter< std::vector< double > >("mpg.vertex")};   
                                    !mpgVertex.empty()) { 
                                mpg->setMpgVertex(G4ThreeVector( mpgVertex[0]*mm , mpgVertex[1]*mm , mpgVertex[2]*mm )); 
                            }

                            if (auto mpgMomentum{ parameters.getParameter< std::vector< double > >("mpg.p")};
                                    !mpgMomentum.empty()) {
                                mpg->setMpgMomentum( G4ThreeVector( mpgMomentum[0]*MeV , mpgMomentum[1]*MeV , mpgMomentum[2]*MeV )); 
                            }

                            generators_.push_back(mpg); 

                        }
                    } else if (gen.compare("stdhep") == 0) { 
                        EXCEPTION_RAISE("NotImplemented", 
                                "Generator has not been implemented."); 
                    }
                    else {
                        EXCEPTION_RAISE("UnknownGenerator", 
                                "A generator of type " + gen + " doesn't exists."); 
                    }
                }
        );
    }

} // ldmx
