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

    PnWeightProcessor::PnWeightProcessor(const std::string &name, Process &process) :
        Producer(name, process) { 
    }

    PnWeightProcessor::~PnWeightProcessor() { 
    }

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
        if (pnGamma == nullptr) return; // throw a runtime exception 
        
        double ke_nucleon = -10., theta_nucleon = -10., w_nucleon = -10., wfit_nucleon = -10., weight_nucleon = 1.;
        double ke_hard = -10., p_hard = -10., pz_hard = -10., w_hard = -10., theta_hard = -10.;
        int A_hard = -1, A_heavy = -1;
        double ke_heavy = -10., p_heavy = -10., pz_heavy = -10., w_heavy = -10., theta_heavy = -10.;
        double ke_dau = -10., p_dau = -10., pz_dau = -10., w_dau = -10., theta_dau = -10.;
        int pdg_dau = -10;

       	SimParticle * sim_nucleon{nullptr}, * sim_hard{0}, * sim_heavy{0}, * sim_dau{0};
        for (int pnDaughterCount = 0; pnDaughterCount < pnGamma->getDaughterCount(); ++pnDaughterCount) { 
            SimParticle* pnDaughter = pnGamma->getDaughter(pnDaughterCount);
            double ke = (pnDaughter->getEnergy() - pnDaughter->getMass());
            double px = pnDaughter->getMomentum()[0];
            double py = pnDaughter->getMomentum()[1];
            double pz = pnDaughter->getMomentum()[2];
            double p = sqrt(px*px + py*py + pz*pz); 
            double theta = acos(pz/p)*180.0/3.14159;
            long int dauID = TMath::Abs(pnDaughter->getPdgID());

            long int nucPrefix = 1000000000;

            // if daughter is nucleus, extract A from pdgID=10LZZZAAAI
            int nucA = (dauID > nucPrefix) ? 
                       (dauID % 10000) / 10 :
                       -1;
            //std::cout << "Found daughter with nucleus weight: " << dauID << " " << nucA << std::endl;

            // hardest proton or neutron
            if ((dauID == 2212 || dauID == 2112) && (ke > ke_nucleon)) {
                ke_nucleon = ke;
                sim_nucleon = pnDaughter;
                theta_nucleon = theta;
            }
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

        /*
        if(verb) std::cout<<TString::Format(
                         "ke_nucleon %0.3f, theta_nucleon %0.3f, w_nucleon %0.3f, wfit_nucleon %0.3f, weight_nucleon %0.3f, ke_hard %0.3f, p_hard %0.3f, pz_hard %0.3f, w_hard %0.3f, theta_hard %0.3f, A_hard %d, ke_heavy %0.3f, p_heavy %0.3f, pz_heavy %0.3f, w_heavy %0.3f, theta_heavy %0.3f, A_heavy %d, ke_dau %0.3f, p_dau %0.3f, pz_dau %0.3f, w_dau %0.3f, theta_dau %0.3f, pdg_dau %d",
                          ke_nucleon, theta_nucleon, w_nucleon, wfit_nucleon, weight_nucleon,
                          ke_hard, p_hard, pz_hard, w_hard, theta_hard, A_hard,
                          ke_heavy, p_heavy, pz_heavy, w_heavy, theta_heavy, A_heavy,
                          ke_dau, p_dau, pz_dau, w_dau, theta_dau, pdg_dau
                        ) << std::endl;*/

        // Add the result to the collection    
        event.addToCollection("PNweight", result_);
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
