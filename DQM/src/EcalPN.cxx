/**
 * @file EcalPN.cxx
 * @brief Analyzer used for ECal PN DQM.
 * @author Omar Moreno, SLAC National Accelerator
 */

#include "DQM/EcalPN.h" 

//----------//
//   ROOT   //
//----------//
#include "TH1F.h"
#include "TH2F.h"
#include "TVector3.h"

//----------//
//   LDMX   //
//----------//
#include "DQM/AnalysisUtils.h"
#include "DQM/Histogram1DBuilder.h"
#include "Event/Event.h"
#include "Event/SimParticle.h"
#include "Event/EcalVetoResult.h"
#include "Event/HcalHit.h"
#include "Framework/HistogramPool.h"

namespace ldmx { 

    EcalPN::EcalPN(const std::string &name, Process &process) : 
        Analyzer(name, process) { }

    EcalPN::~EcalPN() {}

    void EcalPN::onProcessStart() {
       
        // Get an instance of the histogram pool  
        histograms_ = HistogramPool::getInstance();    

        std::vector<std::string> labels = {"", 
            "Nothing hard", // 0  
            "1 n", // 1
            "2 n", // 2
            "#geq 3 n", // 3 
            "1 #pi", // 4
            "2 #pi", // 5 
            "1 #pi_{0}", // 6
            "1 #pi A", // 7
            "1 #pi 2 A", // 8
            "2 #pi A", // 9
            "1 #pi_{0} A", // 10
            "1 #pi_{0} 2 A", // 11
            "#pi_{0} #pi A", // 12
            "1 p", // 13
            "2 p", // 14
            "pn", // 15
            "K^{0}_{L} X", // 16
            "K X", // 17
            "K^{0}_{S} X", // 18
            "exotics", // 19
            "multi-body", // 20
            ""
        };

        TH1* hist = histograms_->get("event_type");
        TH1* hist_bdt = histograms_->get("event_type_bdt");
        TH1* hist_max_pe = histograms_->get("event_type_max_pe");
        for (int ilabel{1}; ilabel < labels.size(); ++ilabel) { 
            hist->GetXaxis()->SetBinLabel(ilabel, labels[ilabel-1].c_str());
            hist_bdt->GetXaxis()->SetBinLabel(ilabel, labels[ilabel-1].c_str());
            hist_max_pe->GetXaxis()->SetBinLabel(ilabel, labels[ilabel-1].c_str());
        }

        // Move into the ECal PN directory
        getHistoDirectory();

        histograms_->create<TH2F>("h_ke_h_theta", 
                            "Kinetic Energy Hardest Photo-nuclear Particle (MeV)",
                            400, 0, 4000, 
                            "#theta of Hardest Photo-nuclear Pion Particle (Degrees)",
                            360, 0, 180);
       
        histograms_->create<TH2F>("bdt_max_pe", 
                            "Max PE", 500, 0, 500, 
                            "BDT Prob", 200, 0.9, 1.0);


    }

