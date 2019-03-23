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

namespace ldmx { 

    EcalPN::EcalPN(const std::string &name, Process &process) : 
        Analyzer(name, process) { }

    EcalPN::~EcalPN() {}

    void EcalPN::onProcessStart() {
        

        // Open the file and move into the histogram directory
        getHistoDirectory();

        h["recoil_vx"]  = Histogram1DBuilder<TH1F>("recoil_vx", 120, -30, 30)
            .xLabel("Recoil e^{-} Vertex x (mm)").build(); 
        h["recoil_vy"]  = Histogram1DBuilder<TH1F>("recoil_vy", 200, -100, 100)
            .xLabel("Recoil e^{-} Vertex y (mm)").build();
        h["recoil_vz"]  = Histogram1DBuilder<TH1F>("recoil_vz", 40, -2, 0)
            .xLabel("Recoil e^{-} Vertex z (mm)").build();

        h["recoil_tp"]  = Histogram1DBuilder<TH1F>("recoil_tp", 250, 0, 2500)
            .xLabel("Recoil e^{-} Truth p (MeV)").build();
        h["recoil_tpt"]  = Histogram1DBuilder<TH1F>("recoil_tpt", 50, 0, 100)
            .xLabel("Recoil e^{-} Truth p_{t} (MeV)").build();
        h["recoil_tpx"]  = Histogram1DBuilder<TH1F>("recoil_tpx", 100, -10, 10)
            .xLabel("Recoil e^{-} Truth p_{x} (MeV)").build();
        h["recoil_tpy"]  = Histogram1DBuilder<TH1F>("recoil_tpy", 100, -10, 10)
            .xLabel("Recoil e^{-} Truth p_{y} (MeV)").build();
        h["recoil_tpz"]  = Histogram1DBuilder<TH1F>("recoil_tpz", 260, -100, 2500)
            .xLabel("Recoil e^{-} Truth p_{z} (MeV)").build();

        h["recoil_tp - Max PE"]  = Histogram1DBuilder<TH1F>("recoil_tp_max_pe", 250, 0, 2500)
            .xLabel("Recoil e^{-} Truth p (MeV)").build();
        h["recoil_tpt - Max PE"]  = Histogram1DBuilder<TH1F>("recoil_tpt", 50, 0, 100)
            .xLabel("Recoil e^{-} Truth p_{t} (MeV)").build();
        h["recoil_tpx - Max PE"]  = Histogram1DBuilder<TH1F>("recoil_tpx_max_pe", 100, -10, 10)
            .xLabel("Recoil e^{-} Truth p_{x} (MeV)").build();
        h["recoil_tpy - Max PE"]  = Histogram1DBuilder<TH1F>("recoil_tpy_max_pe", 100, -10, 10)
            .xLabel("Recoil e^{-} Truth p_{y} (MeV)").build();
        h["recoil_tpz - Max PE"]  = Histogram1DBuilder<TH1F>("recoil_tpz_max_pe", 260, -100, 2500)
            .xLabel("Recoil e^{-} Truth p_{z} (MeV)").build();

        h["pn_particle_mult"] = Histogram1DBuilder<TH1F>("pn_particle_mult", 100, 0, 200)
            .xLabel("Photo-nuclear Multiplicity").build(); 
        h["pn_gamma_energy"] = Histogram1DBuilder<TH1F>("pn_gamma_energy", 500, 0, 5000)
            .xLabel("#gamma Energy (MeV)").build(); 
        h["pn_gamma_int_z"]  = Histogram1DBuilder<TH1F>("pn_gamma_int_z", 50, 200, 400)
            .xLabel("#gamma Interaction Vertex (mm)").build(); 
        h["pn_gamma_vertex_z"] = Histogram1DBuilder<TH1F>("pn_gamma_vertex_z", 100, -1, 1)
            .xLabel("#gamma Vertex (mm)").build(); 
    
        h["hardest_ke"] = Histogram1DBuilder<TH1F>("hardest_ke", 400, 0, 4000)
            .xLabel("Kinetic Energy Hardest Photo-nuclear Particle (MeV)").build(); 
        h["hardest_theta"] = Histogram1DBuilder<TH1F>("hardest_theta", 360, 0, 180)
            .xLabel("#theta of Hardest Photo-nuclear Particle (Degrees)").build();
        h["hardest_p_ke"] = Histogram1DBuilder<TH1F>("hardest_p_ke", 400, 0, 4000)
            .xLabel("Kinetic Energy Hardest Photo-nuclear Proton Particle (MeV)").build(); 
        h["hardest_p_theta"] = Histogram1DBuilder<TH1F>("hardest_p_theta", 360, 0, 180)
            .xLabel("#theta of Hardest Photo-nuclear Proton Particle (Degrees)").build();
        h["hardest_n_ke"] = Histogram1DBuilder<TH1F>("hardest_n_ke", 400, 0, 4000)
            .xLabel("Kinetic Energy Hardest Photo-nuclear Neutron Particle (MeV)").build(); 
        h["hardest_n_theta"] = Histogram1DBuilder<TH1F>("hardest_n_theta", 360, 0, 180)
            .xLabel("#theta of Hardest Photo-nuclear Neutron Particle (Degrees)").build();
        h["hardest_pi_ke"] = Histogram1DBuilder<TH1F>("hardest_pi_ke", 400, 0, 4000)
            .xLabel("Kinetic Energy Hardest Photo-nuclear Pion Particle (MeV)").build(); 
        h["hardest_pi_theta"] = Histogram1DBuilder<TH1F>("hardest_pi_theta", 360, 0, 180)
            .xLabel("#theta of Hardest Photo-nuclear Pion Particle (Degrees)").build();
  
        std::vector<std::string> labels = {"", 
            "Nothing hard", // 0  
            "1n", // 1
            "2n", // 2
            "#geq 3n", // 3 
            "1 #pi", // 4
            "2 #pi", // 5 
            "1 #pi_{0}", // 6
            "1 #pi N", // 7
            "1 #pi 2N", // 8
            "2 #pi N", // 9
            "1 #pi_{0} N", // 10
            "1 #pi_{0} 2N", // 11
            "#pi_{0} #pi N", // 12
            "1p", // 13
            "2N", // 14
            "K^{0}_{L} X", // 15
            "K^{+} X", // 16
            "K^{0}_{S} X", // 17
            "exotics", // 18
            "multi-body", // 19
            ""};

        h["event_type"] = Histogram1DBuilder<TH1F>("event_type", 23, -1, 22).build(); 
        h["event_type - BDT"] = Histogram1DBuilder<TH1F>("event_type - BDT", 23, -1, 22).build(); 
        h["event_type - max PE"] = Histogram1DBuilder<TH1F>("event_type - max PE", 23, -1, 22).build(); 
        for (int i = 1; i < labels.size(); ++i) {
            h["event_type"]->GetXaxis()->SetBinLabel(i, labels[i-1].c_str()); 
            h["event_type - BDT"]->GetXaxis()->SetBinLabel(i, labels[i-1].c_str()); 
            h["event_type - max PE"]->GetXaxis()->SetBinLabel(i, labels[i-1].c_str()); 
        }

        h["h_ke_h_theta"] = new TH2F("h_ke_h_theta", "", 400, 0, 4000, 360, 0, 180);
        h["h_ke_h_theta"]->GetXaxis()->SetTitle("Kinetic Energy Hardest Photo-nuclear Pion Particle (MeV)"); 
        h["h_ke_h_theta"]->GetYaxis()->SetTitle("#theta of Hardest Photo-nuclear Pion Particle (Degrees)"); 

        h["bdt_max_pe"] = new TH2F("bdt_max_pe", "", 250, 0, 500, 100, 0.9, 1.0);
        h["bdt_max_pe"]->GetXaxis()->SetTitle("Max PE"); 
        h["bdt_max_pe"]->GetYaxis()->SetTitle("BDT Prob"); 

    }

