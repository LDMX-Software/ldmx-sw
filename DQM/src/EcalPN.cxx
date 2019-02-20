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
        h["hardest_theta_z"] = Histogram1DBuilder<TH1F>("hardest_theta_z", 360, 0, 180)
            .xLabel("#theta_{z} of Hardest Photo-nuclear Particle (Degrees)").build();
        h["hardest_p_ke"] = Histogram1DBuilder<TH1F>("hardest_p_ke", 400, 0, 4000)
            .xLabel("Kinetic Energy Hardest Photo-nuclear Proton Particle (MeV)").build(); 
        h["hardest_p_theta_z"] = Histogram1DBuilder<TH1F>("hardest_p_theta_z", 360, 0, 180)
            .xLabel("#theta_{z} of Hardest Photo-nuclear Proton Particle (Degrees)").build();
        h["hardest_n_ke"] = Histogram1DBuilder<TH1F>("hardest_n_ke", 400, 0, 4000)
            .xLabel("Kinetic Energy Hardest Photo-nuclear Neutron Particle (MeV)").build(); 
        h["hardest_n_theta_z"] = Histogram1DBuilder<TH1F>("hardest_n_theta_z", 360, 0, 180)
            .xLabel("#theta_{z} of Hardest Photo-nuclear Neutron Particle (Degrees)").build();
        h["hardest_pi_ke"] = Histogram1DBuilder<TH1F>("hardest_pi_ke", 400, 0, 4000)
            .xLabel("Kinetic Energy Hardest Photo-nuclear Pion Particle (MeV)").build(); 
        h["hardest_pi_theta_z"] = Histogram1DBuilder<TH1F>("hardest_pi_theta_z", 360, 0, 180)
            .xLabel("#theta_{z} of Hardest Photo-nuclear Pion Particle (Degrees)").build();
    
        h["h_ke_h_theta_z"] = new TH2F("h_ke_h_theta_z", "", 400, 0, 4000, 360, 0, 180);
        h["h_ke_h_theta_z"]->GetXaxis()->SetTitle("Kinetic Energy Hardest Photo-nuclear Pion Particle (MeV)"); 
        h["h_ke_h_theta_z"]->GetYaxis()->SetTitle("#theta_{z} of Hardest Photo-nuclear Pion Particle (Degrees)"); 
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
   
        double leadKE{-9999},   leadThetaZ{-9999};
        double leadPKE{-9999},  leadPThetaZ{-9999};
        double leadNKE{-9999},  leadNThetaZ{-9999};
        double leadPiKE{-9999}, leadPiThetaZ{-9999};
        for (size_t idaughter = 0; idaughter < pnGamma->getDaughterCount(); ++idaughter) {
            const SimParticle* daughter = pnGamma->getDaughter(idaughter);

            double ke = daughter->getEnergy() - daughter->getMass(); 
            double theta_z = Analysis::thetaZ(daughter); 
            int pdgid = daughter->getPdgID();
            
            if (leadKE < ke) { 
                leadKE = ke;
                leadThetaZ = theta_z; 
            }
            
            if ((pdgid == 2112) && (leadNKE < ke)) {
                leadNKE = ke; 
                leadNThetaZ = theta_z; 
            } 
            
            if ((pdgid == 2212) && (leadPKE < ke)) {
                leadPKE = ke;
                leadPThetaZ = theta_z; 
            } 
            
            if ( ( (abs(pdgid) == 211) || (pdgid == 111) ) 
                    && (leadPiKE < ke) ) { 
                leadPiKE = ke; 
                leadPiThetaZ = theta_z; 
            } 
        }

        h["hardest_ke"]->Fill(leadKE); 
        h["hardest_theta_z"]->Fill(leadThetaZ);
        h["h_ke_h_theta_z"]->Fill(leadKE, leadThetaZ); 
        h["hardest_p_ke"]->Fill(leadPKE); 
        h["hardest_p_theta_z"]->Fill(leadPThetaZ); 
        h["hardest_n_ke"]->Fill(leadNKE); 
        h["hardest_n_theta_z"]->Fill(leadNThetaZ); 
        h["hardest_pi_ke"]->Fill(leadPiKE); 
        h["hardest_pi_theta_z"]->Fill(leadPiThetaZ); 
    }
} // ldmx

DECLARE_ANALYZER_NS(ldmx, EcalPN)
