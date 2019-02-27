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
        h["recoil_tpx"]  = Histogram1DBuilder<TH1F>("recoil_tpx", 100, -10, 10)
            .xLabel("Recoil e^{-} Truth p_{x} (MeV)").build();
        h["recoil_tpy"]  = Histogram1DBuilder<TH1F>("recoil_tpy", 100, -10, 10)
            .xLabel("Recoil e^{-} Truth p_{y} (MeV)").build();
        h["recoil_tpz"]  = Histogram1DBuilder<TH1F>("recoil_tpz", 260, -100, 2500)
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
  
        std::vector<std::string> labels = {"", "Nothing hard", "1n", "2n", "#geq 3n", 
            "1 #pi", "2 #pi", "1 #pi_{0}", "1 #pi 1 N", "1p", "2N", "exotics",  "multi-body", ""}; 
        h["event_type"] = Histogram1DBuilder<TH1F>("event_type", 14, -1, 13).build(); 
        for (int i = 1; i < labels.size(); ++i) {
            h["event_type"]->GetXaxis()->SetBinLabel(i, labels[i-1].c_str()); 
        }


        h["h_ke_h_theta"] = new TH2F("h_ke_h_theta", "", 400, 0, 4000, 360, 0, 180);
        h["h_ke_h_theta"]->GetXaxis()->SetTitle("Kinetic Energy Hardest Photo-nuclear Pion Particle (MeV)"); 
        h["h_ke_h_theta"]->GetYaxis()->SetTitle("#theta of Hardest Photo-nuclear Pion Particle (Degrees)"); 
    }

    void EcalPN::analyze(const Event & event) { 
    
        // Get the collection of simulated particles from the event
        const TClonesArray* particles = event.getCollection("SimParticles");
      
        // Search for the recoil electron 
        const SimParticle* recoil = Analysis::searchForRecoil(particles); 

        // Persist recoil vertex position
        std::vector<double> recoilVertex = recoil->getVertex();
        h["recoil_vx"]->Fill(recoilVertex[0]);  
        h["recoil_vy"]->Fill(recoilVertex[1]);  
        h["recoil_vz"]->Fill(recoilVertex[2]);  
    
        // Use the recoil electron to retrieve the gamma that underwent a 
        // photo-nuclear reaction.
        const SimParticle* pnGamma = Analysis::searchForPNGamma(recoil);
        
        // Get the truth momentum of the recoil
        std::vector<double> recoilPGenVec{recoil->getMomentum()};

        // Get the momentum of the gamma that underwent a photonuclear reaction
        // and calculate the recoil momentum.
        std::vector<double> pnGammaP{pnGamma->getMomentum()}; 
        std::vector<double> recoilPVec = {-1*pnGammaP[0], -1*pnGammaP[1],  recoilPGenVec[2] - pnGammaP[2]}; 

        double recoilP = Analysis::vectorMagnitude(recoilPVec); 

        h["recoil_tp"]->Fill(recoilP); 
        h["recoil_tpx"]->Fill(recoilPVec[0]); 
        h["recoil_tpy"]->Fill(recoilPVec[1]); 
        h["recoil_tpz"]->Fill(recoilPVec[2]); 
    
        h["pn_particle_mult"]->Fill(pnGamma->getDaughterCount());
        h["pn_gamma_energy"]->Fill(pnGamma->getEnergy()); 
        h["pn_gamma_int_z"]->Fill(pnGamma->getEndPoint()[2]); 
        h["pn_gamma_vertex_z"]->Fill(pnGamma->getVertex()[2]);  
   
        double leadKE{-9999},   leadTheta{-9999};
        double leadPKE{-9999},  leadPTheta{-9999};
        double leadNKE{-9999},  leadNTheta{-9999};
        double leadPiKE{-9999}, leadPiTheta{-9999};
        int protonCount{0}, neutronCount{0}, pionCount{0}, pi0Count{0};  
        
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
            
            if (ke <= 200 /* MeV */) continue;
            
            if (abs(pdgID) == 2112) ++neutronCount;
            else if (abs(pdgID) == 2212) ++protonCount; 
            else if (abs(pdgID) == 211) ++pionCount;
            else if (pdgID == 111) ++pi0Count; 
        }

        int eventType = classifyEvent(neutronCount, protonCount, pionCount, pi0Count);
        //std::cout << "Event type: " << eventType << std::endl; 
        h["event_type"]->Fill(eventType); 

        h["hardest_ke"]->Fill(leadKE); 
        h["hardest_theta"]->Fill(leadTheta);
        h["h_ke_h_theta"]->Fill(leadKE, leadTheta); 
        h["hardest_p_ke"]->Fill(leadPKE); 
        h["hardest_p_theta"]->Fill(leadPTheta); 
        h["hardest_n_ke"]->Fill(leadNKE); 
        h["hardest_n_theta"]->Fill(leadNTheta); 
        h["hardest_pi_ke"]->Fill(leadPiKE); 
        h["hardest_pi_theta"]->Fill(leadPiTheta); 
    }

    int EcalPN::classifyEvent(const int& neutronCount, 
                    const int& protonCount, const int& pionCount, 
                    const int& pi0Count) {
        
        /*std::cout << " N: " << neutronCount 
                  << " P: " << protonCount 
                  << " pion: " << pionCount
                  << " pi0: " << pi0Count << std::endl; */

        int count = neutronCount + protonCount + pionCount + pi0Count; 
        if (count == 0) return 0; 
        if (protonCount == 0) {
            if ((pionCount + pi0Count) == 0) {
                if (neutronCount >= 3) return 3; 
                else return neutronCount;
            } else if ((neutronCount + pi0Count) == 0) { 
                if (pionCount == 1) return 4; 
                else if (pionCount == 2) return 5; 
            } else if ((neutronCount + pionCount) == 0) {
                if (pi0Count == 1) return 6;
            }
        } 
        
        if (pi0Count == 0) {
            if ((pionCount == 1) && (protonCount == 1 || neutronCount == 1)) return 7;
            if (pionCount == 0) {
                if ((protonCount == 1) && (neutronCount == 0)) return 8; 
                if (((protonCount == 1) && (neutronCount == 1)) || 
                        (protonCount == 2) && (neutronCount == 0)) return 9;
            }
        } 
    
        if ((neutronCount > 0
                && (protonCount > 0 || pionCount > 0 || pi0Count > 0)) ||
            (protonCount > 0
                && (neutronCount > 0 || pionCount > 0 || pi0Count > 0)) ||
            (pionCount > 0
                && (neutronCount > 0 || protonCount > 0 || pi0Count > 0)) ||
            (pi0Count > 0
                && (neutronCount > 0 || protonCount > 0 || pionCount > 0))) {
             return 11;
        }


        return -9999;    
    }
} // ldmx

DECLARE_ANALYZER_NS(ldmx, EcalPN)
