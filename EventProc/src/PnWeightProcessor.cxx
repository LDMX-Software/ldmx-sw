/**
 * @file pnWeightProcessor.cxx
 * @brief Processor that calculates pnWeight based on photonNuclear track 
 *        properties.
 * @author Alex Patterson, UCSB
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @note
 * pnWeightProcessor calculates an event weight which is added to the 
 * collection as a pnWeight object. This weight is based on simParticles
 * arising from photonNuclear reactions, and is intended to correct
 * the simulation in the case of high-momentum, backwards-going nucleons
 * arising from those reactions.
 *   fit variable W_p = 0.5*(p_tot + K)*(1.12-0.5*(p_z/p))
 *   where p_tot = sqrt(K^2 + 2*K*m)
 *           K = kinetic energy of nucleon at PN vertex
 *           p, p_z = momentum, z-component of nucleon at PN vertex
 */

#include "EventProc/PnWeightProcessor.h"

namespace ldmx {

    void PnWeightProcessor::configure(const ParameterSet& pSet) {
        wpThreshold_ = pSet.getDouble("wp_threshold");
    }

    void PnWeightProcessor::produce(Event& event) {
        result_.Clear();

        // Get the collection of sim particles from the event.  If the 
        // collection of sim particles is empty, don't process the
        // event.
        const TClonesArray *simParticles = event.getCollection("SimParticles");
        if (simParticles->GetEntriesFast() == 0) return; 

        // Calculate max(Wp) over all daughers of PN reactions
        // nucleon_max = PN daughter with highest Wp
        std::set<double> setWp;
        double wp_max = -1.;
        SimParticle* nucleon_max{nullptr};
        double keNucleon_max{0};
        double thetaNucleon_max{0};
        for(int particleCount = 0; particleCount < simParticles->GetEntriesFast(); ++particleCount){ 
            SimParticle* simParticle = static_cast<SimParticle*>(simParticles->At(particleCount));
            if( (simParticle->getProcessType() == 121) && (simParticle->getParentCount() != 0) ){
                double thisWp = this->calculateWp(simParticle);
                if(thisWp > wp_max) {
                  wp_max = thisWp;
                  double ke = (simParticle->getEnergy() - simParticle->getMass());
                  double px = simParticle->getMomentum()[0];
                  double py = simParticle->getMomentum()[1];
                  double pz = simParticle->getMomentum()[2];
                  double p = sqrt(px*px + py*py + pz*pz);
                  double theta = acos(pz/p)*180.0/3.14159;
                  keNucleon_max = ke;
                  nucleon_max = simParticle;
                  thetaNucleon_max = theta;
                }
                setWp.insert(thisWp);
            }
        }
        double wp_max_set = *setWp.rbegin();
        //std::cout << TString::Format("wp_max %.3f, keNucleon_max %.3f, thetaNucleon_max %.3f", wp_max, keNucleon_max, thetaNucleon_max) << std::endl;

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

        // Search for the PN gamma and use it to get the PN daughters.
        SimParticle* pnGamma{nullptr};
        for (int daughterCount = 0; daughterCount < recoilElectron->getDaughterCount(); ++daughterCount) {
            SimParticle* daughter = recoilElectron->getDaughter(daughterCount);
            if ((daughter->getDaughterCount() > 0) 
                &&(daughter->getDaughter(0)->getProcessType() == 121)) {
                pnGamma = daughter; 
                break;
            }
        }
        
        // For PN biased events, there should always be a gamma that 
        // underwent a PN reaction.
        if (pnGamma == nullptr) return; // throw a runtime exception 
        
        // Loop over all PN daughters and find the highest energy 
        // back-scattered neutron and proton
       	SimParticle* nucleon{nullptr}, *nucleon_p{nullptr};
        double keNucleon{0}, keNucleon_p{0};
        double thetaNucleon{0}, thetaNucleon_p{0}; 
        for (int pnDaughterCount = 0; pnDaughterCount < pnGamma->getDaughterCount(); ++pnDaughterCount) { 
            SimParticle* pnDaughter = pnGamma->getDaughter(pnDaughterCount);
            double ke = (pnDaughter->getEnergy() - pnDaughter->getMass());
            double px = pnDaughter->getMomentum()[0];
            double py = pnDaughter->getMomentum()[1];
            double pz = pnDaughter->getMomentum()[2];
            double p = sqrt(px*px + py*py + pz*pz); 
            double theta = acos(pz/p)*180.0/3.14159;
            if ((pnDaughter->getPdgID() == 2112) && (ke > keNucleon)) { 
                keNucleon = ke;
                nucleon = pnDaughter;
                thetaNucleon = theta;  
            }
            if ((pnDaughter->getPdgID() == 2212) && (ke > keNucleon)) { 
                keNucleon_p = ke;
                nucleon_p = pnDaughter;
                thetaNucleon_p = theta;  
            }
        }

        if (nucleon == nullptr) return; // throw a runtime exception
        //if (nucleon_p == nullptr) return; // throw a runtime exception

        // Calculate W_p
        double wp = this->calculateWp(nucleon); 
        double wp_p = (nucleon_p != nullptr) ? this->calculateWp(nucleon_p) : -1;

        // Calculate weight from Wp fit
        // neutrons
        double weight = 1.0;
        double wpFit = 0.0;
        if ((wp >= wpThreshold_ ) && (thetaNucleon > 100)) {
            wpFit = this->calculateFitWp(wp);  
            weight = wpFit/wp; 
        } 
        //std::cout << "[ pnWeightProcessor ] : PN weight: " << result_.getWeight() << std::endl;
        // protons
        double weight_p = 1.0;
        double wpFit_p = 0.0;
        if ((wp_p >= wpThreshold_ ) && (thetaNucleon_p > 100)) {
            wpFit_p = this->calculateFitWp(wp_p);  
            weight_p = wpFit_p/wp_p; 
        }
        // max
        double weight_max = 1.0;
        double wpFit_max = 0.0;
        if ((wp_max >= wpThreshold_ ) && (thetaNucleon_max > 100)) {
            wpFit_max = this->calculateFitWp(wp_max);  
            weight_max = wpFit_max/wp_max; 
        }

        //std::cout << TString::Format("wp_max %.3f wpFit_p %.3f keNucleon_p %.3f wp_p %.3f thetaNucleon_p %.3f weight_p %.3f wpFit_max %.3f weight_max %.3f",
        //                              wp_max,     wpFit_p,     keNucleon_p,     wp_p,     thetaNucleon_p,     weight_p,     wpFit_max,     weight_max) << std::endl;

        // Set the resulting weight.
        result_.setResult(wpFit, keNucleon, wp, thetaNucleon, weight, wpFit_max, keNucleon_max, wp_max, thetaNucleon_max, weight_max, wpFit_p, keNucleon_p, wp_p, thetaNucleon_p, weight_p);

        // Add the result to the collection     
        event.addToCollection("pnWeight", result_);
    }

    double PnWeightProcessor::calculateFitWp(double wp) { 
        return 1.78032e+04*exp(-8.07561e-03*(wp - 7.91244e+02)); 
    }

    double PnWeightProcessor::calculateWp(SimParticle* particle) {
        double px = particle->getMomentum()[0];
        double py = particle->getMomentum()[1];
        double pz = particle->getMomentum()[2];
        double p = sqrt(px*px + py*py + pz*pz); 
        double ke = particle->getEnergy() - particle->getMass();

        return 0.5*(p + ke)*(1.12 - 0.5*(pz/p));
    }
}

DECLARE_PRODUCER_NS(ldmx, PnWeightProcessor)
