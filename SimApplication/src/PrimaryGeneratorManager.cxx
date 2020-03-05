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
                [this] (auto gen) {
                    std::cout << "Generator: " << gen << std::endl;
                    if (gen.compare("gun") == 0) {
                        generators_.push_back(new ParticleGun());  
                    } else {
                        EXCEPTION_RAISE("UnknownGenerator", 
                                "A generator of type " + gen + " doesn't exists."); 
                    }
                }
        );
    }

} // ldmx
