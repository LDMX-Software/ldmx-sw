/**
 * @file EcalDigiVerifier.cxx
 * @brief Generate histograms to check digi performance
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Ecal/EcalDigiVerifier.h"

namespace ldmx {

    void EcalDigiVerifier::configure(Parameters& ps) {

        ecalSimHitColl_ = ps.getParameter<std::string>( "ecalSimHitColl" );
        ecalSimHitPass_ = ps.getParameter<std::string>( "ecalSimHitPass" );
        ecalRecHitColl_ = ps.getParameter<std::string>( "ecalRecHitColl" );
        ecalRecHitPass_ = ps.getParameter<std::string>( "ecalSimHitPass" );

        if ( ecalSimHitColl_.empty() ) ecalSimHitColl_ = "EcalSimHits";
        if ( ecalRecHitColl_.empty() ) ecalRecHitColl_ = "EcalRecHits";

        return;
    }

    void EcalDigiVerifier::analyze(const ldmx::Event& event) {

        //get truth information sorted into an ID based map
        std::vector<SimCalorimeterHit> ecalSimHits = event.getCollection<SimCalorimeterHit>( ecalSimHitColl_ , ecalSimHitPass_ );

        //sort sim hits by ID
        std::sort( ecalSimHits.begin() , ecalSimHits.end() , 
                []( const SimCalorimeterHit &lhs , const SimCalorimeterHit &rhs ) {
                    return lhs.getID() < rhs.getID();
                }
                );

        std::vector<EcalHit> ecalRecHits = event.getCollection<EcalHit>( ecalRecHitColl_ , ecalRecHitPass_ );

        //sort rec hits by ID
        std::sort( ecalRecHits.begin() , ecalRecHits.end() , 
                []( const EcalHit &lhs , const EcalHit &rhs ) {
                    return lhs.getID() < rhs.getID();
                }
                );

        double totalRecEnergy = 0.;
        for ( const EcalHit &recHit : ecalRecHits ) {

            //skip anything that digi flagged as noise
            if ( recHit.isNoise() ) continue;

            int rawID = recHit.getID();

            //get information for this hit
            int numSimHits = 0;
            double totalSimEDep = 0.;
            for ( const SimCalorimeterHit &simHit : ecalSimHits ) {
                if ( rawID == simHit.getID() ) {
                    numSimHits += simHit.getNumberOfContribs();
                    totalSimEDep += simHit.getEdep();
                } else if ( rawID < simHit.getID() ) {
                    //later sim hits - all done
                    break;
                }
            }

            h_NumSimHitsPerCell_->Fill( numSimHits );

            h_SimEDep_RecAmplitude_->Fill( totalSimEDep , recHit.getAmplitude() );

            totalRecEnergy += recHit.getEnergy();
        }

        h_TotalRecEnergy_->Fill( totalRecEnergy );

        if ( totalRecEnergy > 6000. ) {
            setStorageHint( hint_shouldKeep );
        } else {
            setStorageHint( hint_shouldDrop );
        }

        return;
    }
    
    void EcalDigiVerifier::onProcessStart() {

        getHistoDirectory();

        h_SimEDep_RecAmplitude_ = new TH2F(
                "h_SimEDep_RecAmplitude_",
                "Total Energy Deposited in ECal Cell;Simulated [MeV];Reconstructed [MeV];Count",
                100,0,50.,
                100,0,50.
                );

        h_TotalRecEnergy_ = new TH1F(
                "h_TotalRecEnergy_",
                ";Total Reconstructed Energy in ECal [MeV];Count",
                800,0,8000.
                );
        h_TotalRecEnergy_->SetCanExtend( TH1::kXaxis );

        h_NumSimHitsPerCell_ = new TH1F(
                "h_NumSimHitsPerCell_",
                ";Number SimHits per ECal Cell (excluding empty rec cells);Count",
                20,0,20
                );
        h_NumSimHitsPerCell_->SetCanExtend( TH1::kXaxis );

        return;
    }

    void EcalDigiVerifier::onProcessEnd() {

        return;
    }

}

DECLARE_ANALYZER_NS(ldmx, EcalDigiVerifier);
