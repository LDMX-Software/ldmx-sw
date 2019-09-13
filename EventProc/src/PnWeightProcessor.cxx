/**
 * @file PnWeightProcessor.cxx
 * @brief Processor that calculates an event weight based on the kinematics of 
 *        Photonuclear event.
 * @author Alex Patterson, UCSB
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @note
 * PnWeightProcessor calculates an event weight which is persisted as a PnWeight
 * object. This weight is based on the kinematics of a photonuclear reaction and 
 * is intended to correct for the overproduction of events with high-momentum,
 * backwards-going nucleons.  A weight is assigned to the event if the variable
 *      W = 0.5*(p_tot + K)*(1.12-0.5*(p_z/p))
 *          p_tot = sqrt(K^2 + 2*K*m),
 *          K = kinetic energy of nucleon at PN vertex
 *          p, p_z = momentum, z-component of nucleon at PN vertex
 * is above some threshold and the hardest nucleon in the event has a polar 
 * angle > 100 degrees.  The W variable is calculated using the hardest 
 * nucleon in the event.
 */

#include "EventProc/PnWeightProcessor.h"

namespace ldmx {

    const int PnWeightProcessor::PROTON_PDGID = 2212;

    const int PnWeightProcessor::NEUTRON_PDGID = 2112; 

    PnWeightProcessor::PnWeightProcessor(const std::string &name, Process &process) :
        Producer(name, process) {

            lFit = std::make_unique<TF1>( "lfit" , "exp([1]-[0]*x)" , 950 , 1150 );
            lFit->SetParameters(0.01093, 2.766); 

            std::string func = "exp([1]-[0]*x)*([2]+[3]*x + [4]*pow(x,2)"; 
            func += " + [5]*pow(x,3) + [6]*pow(x,4) + [7]*pow(x,5) + [8]*pow(x,6))"; 
            hFit = std::make_unique<TF1>("hfit", func.c_str(), 1150, 3700);
            hFit->SetParameters(0.004008, 11.23, 1.242e-6, -1.964e-9, 
                    8.243e-13, 9.333e-17, -7.584e-20, -1.991e-23, 9.757e-27);
    }

    PnWeightProcessor::~PnWeightProcessor() { 
    }

    void PnWeightProcessor::configure(const ParameterSet& pSet) {
        wThreshold_ = pSet.getDouble("w_threshold");
        thetaThreshold_ = pSet.getDouble("theta_threshold");
    }

