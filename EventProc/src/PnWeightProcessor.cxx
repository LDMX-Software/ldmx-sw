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

    void PnWeightProcessor::configure(const ParameterSet& pSet) {
        wThreshold_ = pSet.getDouble("w_threshold");
        wTheta_ = pSet.getDouble("w_theta");
        wPdgId_ = 2212; // demoted from pSet. use proton or neutron hists and fit curve.

        // open data files
        TString fName = "$LDMXSW_DIR/data/config/W_hists_Owen_Omar.root";
        TString hName = "W_Omar_5GeV_Pb_";
        hName += (wPdgId_ == 2212) ? "Protons" : "Neutrons";
        wFile = TFile::Open(fName);
        if(!wFile) throw std::invalid_argument(TString::Format("[PnWeightProcessor::configure] Cannot find data file %s",fName.Data()).Data());
        wHist = (TH1F*)wFile->Get(hName);
        if(!wHist) throw std::invalid_argument(TString::Format("[PnWeightProcessor::configure] Cannot hist %s in data file %s",hName.Data(),fName.Data()).Data());
        std::cout << TString::Format("[PnWeightProcessor::configure] Hist %s from data file %s opened successfully",hName.Data(),fName.Data()) << std::endl;
    }

    void PnWeightProcessor::produce(Event& event) {
        result_.Clear();
        bool verb = false;

        // Get the collection of sim particles from the event.  If the 
        // collection of sim particles is empty, don't process the
        // event.
        const TClonesArray *simParticles = event.getCollection("SimParticles");
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
                if(verb) std::cout << "[ pnWeightProcessor ]: Recoil electron found." << std::endl;
                recoilElectron = simParticle; 
                break;
            }
        }

        // Search for the PN gamma and use it to get the PN daughters.
        // TODO: Improve 121 (hadronInelastic) check with processType=="photonNuclear" check
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
        // back-scattered nucleon
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
            // can use == wPdgId_ here for exclusively neutrons or protons
            if ((pnDaughter->getPdgID() == 2212 || pnDaughter->getPdgID() == 2112) && (ke > keNucleon)) {
                keNucleon = ke;
                nucleon = pnDaughter;
                thetaNucleon = theta;  
            }
        }

        if(nucleon && verb) std::cout << TString::Format("Nucleon ke %0.3f, pdgId %d, theta %0.3f", keNucleon, nucleon->getPdgID(), thetaNucleon) << std::endl;
        if (nucleon == nullptr) return; // throw a runtime exception

        // Calculate W
        double w = this->calculateW(nucleon);

        // Calculate weight from W fit
        double weight = 1.0;
        double wFit = 0.0;
        double denomHist = 0.;
        if ((w >= wThreshold_ ) && (thetaNucleon > wTheta_)) {
            wFit = this->calculateFitW(w);
            denomHist = wHist->Interpolate(w);
            weight = wFit/denomHist;
        } 
        if(verb) std::cout << TString::Format("wThreshold_ %0.3f, w %0.6f, wFit %0.6f, denomHist %0.6f, weight %0.9f",
                                               wThreshold_, w, wFit, denomHist, weight) << std::endl;
        if(verb) std::cout << "[ pnWeightProcessor ] : PN weight: " << result_.getWeight() << std::endl;

        // Set the resulting weight.
        result_.setResult(keNucleon, thetaNucleon, w, wFit, weight);

        // Add the result to the collection     
        event.addToCollection("pnWeight", result_);
    }

    double PnWeightProcessor::calculateFitW(double w) {
        if(wPdgId_ == 2212){
          return exp(3.66141e+00+-8.14167e-03*w); // proton curve
        }else if(wPdgId_ == 2112){
          return exp(3.79133e+00+-8.33342e-03*w); // neutron curve
        }else{
          throw std::invalid_argument(TString::Format("[PnWeightProcessor::calculateFitW] invalid wPdgId_ value of %d, expected 2212 or 2112",wPdgId_).Data());
        }
    }

    double PnWeightProcessor::calculateW(SimParticle* particle) {
        double px = particle->getMomentum()[0];
        double py = particle->getMomentum()[1];
        double pz = particle->getMomentum()[2];
        double p = sqrt(px*px + py*py + pz*pz); 
        double ke = particle->getEnergy() - particle->getMass();

        return 0.5*(p + ke)*(1.12 - 0.5*(pz/p));
    }
}

DECLARE_PRODUCER_NS(ldmx, PnWeightProcessor)