    void EcalPN::analyze(const Event & event) { 
    
        // Get the collection of simulated particles from the event
        const TClonesArray* particles = event.getCollection("SimParticles");
      
        // Search for the recoil electron 
        const SimParticle* recoil = Analysis::searchForRecoil(particles); 

        // Fill the recoil vertex position histograms
        std::vector<double> recoilVertex = recoil->getVertex();
        h["recoil_vx"]->Fill(recoilVertex[0]);  
        h["recoil_vy"]->Fill(recoilVertex[1]);  
        h["recoil_vz"]->Fill(recoilVertex[2]);  
    
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

        h["recoil_tp"]->Fill(recoilP.Mag()); 
        h["recoil_tpx"]->Fill(recoilP.Px()); 
        h["recoil_tpy"]->Fill(recoilP.Py()); 
        h["recoil_tpz"]->Fill(recoilP.Pz()); 
    
        h["pn_particle_mult"]->Fill(pnGamma->getDaughterCount());
        h["pn_gamma_energy"]->Fill(pnGamma->getEnergy()); 
        h["pn_gamma_int_z"]->Fill(pnGamma->getEndPoint()[2]); 
        h["pn_gamma_vertex_z"]->Fill(pnGamma->getVertex()[2]);  
   
        double leadKE{-9999},   leadTheta{-9999};
        double leadPKE{-9999},  leadPTheta{-9999};
        double leadNKE{-9999},  leadNTheta{-9999};
        double leadPiKE{-9999}, leadPiTheta{-9999};
        
        // Loop through all of the PN daughters and extract kinematic 
        // information.
        for (size_t idaughter = 0; idaughter < pnGamma->getDaughterCount(); ++idaughter) {

            // Get the ith daughter
            const SimParticle* daughter = pnGamma->getDaughter(idaughter);

            // Get the PDG ID
            int pdgID = daughter->getPdgID();
            if (pdgID == 22 || pdgID > 10000) continue;

            // Calculate the kinetic energy
            double ke = daughter->getEnergy() - daughter->getMass();

            std::vector<double> vec = daughter->getMomentum(); 
            TVector3 pvec(vec[0], vec[1], vec[2]); 

            //  Calculate the polar angle
            double theta = pvec.Theta()*(180/3.14159);
 
            if (leadKE < ke) { 
                leadKE = ke;
                leadTheta = theta; 
            }
            
            if ((pdgID == 2112) && (leadNKE < ke)) {
                leadNKE = ke; 
                leadNTheta = theta; 
            } 
            
            if ((pdgID == 2212) && (leadPKE < ke)) {
                leadPKE = ke;
                leadPTheta = theta; 
            } 
            
            if ( ( (abs(pdgID) == 211) || (pdgID == 111) ) 
                    && (leadPiKE < ke) ) { 
                leadPiKE = ke; 
                leadPiTheta = theta; 
            }
        }

        int eventType = classifyEvent(pnGamma, 200); 
        //std::cout << "Event type: " << eventType << std::endl; 
        h["event_type"]->Fill(eventType);

        /*
        if (eventType == 19) {
            std::cout << "New event: " << std::endl;
            for (size_t idaughter = 0; idaughter < pnGamma->getDaughterCount(); ++idaughter) {
                const SimParticle* daughter = pnGamma->getDaughter(idaughter);
                // Calculate the kinetic energy
                double ke = daughter->getEnergy() - daughter->getMass();
                std::vector<double> vec = daughter->getMomentum(); 
                TVector3 pvec(vec[0], vec[1], vec[2]); 

                //  Calculate the polar angle
                double theta = pvec.Theta()*(180/3.14159);
                
                std::cout << "PDG ID: " << daughter->getPdgID() 
                          << " Kinetic Energy: " <<  ke 
                          << " p: " << pvec.Mag()
                          << " theta: " << theta 
                          << " vertex: (" << daughter->getVertex()[0] 
                          << ", " << daughter->getVertex()[1] << ", "
                          << daughter->getVertex()[2] << " )"
                          << " end point: (" << daughter->getEndPoint()[0] 
                          << ", " << daughter->getEndPoint()[1] << ", "
                          << daughter->getEndPoint()[2] << " )" << std::endl;
            }
        }*/

        h["hardest_ke"]->Fill(leadKE); 
        h["hardest_theta"]->Fill(leadTheta);
        h["h_ke_h_theta"]->Fill(leadPKE, leadPTheta); 
        h["hardest_p_ke"]->Fill(leadPKE); 
        h["hardest_p_theta"]->Fill(leadPTheta); 
        h["hardest_n_ke"]->Fill(leadNKE); 
        h["hardest_n_theta"]->Fill(leadNTheta); 
        h["hardest_pi_ke"]->Fill(leadPiKE); 
        h["hardest_pi_theta"]->Fill(leadPiTheta); 

        // Get the collection of simulated particles from the event
        const EcalVetoResult* veto = static_cast<const EcalVetoResult*>(
                event.getCollection("EcalVeto", "recon")->At(0));
       
        float bdtProb = veto->getDisc();
        if (bdtProb > .94) {
            h["event_type - BDT"]->Fill(eventType); 
        }

        const TClonesArray* hcalHits = event.getCollection("hcalDigis", "recon");

        float maxPE{0}; 
        for (size_t ihit{0}; ihit < hcalHits->GetEntriesFast(); ++ihit) {
            HcalHit* hit = static_cast<HcalHit*>(hcalHits->At(ihit)); 
            maxPE = std::max(maxPE, hit->getPE()); 
        }

        h["bdt_max_pe"]->Fill(maxPE, bdtProb);
        
        if (maxPE < 50) {
            h["event_type - max PE"]->Fill(eventType); 
            h["recoil_tp - Max PE"]->Fill(recoilP.Mag()); 
            h["recoil_tpx - Max PE"]->Fill(recoilP.Px()); 
            h["recoil_tpy - Max PE"]->Fill(recoilP.Py()); 
            h["recoil_tpz - Max PE"]->Fill(recoilP.Pz()); 

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
            else if ( (p == 1) && (count_b == 0) ) return 14; // 2N
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
        
        if (k0l == 1) return 15;
        if (kp == 1) return 16;
        if (k0s == 1) return 17;

        if ( (exotic > 0) && (count_h == 0) ) return 18;

        return 19;
    }

} // ldmx

DECLARE_ANALYZER_NS(ldmx, EcalPN)