    void PnWeightProcessor::produce(Event& event) {
        
        result_.Clear();

        // Get the collection of sim particles from the event.  If the 
        // collection of sim particles is empty, don't process the
        // event.
        const TClonesArray* simParticles = event.getCollection("SimParticles");
        if (simParticles->GetEntriesFast() == 0) return; 

        // Loop through all of the particles and search for the recoil electron
        // i.e. an electron which doesn't have any parents.
        SimParticle* recoilElectron{nullptr};
        for (int particleCount = 0; particleCount < simParticles->GetEntriesFast(); ++particleCount) { 
            
            // Get the nth particle from the collection of particles
            SimParticle* simParticle = static_cast<SimParticle*>(simParticles->At(particleCount));

            // If the particle doesn't correspond to the recoil electron, 
            // continue to the next particle.
            if ((simParticle->getPdgID() == 11) && (simParticle->getParentCount() == 0)) {
                //std::cout << "[ pnWeightProcessor ]: Recoil electron found." << std::endl;
                recoilElectron = simParticle; 
                break;
            }
        }
        /*std::cout << "[ PnWeightProcessor ]: Recoil electron total daughters: " 
                  << recoilElectron->getDaughterCount() 
                  << std::endl;*/

        // Search for the PN gamma and use it to get the PN daughters.
        SimParticle* pnGamma{nullptr};
        for (int daughterCount = 0; daughterCount < recoilElectron->getDaughterCount(); ++daughterCount) {
            SimParticle* daughter = recoilElectron->getDaughter(daughterCount);
            /*std::cout << "[ PnWeightProcessor ]: Total daughters: "
                      << daughter->getDaughterCount() 
                      << std::endl;*/
            if ((daughter->getDaughterCount() > 0) && 
                    (daughter->getDaughter(0)->getProcessType() == SimParticle::ProcessType::photonNuclear)) {
                /*std::cout << "[ PnWeightProcessor ]: Found PN gamma!"
                          << std::endl;*/
                pnGamma = daughter; 
                break;
            }
        }

        // For PN biased events, there should always be a gamma that
        // underwent a PN reaction.
        if (pnGamma == nullptr) {
            throw std::runtime_error("[ PnWeightProcessor ]: Event doesn't contain a PN Gamma."); 
        }

        double hardestNucleonKe = -9999;
        double hardestNucleonTheta = -9999;
        double hardestNucleonW = -9999; 
        double highestWNucleonKe = -9999;
        double highestWNucleonTheta = -9999;
        double highestWNucleonW = -9999; 
        for (int pnDaughterCount = 0; pnDaughterCount < pnGamma->getDaughterCount(); ++pnDaughterCount) { 
           
            // Get a daughter of the PN gamma 
            SimParticle* pnDaughter = pnGamma->getDaughter(pnDaughterCount);

            // Calculate the kinetic energy
            double ke = (pnDaughter->getEnergy() - pnDaughter->getMass());

            // Calculate the momentum
            double px = pnDaughter->getMomentum()[0];
            double py = pnDaughter->getMomentum()[1];
            double pz = pnDaughter->getMomentum()[2];
            double p = sqrt(px*px + py*py + pz*pz); 

            // Calculate the polar angle
            double theta = acos(pz/p)*180.0/3.14159;

            // Get the PDG ID of the daughter
            long int pdgID = std::abs(pnDaughter->getPdgID());

            double w = this->calculateW(pnDaughter); 

            // Check if the daughter particle is a proton or neutron
            if ((pdgID == PROTON_PDGID) || (pdgID == NEUTRON_PDGID)) { 

                // Add the W of the current nucleon to the inclusive collection.
                result_.addW(w);
                
                // Add the theta of the current nucleon to the inclusive collection. 
                result_.addTheta(theta); 

                // Find the nucleon with the greatest kinetic energy   
                if (ke > hardestNucleonKe) { 
                    hardestNucleonKe = ke;
                    hardestNucleonTheta = theta;
                    hardestNucleonW = w;  
                }
                
                // Find the nucleon with the highest W 
                if (w > highestWNucleonW) { 
                    highestWNucleonKe = ke;
                    highestWNucleonTheta = theta;
                    highestWNucleonW = w;  
                }
            }
        }
       
        // If the W of the highest W nucleon is above the threshold and the
        // polar angle is above the angle threshold, reweight the event. 
        // Otherwise, set the event weight to 1. 
        double eventWeight = 1; 
        if ((highestWNucleonW > wThreshold_) && (highestWNucleonTheta > thetaThreshold_)) {
            eventWeight = calculateWeight(highestWNucleonW);   
        }

        result_.setHardestNucleonKe(hardestNucleonKe); 
        result_.setHardestNucleonTheta(hardestNucleonTheta); 
        result_.setHardestNucleonW(hardestNucleonW); 
        result_.setHighestWNucleonKe(highestWNucleonKe); 
        result_.setHighestWNucleonTheta(highestWNucleonTheta); 
        result_.setHighestWNucleonW(highestWNucleonW); 
        result_.setWeight(eventWeight); 

        // Add the result to the collection    
        event.addToCollection("PNweight", result_);
    }

    double PnWeightProcessor::calculateWeight(double w) {
        return lFit->Eval(w)/hFit->Eval(w); 
    }

    double PnWeightProcessor::calculateW(SimParticle* particle, double delta) {
        double px = particle->getMomentum()[0];
        double py = particle->getMomentum()[1];
        double pz = particle->getMomentum()[2];
        double p = sqrt(px*px + py*py + pz*pz); 
        double ke = particle->getEnergy() - particle->getMass();

        return 0.5*(p + ke)*(sqrt(1 + (delta*delta)) - delta*(pz/p));
    }
}

DECLARE_PRODUCER_NS(ldmx, PnWeightProcessor)