    void EcalPN::analyze(const Event & event) { 
   
        // Get the collection of simulated particles from the event
        const TClonesArray* particles = event.getCollection("SimParticles");
      
        // Search for the recoil electron 
        const SimParticle* recoil = Analysis::searchForRecoil(particles); 

        // Fill the recoil vertex position histograms
        std::vector<double> recoilVertex = recoil->getVertex();
        histograms_->get("recoil_vx")->Fill(recoilVertex[0]);  
        histograms_->get("recoil_vy")->Fill(recoilVertex[1]);  
        histograms_->get("recoil_vz")->Fill(recoilVertex[2]);  
   
        // Use the recoil electron to retrieve the gamma that underwent a 
        // photo-nuclear reaction.
        const SimParticle* pnGamma = Analysis::searchForPNGamma(recoil);

        // Get the truth momentum of the gamma that underwent a photo-nuclear 
        // reaction.
        TVector3 pnGammaP(pnGamma->getMomentum().data()); 

        // Get the truth momentum of the recoil
        TVector3 recoilPGen(recoil->getMomentum().data()); 
        
        // Calculate the momentum of the recoil electron
        TVector3 recoilP = recoilPGen - pnGammaP; 

        histograms_->get("recoil_tp")->Fill(recoilP.Mag());
        histograms_->get("recoil_tpt")->Fill(recoilP.Pt()); 
        histograms_->get("recoil_tpx")->Fill(recoilP.Px()); 
        histograms_->get("recoil_tpy")->Fill(recoilP.Py()); 
        histograms_->get("recoil_tpz")->Fill(recoilP.Pz()); 
    
        histograms_->get("pn_particle_mult")->Fill(pnGamma->getDaughterCount());
        histograms_->get("pn_gamma_energy")->Fill(pnGamma->getEnergy()); 
        histograms_->get("pn_gamma_int_z")->Fill(pnGamma->getEndPoint()[2]); 
        histograms_->get("pn_gamma_vertex_z")->Fill(pnGamma->getVertex()[2]);  

        double lke{-1},   lt{-1}; 
        double lpke{-1},  lpt{-1};
        double lnke{-1},  lnt{-1};
        double lpike{-1}, lpit{-1};
        
        // Loop through all of the PN daughters and extract kinematic 
        // information.
        for (size_t idaughter = 0; idaughter < pnGamma->getDaughterCount(); ++idaughter) {

            // Get the ith daughter
            const SimParticle* daughter = pnGamma->getDaughter(idaughter);

            // Get the PDG ID
            int pdgID = daughter->getPdgID();

            // Ignore photons and nuclei
            if (pdgID == 22 || pdgID > 10000) continue;

            // Calculate the kinetic energy
            double ke = daughter->getEnergy() - daughter->getMass();

            std::vector<double> vec = daughter->getMomentum(); 
            TVector3 pvec(vec[0], vec[1], vec[2]); 

            //  Calculate the polar angle
            double theta = pvec.Theta()*(180/3.14159);
 
            if (lke < ke) { lke = ke; lt = theta; }  
            
            if ((pdgID == 2112) && (lnke < ke)) { lnke = ke; lnt = theta; }
           
            if ((pdgID == 2212) && (lpke < ke)) { lpke = ke; lpt = theta; }
            
            if ( ( (abs(pdgID) == 211) || (pdgID == 111) ) && (lpike < ke) ) {
                lpike = ke; 
                lpit = theta; 
            }
        
        }

        histograms_->get("hardest_ke")->Fill(lke); 
        histograms_->get("hardest_theta")->Fill(lt);
        histograms_->get("h_ke_h_theta")->Fill(lke, lt); 
        histograms_->get("hardest_p_ke")->Fill(lpke); 
        histograms_->get("hardest_p_theta")->Fill(lpt); 
        histograms_->get("hardest_n_ke")->Fill(lnke); 
        histograms_->get("hardest_n_theta")->Fill(lnt); 
        histograms_->get("hardest_pi_ke")->Fill(lpike); 
        histograms_->get("hardest_pi_theta")->Fill(lpit); 

        // Classify the event
        int eventType = classifyEvent(pnGamma, 200); 
        histograms_->get("event_type")->Fill(eventType);

        // Get the collection of ECal veto results if it exist
        float bdtProb{-1}; 
        if (event.exists("EcalVeto")) {
            const EcalVetoResult* veto 
                = static_cast<const EcalVetoResult*>(event.getCollection("EcalVeto")->At(0));
       
            // Get the BDT probability  
            bdtProb = veto->getDisc();
            
            // Fill the histograms if the event passes the ECal veto
            if (bdtProb >= .98) {
                histograms_->get("event_type_bdt")->Fill(eventType);
        
                histograms_->get("recoil_tp_bdt")->Fill(recoilP.Mag());
                histograms_->get("recoil_tpt_bdt")->Fill(recoilP.Pt()); 
                histograms_->get("recoil_tpx_bdt")->Fill(recoilP.Px()); 
                histograms_->get("recoil_tpy_bdt")->Fill(recoilP.Py()); 
                histograms_->get("recoil_tpz_bdt")->Fill(recoilP.Pz()); 
        
                histograms_->get("pn_particle_mult_bdt")->Fill(pnGamma->getDaughterCount());
                histograms_->get("pn_gamma_energy_bdt")->Fill(pnGamma->getEnergy()); 
            }
        }

        // Get the collection of HCal digitized hits if the exists 
        float maxPE{-1}; 
        if (event.exists("hcalDigis")) { 
            const TClonesArray* hcalHits = event.getCollection("hcalDigis");
        
            for (size_t ihit{0}; ihit < hcalHits->GetEntriesFast(); ++ihit) {
                HcalHit* hit = static_cast<HcalHit*>(hcalHits->At(ihit)); 
                maxPE = std::max(maxPE, hit->getPE()); 
            }

            if (maxPE < 3) { 
                histograms_->get("event_type_max_pe")->Fill(eventType);
                
                histograms_->get("recoil_tp_max_pe")->Fill(recoilP.Mag());
                histograms_->get("recoil_tpt_max_pe")->Fill(recoilP.Pt()); 
                histograms_->get("recoil_tpx_max_pe")->Fill(recoilP.Px()); 
                histograms_->get("recoil_tpy_max_pe")->Fill(recoilP.Py()); 
                histograms_->get("recoil_tpz_max_pe")->Fill(recoilP.Pz()); 
                
                histograms_->get("pn_particle_mult_max_pe")->Fill(pnGamma->getDaughterCount());
                histograms_->get("pn_gamma_energy_max_pe")->Fill(pnGamma->getEnergy()); 
            }
        }
        
        histograms_->get("bdt_max_pe")->Fill(maxPE, bdtProb); 
    }

