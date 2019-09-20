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
#include "Event/EcalVetoResult.h"
#include "Event/Event.h"
#include "Event/HcalHit.h"
#include "Event/HcalVetoResult.h"
#include "Event/SimParticle.h"
#include "Event/TrackerVetoResult.h"
#include "Framework/HistogramPool.h"
#include "Tools/AnalysisUtils.h"

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

        std::vector<TH1*> hists = { 
            histograms_->get("event_type"),
            histograms_->get("event_type_track_veto"),
            histograms_->get("event_type_bdt"),
            histograms_->get("event_type_hcal"),
            histograms_->get("event_type_track_bdt"),
            histograms_->get("event_type_vetoes"),
            histograms_->get("event_type_500mev"),
            histograms_->get("event_type_500mev_track_veto"),
            histograms_->get("event_type_500mev_bdt"),
            histograms_->get("event_type_500mev_hcal"),
            histograms_->get("event_type_500mev_track_bdt"),
            histograms_->get("event_type_500mev_vetoes"),
            histograms_->get("event_type_2000mev"),
            histograms_->get("event_type_2000mev_track_veto"),
            histograms_->get("event_type_2000mev_bdt"),
            histograms_->get("event_type_2000mev_hcal"),
            histograms_->get("event_type_2000mev_track_bdt"),
            histograms_->get("event_type_2000mev_vetoes"),

        };


        for (int ilabel{1}; ilabel < labels.size(); ++ilabel) { 
            for (auto& hist : hists) {
                hist->GetXaxis()->SetBinLabel(ilabel, labels[ilabel-1].c_str());
            }
        }

        labels = {"", 
            "Nothing hard", // 0  
            "1 n", // 1
            "#geq 2 n", // 2
            "1 #pi", // 3
            "1 p", // 4
            "1 K^{0}", // 5
            "K X", // 6
            "multi-body", // 7
            ""
        };

        hists = {
            histograms_->get("event_type_compact"),
            histograms_->get("event_type_compact_track_veto"),
            histograms_->get("event_type_compact_bdt"),
            histograms_->get("event_type_compact_hcal"),
            histograms_->get("event_type_compact_track_bdt"),
            histograms_->get("event_type_compact_vetoes"),
            histograms_->get("event_type_compact_500mev"),
            histograms_->get("event_type_compact_500mev_track_veto"),
            histograms_->get("event_type_compact_500mev_bdt"),
            histograms_->get("event_type_compact_500mev_hcal"),
            histograms_->get("event_type_compact_500mev_track_bdt"),
            histograms_->get("event_type_compact_500mev_vetoes"),
            histograms_->get("event_type_compact_2000mev"),
            histograms_->get("event_type_compact_2000mev_track_veto"),
            histograms_->get("event_type_compact_2000mev_bdt"),
            histograms_->get("event_type_compact_2000mev_hcal"),
            histograms_->get("event_type_compact_2000mev_track_bdt"),
            histograms_->get("event_type_compact_2000mev_vetoes"),
        };

        for (int ilabel{1}; ilabel < labels.size(); ++ilabel) { 
            for (auto& hist : hists) {
                hist->GetXaxis()->SetBinLabel(ilabel, labels[ilabel-1].c_str());
            }
        }

        // Move into the ECal PN directory
        getHistoDirectory();

        histograms_->create<TH2F>("h_ke_h_theta", 
                            "Kinetic Energy Hardest Photo-nuclear Particle (MeV)",
                            400, 0, 4000, 
                            "#theta of Hardest Photo-nuclear Particle (Degrees)",
                            360, 0, 180);

        histograms_->create<TH2F>("1n_ke:2nd_h_ke", 
                            "Kinetic Energy of Leading Neutron (MeV)",
                            400, 0, 4000, 
                            "Kinetic Energy of 2nd Hardest Particle",
                            400, 0, 4000);
        histograms_->create<TH2F>("1kp_ke:2nd_h_ke", 
                            "Kinetic Energy of Leading Charged Kaon (MeV)",
                            400, 0, 4000, 
                            "Kinetic Energy of 2nd Hardest Particle",
                            400, 0, 4000);
        histograms_->create<TH2F>("1k0_ke:2nd_h_ke", 
                            "Kinetic Energy of Leading K0 (MeV)",
                            400, 0, 4000, 
                            "Kinetic Energy of 2nd Hardest Particle",
                            400, 0, 4000);

        std::vector<std::string> n_labels = {"", "",
            "nn", // 1
            "pn", // 2
            "#pi^{+}n", // 3 
            "#pi^{0}n", // 4
            ""
        };

        TH1* hist = histograms_->get("1n_event_type"); 
        for (int ilabel{1}; ilabel < n_labels.size(); ++ilabel) { 
            hist->GetXaxis()->SetBinLabel(ilabel, n_labels[ilabel-1].c_str());
        }
       
    }

    void EcalPN::configure(const ParameterSet& ps) {
        ecalVetoCollectionName_ = ps.getString("ecal_veto_collection");
    }

    void EcalPN::analyze(const Event & event) { 
 

        // Get the collection of simulated particles from the event
        const TClonesArray* particles = event.getCollection("SimParticles");
      
        // Search for the recoil electron 
        const SimParticle* recoil = Analysis::searchForRecoil(particles); 

        // Use the recoil electron to retrieve the gamma that underwent a 
        // photo-nuclear reaction.
        const SimParticle* pnGamma = Analysis::searchForPNGamma(recoil);
        if (pnGamma == nullptr) { 
            std::cout << "[ EcalPN ]: PN Daughter is lost, skipping." << std::endl;
            return;
        }

        histograms_->get("pn_particle_mult")->Fill(pnGamma->getDaughterCount());
        histograms_->get("pn_gamma_energy")->Fill(pnGamma->getEnergy()); 
        histograms_->get("pn_gamma_int_z")->Fill(pnGamma->getEndPoint()[2]); 
        histograms_->get("pn_gamma_vertex_z")->Fill(pnGamma->getVertex()[2]);  

        double lke{-1},   lt{-1}; 
        double lpke{-1},  lpt{-1};
        double lnke{-1},  lnt{-1};
        double lpike{-1}, lpit{-1};
        
        std::vector<const SimParticle*> pnDaughters; 

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
            
            pnDaughters.push_back(daughter); 
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
        int eventType500MeV = classifyEvent(pnGamma, 500); 
        int eventType2000MeV = classifyEvent(pnGamma, 2000);

        int eventTypeComp = classifyCompactEvent(eventType);  
        int eventTypeComp500MeV = classifyCompactEvent(eventType500MeV);  
        int eventTypeComp2000MeV = classifyCompactEvent(eventType2000MeV);  

        histograms_->get("event_type")->Fill(eventType);
        histograms_->get("event_type_500mev")->Fill(eventType500MeV);
        histograms_->get("event_type_2000mev")->Fill(eventType2000MeV);

        histograms_->get("event_type_compact")->Fill(eventTypeComp);
        histograms_->get("event_type_compact_500mev")->Fill(eventTypeComp500MeV);
        histograms_->get("event_type_compact_2000mev")->Fill(eventTypeComp2000MeV);

        double slke{-9999};
        double nEnergy{-9999}, energyDiff{-9999}, energyFrac{-9999}; 
        if (eventType == 1 || eventType == 17 || eventType == 16 || eventType == 18 || eventType == 2) {
            //Analysis::printDaughters(pnGamma);
            std::sort (pnDaughters.begin(), pnDaughters.end(), [] (const auto& lhs, const auto& rhs) 
            {
                double lhs_ke = lhs->getEnergy() - lhs->getMass(); 
                double rhs_ke = rhs->getEnergy() - rhs->getMass(); 
                return lhs_ke > rhs_ke; 
            }); 
           

            nEnergy = pnDaughters[0]->getEnergy() - pnDaughters[0]->getMass(); 
            slke = pnDaughters[1]->getEnergy() - pnDaughters[1]->getMass();
            energyDiff = pnGamma->getEnergy() - nEnergy; 
            energyFrac = nEnergy/pnGamma->getEnergy(); 

            if (eventType == 1) { 
                histograms_->get("1n_ke:2nd_h_ke")->Fill(nEnergy, slke);
                histograms_->get("1n_neutron_energy")->Fill(nEnergy);  
                histograms_->get("1n_energy_diff")->Fill(energyDiff);
                histograms_->get("1n_energy_frac")->Fill(energyFrac); 
            } else if (eventType == 2) { 
                histograms_->get("2n_n2_energy")->Fill(slke); 
                auto energyFrac2n = (nEnergy + slke)/pnGamma->getEnergy();
                histograms_->get("2n_energy_frac")->Fill(energyFrac2n);
                  
                  
            } else if (eventType == 17) { 
                histograms_->get("1kp_ke:2nd_h_ke")->Fill(nEnergy, slke);
                histograms_->get("1kp_energy")->Fill(nEnergy);  
                histograms_->get("1kp_energy_diff")->Fill(energyDiff);
                histograms_->get("1kp_energy_frac")->Fill(energyFrac); 
            } else if (eventType == 16 || eventType == 18) { 
                histograms_->get("1k0_ke:2nd_h_ke")->Fill(nEnergy, slke);
                histograms_->get("1k0_energy")->Fill(nEnergy);  
                histograms_->get("1k0_energy_diff")->Fill(energyDiff);
                histograms_->get("1k0_energy_frac")->Fill(energyFrac); 
            }

            int nPdgID = abs(pnDaughters[1]->getPdgID());
            int nEventType = -10; 
            if (nPdgID == 2112) nEventType = 1; 
            else if (nPdgID == 2212) nEventType = 2; 
            else if (nPdgID == 211) nEventType = 3;
            else if (nPdgID == 111) nEventType = 4; 
       
            histograms_->get("1n_event_type")->Fill(nEventType); 

        } 

        // Get the collection of ECal veto results if it exist
        float bdtProb{-1}; 
        bool passesBDT{false};  
        if (event.exists(ecalVetoCollectionName_)) {
            const EcalVetoResult* veto 
                = static_cast<const EcalVetoResult*>(
                        event.getCollection(ecalVetoCollectionName_)->At(0));
       
            // Get the BDT probability  
            bdtProb = veto->getDisc();
            
            // Fill the histograms if the event passes the ECal veto
            if (bdtProb >= .99) {
                histograms_->get("event_type_bdt")->Fill(eventType);
                histograms_->get("event_type_500mev_bdt")->Fill(eventType500MeV);
                histograms_->get("event_type_2000mev_bdt")->Fill(eventType2000MeV); 
                histograms_->get("event_type_compact_bdt")->Fill(eventTypeComp);
                histograms_->get("event_type_compact_500mev_bdt")->Fill(eventTypeComp500MeV);
                histograms_->get("event_type_compact_2000mev_bdt")->Fill(eventTypeComp2000MeV);
                histograms_->get("pn_particle_mult_bdt")->Fill(pnGamma->getDaughterCount());
                histograms_->get("pn_gamma_energy_bdt")->Fill(pnGamma->getEnergy());
                histograms_->get("1n_neutron_energy_bdt")->Fill(nEnergy);  
                histograms_->get("1n_energy_diff_bdt")->Fill(energyDiff);
                histograms_->get("1n_energy_frac_bdt")->Fill(energyFrac); 
                passesBDT = true; 
            }
        }

        bool passesHcalVeto{false}; 
        // Check if the HcalVeto result exists
        if (event.exists("HcalVeto")) {
        
            // Get the collection of HCalDQM digitized hits if the exists 
            const TClonesArray* hcalVeto = event.getCollection("HcalVeto");

            HcalVetoResult* veto = static_cast<HcalVetoResult*>(hcalVeto->At(0));
            //HcalHit* maxPEHit = veto->getMaxPEHit(); 

            //std::cout << "max PE: " << maxPEHit->getPE() << std::endl; 
    
            if (veto->passesVeto()) {
                histograms_->get("event_type_hcal")->Fill(eventType);
                histograms_->get("event_type_500mev_hcal")->Fill(eventType500MeV);
                histograms_->get("event_type_2000mev_hcal")->Fill(eventType2000MeV);
                histograms_->get("event_type_compact_hcal")->Fill(eventTypeComp);
                histograms_->get("event_type_compact_500mev_hcal")->Fill(eventTypeComp500MeV);
                histograms_->get("event_type_compact_2000mev_hcal")->Fill(eventTypeComp2000MeV);
                histograms_->get("pn_particle_mult_hcal")->Fill(pnGamma->getDaughterCount());
                histograms_->get("pn_gamma_energy_hcal")->Fill(pnGamma->getEnergy()); 
                histograms_->get("1n_neutron_energy_hcal")->Fill(nEnergy);  
                histograms_->get("1n_energy_diff_hcal")->Fill(energyDiff);
                histograms_->get("1n_energy_frac_hcal")->Fill(energyFrac); 
                passesHcalVeto = veto->passesVeto();  
            }
        }

        bool passesTrackVeto{false};
        // Check if the TrackerVeto result exists
        if (event.exists("TrackerVeto")) { 

            // Get the collection of trackerVeto results
            const TClonesArray* trackerVeto = event.getCollection("TrackerVeto");

            TrackerVetoResult* veto = static_cast<TrackerVetoResult*>(trackerVeto->At(0)); 
            // Check if the event passes the tracker veto
            if (veto->passesVeto()) { 
                
                passesTrackVeto = true; 
                
                histograms_->get("event_type_track_veto")->Fill(eventType);
                histograms_->get("event_type_500mev_track_veto")->Fill(eventType500MeV);
                histograms_->get("event_type_2000mev_track_veto")->Fill(eventType2000MeV); 
                histograms_->get("event_type_compact_track_veto")->Fill(eventTypeComp);
                histograms_->get("event_type_compact_500mev_track_veto")->Fill(eventTypeComp500MeV);
                histograms_->get("event_type_compact_2000mev_track_veto")->Fill(eventTypeComp2000MeV);
                histograms_->get("pn_particle_mult_track_veto")->Fill(pnGamma->getDaughterCount());    
                histograms_->get("pn_gamma_energy_track_veto")->Fill(pnGamma->getEnergy());
                histograms_->get("1n_neutron_energy_track_veto")->Fill(nEnergy);  
                histograms_->get("1n_energy_diff_track_veto")->Fill(energyDiff);
                histograms_->get("1n_energy_frac_track_veto")->Fill(energyFrac); 

            }
        }
        
        if (passesTrackVeto && passesBDT) { 
            histograms_->get("event_type_track_bdt")->Fill(eventType);
            histograms_->get("event_type_500mev_track_bdt")->Fill(eventType500MeV);
            histograms_->get("event_type_2000mev_track_bdt")->Fill(eventType2000MeV); 
            histograms_->get("event_type_compact_track_bdt")->Fill(eventTypeComp);
            histograms_->get("event_type_compact_500mev_track_bdt")->Fill(eventTypeComp500MeV);
            histograms_->get("event_type_compact_2000mev_track_bdt")->Fill(eventTypeComp2000MeV);
            histograms_->get("pn_particle_mult_track_bdt")->Fill(pnGamma->getDaughterCount());
            histograms_->get("pn_gamma_energy_track_bdt")->Fill(pnGamma->getEnergy());
            histograms_->get("1n_neutron_energy_track_bdt")->Fill(nEnergy);  
            histograms_->get("1n_energy_diff_track_bdt")->Fill(energyDiff);
            histograms_->get("1n_energy_frac_track_bdt")->Fill(energyFrac); 
        }

        if (passesTrackVeto && passesHcalVeto && passesBDT) { 
            histograms_->get("event_type_vetoes")->Fill(eventType);
            histograms_->get("event_type_500mev_vetoes")->Fill(eventType500MeV);
            histograms_->get("event_type_2000mev_vetoes")->Fill(eventType2000MeV);
            histograms_->get("event_type_compact_vetoes")->Fill(eventTypeComp);
            histograms_->get("event_type_compact_500mev_vetoes")->Fill(eventTypeComp500MeV);
            histograms_->get("event_type_compact_2000mev_vetoes")->Fill(eventTypeComp2000MeV);
            histograms_->get("pn_particle_mult_vetoes")->Fill(pnGamma->getDaughterCount());
            histograms_->get("pn_gamma_energy_vetoes")->Fill(pnGamma->getEnergy()); 
            histograms_->get("1n_neutron_energy_vetoes")->Fill(nEnergy);  
            histograms_->get("1n_energy_diff_vetoes")->Fill(energyDiff);
            histograms_->get("1n_energy_frac_vetoes")->Fill(energyFrac); 
                
        
        }
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

    int EcalPN::classifyCompactEvent(int event_type) { 
    
        if (event_type == 0) return 0; 
        if (event_type == 1) return 1; 
        if ( (event_type == 2) || (event_type == 3) ) return 2;
        if ( (event_type == 4) || (event_type == 6) ) return 3;
        if (event_type == 13) return 4; 
        if ( (event_type == 16) || (event_type == 18) ) return 5; 
        if (event_type == 17) return 6; 
        
        return 7; 
    
    }

} // ldmx

DECLARE_ANALYZER_NS(ldmx, EcalPN)
