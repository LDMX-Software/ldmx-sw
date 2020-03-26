/**
 * @file SimParticle.cxx
 * @brief Class which implements an MC particle that stores information about 
 *        tracks from the simulation
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Event/SimParticle.h"

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>

ClassImp(ldmx::SimParticle)

namespace ldmx {

    SimParticle::ProcessTypeMap SimParticle::createProcessTypeMap() {
        ProcessTypeMap procMap;
        procMap["eBrem"] = ProcessType::eBrem; /* electron brem */
        procMap["conv"] = ProcessType::conv; /* gamma to e+e- */
        procMap["annihil"] = ProcessType::annihil; /* positron annihilation */
        procMap["compt"] = ProcessType::compt; /* compton scattering */
        procMap["phot"] = ProcessType::phot; /* photoelectric */
        procMap["eIoni"] = ProcessType::eIoni; /* electron ionization */
        procMap["msc"] = ProcessType::msc; /* multiple coulomb scattering */
        procMap["photonNuclear"] = ProcessType::photonNuclear; /* photonuclear */
        procMap["electronNuclear"] = ProcessType::electronNuclear; /* electronuclear*/
        procMap["GammaToMuPair"] = ProcessType::GammaToMuPair; /* gamma to mu+mu- */
        procMap["eDBrem"] = ProcessType::eDarkBrem; /* e- --> A' + e- */
        return procMap;
    }

    SimParticle::ProcessTypeMap SimParticle::PROCESS_MAP = SimParticle::createProcessTypeMap();

    SimParticle::SimParticle() { }

    SimParticle::~SimParticle() {
    }

    void SimParticle::Clear() {

        daughters_.clear();
        parents_.clear();

        energy_ = 0;
        pdgID_ = 0;
        genStatus_ = -1;
        time_ = 0;
        x_ = 0;
        y_ = 0;
        z_ = 0;
        endX_ = 0;
        endY_ = 0;
        endZ_ = 0;
        px_ = 0;
        py_ = 0;
        pz_ = 0;
        endpx_ = 0;
        endpy_ = 0;
        endpz_ = 0;
        mass_ = 0;
        charge_ = 0;
        processType_ = ProcessType::unknown;
    }

    void SimParticle::Print() const {
        std::cout << "SimParticle { " <<
                "energy: " << energy_ << ", " <<
                "PDG ID: " << pdgID_ << ", " <<
                "genStatus: " << genStatus_ << ", " <<
                "time: " << time_ << ", " <<
                "vertex: ( " << x_ << ", " << y_ << ", " << z_ << " ), " <<
                "endPoint: ( " << endX_ << ", " << endY_ << ", " << endZ_ << " ), " <<
                "momentum: ( " << px_ << ", " << py_ << ", " << pz_ << " ), " <<
                "endPointMomentum: ( " << endpx_ << ", " << endpy_ << ", " << endpz_ << " ), " <<
                "mass: " << mass_ << ", " <<
                "nDaughters: " << getDaughterCount() << ", "
                "nParents: " << getParentCount() << ", "
                "processType: " << processType_ <<
                " }" << std::endl;
    }

    SimParticle::ProcessType SimParticle::findProcessType(std::string processName) {

        if (processName.find("biasWrapper") != std::string::npos) { 
            std::size_t pos = processName.find_first_of("(") + 1;
            processName = processName.substr(pos, processName.size() - pos - 1); 
        }  
                
        if (PROCESS_MAP.find(processName) != PROCESS_MAP.end()) {
            return PROCESS_MAP[processName];
        } else {
            return ProcessType::unknown;
        }
    }

}