    int EcalPN::classifyEvent(const SimParticle* particle, double threshold) {
        short n{0}, p{0}, pi{0}, pi0{0}, exotic{0}, k0l{0}, kp{0}, k0s{0}, 
              lambda{0};

        // Loop through all of the PN daughters and extract kinematic 
        // information.
        for (size_t idaughter = 0; idaughter < particle->getDaughterCount(); 
                ++idaughter) {

            // Get the ith daughter
            const SimParticle* daughter = particle->getDaughter(idaughter);
        
            // Calculate the kinetic energy
            double ke = daughter->getEnergy() - daughter->getMass();
            
            // If the kinetic energy is below threshold, continue
            if (ke <= threshold) continue;

            // Get the PDG ID
            int pdgID = abs(daughter->getPdgID());

            if (pdgID == 2112) n++;
            else if (pdgID == 2212) p++;
            else if (pdgID == 211) pi++;
            else if (pdgID == 111) pi0++;
            else if (pdgID == 130) k0l++; 
            else if (pdgID == 321) kp++; 
            else if (pdgID == 310) k0s++;
            else exotic++;
        }

        int kaons = k0l + kp + k0s; 
        int nucleons = n + p; 
        int pions = pi + pi0; 
        int count = nucleons + pions + exotic + kaons ;
        int count_a = p + pions + exotic + kaons ;
        int count_b = pions + exotic+ kaons;
        int count_c = nucleons + pi0 + exotic+ kaons;
        int count_d = n + pi0 + exotic+ k0l + kp + k0s;
        int count_e = p + pi0 + exotic+ k0l + kp + k0s;
        int count_f = n + p + pi + exotic+ k0l + kp + k0s;
        int count_g = n + pi + pi0 + exotic+ k0l + kp + k0s;
        int count_h = n + p + pi + pi0 + k0l + kp + k0s;
        int count_i = p + pi + exotic+ k0l + kp + k0s; 
        int count_j = n + pi + exotic+ k0l + kp + k0s; 
        int count_k = nucleons + pions + exotic + kp + k0s; 
        int count_l = nucleons + pions + exotic + k0l + k0s; 
        int count_m = nucleons + pions + exotic + kp + k0l; 
        int count_n = pi0 + exotic + kaons; 
        int count_o = pi + exotic + kaons;
        int count_p = exotic + kaons; 

        if (count == 0) return 0; // Nothing hard
        
        if (n == 1) {
            if (count_a == 0) return 1; // 1n
            else if ( (p == 1) && (count_b == 0) ) return 15; // pn
        }
        
        if ( (n == 2) && (count_a == 0) ) return 2; // 2 n

        if ( (n >= 3) && (count_a == 0) ) return 3; // >= 3 n
        
        if (pi == 1) {
                if (count_c == 0) return 4; // 1 pi
                else if ( (p == 1) && (count_d == 0) ) return 7;   // 1 pi 1 p
                else if ( (p == 2) && (count_d == 0) ) return 8;   // 1 pi 1 p
                else if ( (n == 1) && (count_e == 0) ) return 7;  // 1 pi 1 n
                else if ( (n == 2) && (count_e == 0) ) return 8;  // 1 pi 1 n
                else if ( (n == 1) && (p == 1) &&  (count_n == 0) ) return 8; 
        }

        if (pi == 2) {
            if (count_c == 0) return 5; // 2pi
            else if ( (p == 1) && (count_d == 0) ) return 9; // 2pi p 
            else if ( (n == 1) && (count_e == 0) ) return 9; // 2pi n
        }
        
        if ( pi0 == 1 ) {
             if (count_f == 0) return 6; // 1 pi0
             else if ( (n == 1) && (count_i == 0) ) return 10; // 1pi0 1 p
             else if ( (n == 2) && (count_i == 0) ) return 11; // 1pi0 1 p
             else if ( (p == 1) && (count_j == 0) ) return 10; // 1pi0 1 n
             else if ( (p == 2) && (count_j == 0) ) return 11;
             else if ( (n == 1) && (p == 1) && (count_o == 0) ) return 11;  
             else if ( (pi == 1) && ((p == 1) || (n == 1)) && (count_p == 0)) return 12;
        }


        if ( (p == 1) && (count_g == 0) ) return 13; // 1 p 
        if ( (p == 2) && (count_g == 0) ) return 14; // 2 p
        
        if (k0l == 1) return 16;
        if (kp == 1) return 17;
        if (k0s == 1) return 18;

        if ( (exotic > 0) && (count_h == 0) ) return 19;

        return 20;
    }

} // ldmx

DECLARE_ANALYZER_NS(ldmx, EcalPN)
