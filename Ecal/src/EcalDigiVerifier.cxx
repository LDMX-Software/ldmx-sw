/**
 * @file EcalDigiVerifier.cxx
 * @brief Generate histograms to check digi performance
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Ecal/EcalDigiVerifier.h"

namespace ldmx {

    void EcalDigiVerifier::configure(const ldmx::ParameterSet& ps) {

        ecalSimHitColl_ = ps.getString( "ecalSimHitColl" , "EcalSimHits" );
        ecalSimHitPass_ = ps.getString( "ecalSimHitPass" , "sim" );
        ecalRecHitColl_ = ps.getString( "ecalRecHitColl" , "EcalRecHits" );
        ecalRecHitPass_ = ps.getString( "ecalSimHitPass" , "" );

        return;
    }

    void EcalDigiVerifier::analyze(const ldmx::Event& event) {

        //get truth information sorted into an ID based map
        const TClonesArray *ecalSimHits = event.getCollection( ecalSimHitColl_ , ecalSimHitPass_ );
        int numSimHits = ecalSimHits->GetEntriesFast();
        std::map< int , std::vector< SimCalorimeterHit *> > id_simHit;
        for ( int iHit = 0; iHit < numSimHits; iHit++ ) {
            
            SimCalorimeterHit *simHit = (SimCalorimeterHit *)(ecalSimHits->At( iHit ));

            int  rawID = simHit->getID();
            id_simHit[ rawID ].push_back( simHit );
        }

        const TClonesArray *ecalRecHits = event.getCollection( ecalRecHitColl_ , ecalRecHitPass_ );
        int numRecHits = ecalRecHits->GetEntriesFast();
        double totalRecEnergy = 0.;
        for ( int iRecHit = 0; iRecHit < numRecHits; iRecHit++ ) {

            EcalHit *recHit = (EcalHit *)(ecalRecHits->At( iRecHit ));

            //skip anything that digi flagged as noise
            if ( recHit->isNoise() ) continue;

            int rawID = recHit->getID();

            int numSimHits = id_simHit[rawID].size();
            h_NumSimHitsPerCell_->Fill( numSimHits );

            double totalSimEDep = 0.;
            for ( int iSimHit = 0; iSimHit < numSimHits; iSimHit++ ) {
                totalSimEDep += id_simHit[rawID][iSimHit]->getEdep();
            }

            h_SimEDep_RecAmplitude_->Fill( totalSimEDep , recHit->getAmplitude() );

            totalRecEnergy += recHit->getEnergy();
        }

        h_TotalRecEnergy_->Fill( totalRecEnergy );

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
                ";Number SimHits per ECal Cell (excluding empty rec cells);Count",
                20,0,20
                );

        return;
    }

    void EcalDigiVerifier::onProcessEnd() {

        return;
    }

}

DECLARE_ANALYZER_NS(ldmx, EcalDigiVerifier);
