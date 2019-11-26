/**
 * @file EcalDigiVerifier.cxx
 * @brief Generate histograms to check digi performance
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Ecal/EcalDigiVerifier.h"

namespace ldmx {

    void EcalDigiVerifier::configure(const ldmx::ParameterSet& ps) {



        return;
    }

    void EcalDigiVerifier::analyze(const ldmx::Event& event) {

        return;
    }
    
    void EcalDigiVerifier::onProcessStart() {

        getHistoDirectory();

        h_SimEDep_RecAmplitude_ = new TH2F(
                "h_SimEDep_RecAmplitude_",
                "Total Energy Deposited in ECal Cell;Simulated [MeV];Reconstructed [MeV];Count",
                100,0,25.,
                100,0,25.
                );

        h_TotalRecEnergy_ = new TH1F(
                "h_TotalRecEnergy_",
                ";Total Reconstructed Energy in ECal [MeV];Count",
                800,0,8000.
                );

        h_NumSimHitsPerCell_ = new TH1F(
                "h_NumSimHitsPerCell_",
                ";Number SimHits per ECal Cell (excluding empty cells);Count",
                20,0,20
                );

        return;
    }

    void EcalDigiVerifier::onProcessEnd() {

        return;
    }

}

DECLARE_ANALYZER_NS(ldmx, EcalDigiVerifier);
