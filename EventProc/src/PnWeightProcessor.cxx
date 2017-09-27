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
 *   fit variable W = 0.5*(p_tot + K)*(1.12-0.5*(p_z/p))
 *   where p_tot = sqrt(K^2 + 2*K*m)
 *           K = kinetic energy of nucleon at PN vertex
 *           p, p_z = momentum, z-component of nucleon at PN vertex
 */

#include "EventProc/PnWeightProcessor.h"

namespace ldmx {

    const int PnWeightProcessor::PROTON_PDGID = 2212;

    const int PnWeightProcessor::NEUTRON_PDGID = 2112; 

    PnWeightProcessor::PnWeightProcessor(const std::string &name, Process &process) :
        Producer(name, process) { 
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
                
                // Find the nucleon with the greatest kinetic energy   
                if (ke > hardestNucleonKe) { 
                    hardestNucleonKe = ke;
                    hardestNucleonTheta = theta;
                    hardestNucleonW = w;  
                }
            }
        }
        
        // If the W of the hardest nucleon is above the threshold and the
        // polar angle is above the angle threshold, reweight the event. 
        // Otherwise, set the event weight to 1. 
        double eventWeight = 1; 
        if ((hardestNucleonW > wThreshold_) && (hardestNucleonTheta > thetaThreshold_)) {
            eventWeight = 0;   
        }

        result_.setHardestNucleonKe(hardestNucleonKe); 
        result_.setHardestNucleonTheta(hardestNucleonTheta); 
        result_.setHardestNucleonW(hardestNucleonW); 
        result_.setWeight(eventWeight); 

        // Add the result to the collection    
        event.addToCollection("PNweight", result_);

        /* 
        double ke_nucleon = -10., theta_nucleon = -10., w_nucleon = -10., wfit_nucleon = -10., weight_nucleon = 1.;
        double ke_hard = -10., p_hard = -10., pz_hard = -10., w_hard = -10., theta_hard = -10.;
        int A_hard = -1, A_heavy = -1;
        double ke_heavy = -10., p_heavy = -10., pz_heavy = -10., w_heavy = -10., theta_heavy = -10.;
        double ke_dau = -10., p_dau = -10., pz_dau = -10., w_dau = -10., theta_dau = -10.;
        int pdg_dau = -10;

       	SimParticle * sim_nucleon{nullptr}, * sim_hard{0}, * sim_heavy{0}, * sim_dau{0};
        for (int pnDaughterCount = 0; pnDaughterCount < pnGamma->getDaughterCount(); ++pnDaughterCount) { 

            long int nucPrefix = 1000000000;

            // if daughter is nucleus, extract A from pdgID=10LZZZAAAI
            int nucA = (dauID > nucPrefix) ? 
                       (dauID % 10000) / 10 :
                       -1;
            //std::cout << "Found daughter with nucleus weight: " << dauID << " " << nucA << std::endl;

            // hardest nucleus
            if ((dauID > nucPrefix) && (ke > ke_hard)) {
                p_hard = p;
                pz_hard = TMath::Abs(pz);
                ke_hard = ke;
                sim_hard = pnDaughter;
                theta_hard = theta;
                A_hard = nucA;
            }
            // heaviest nucleus
            if ((dauID > nucPrefix) && (nucA > A_heavy)) {
                p_heavy = p;
                pz_heavy = TMath::Abs(pz);
                ke_heavy = ke;
                sim_heavy = pnDaughter;
                theta_heavy = theta;
                A_heavy = nucA;
            }
            // hardest daughter which is not a nucleus
            if ((dauID < nucPrefix) && (ke > ke_dau)) {
                p_dau = p;
                pz_dau = TMath::Abs(pz);
                ke_dau = ke;
                sim_dau = pnDaughter;
                theta_dau = theta;
                pdg_dau = dauID;
            }
        }

        // calculate W, fit value at W, hist value at W, weight value at W...
        if(sim_nucleon) {
          w_nucleon = this->calculateW(sim_nucleon);
          if ((w_nucleon >= wThreshold_ ) && (theta_nucleon > wTheta_)) {
              wfit_nucleon = this->calculateFitW(w_nucleon);
              weight_nucleon = wfit_nucleon/wHist->Interpolate(w_nucleon);
          }
        }
        if(sim_hard){ w_hard = this->calculateW(sim_hard); }
        if(sim_heavy){ w_heavy = this->calculateW(sim_heavy); }
        if(sim_dau){ w_dau = this->calculateW(sim_dau); }

        // Set the resulting weight.
        result_.setResult(
                           ke_nucleon, theta_nucleon, w_nucleon, wfit_nucleon, weight_nucleon,
                           ke_hard, p_hard, pz_hard, w_hard, theta_hard, A_hard,
                           ke_heavy, p_heavy, pz_heavy, w_heavy, theta_heavy, A_heavy,
                           ke_dau, p_dau, pz_dau, w_dau, theta_dau, pdg_dau
                         );
        */

    }

    double PnWeightProcessor::calculateWeight(double w) {
        return exp(8.281 - 0.01092*w)/exp(-1.296-0.001613*w); 
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
