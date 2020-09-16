
#include "DQM/EcalDigiVerifier.h"

namespace ldmx {

    void EcalDigiVerifier::configure(Parameters& ps) {

        ecalSimHitColl_ = ps.getParameter<std::string>( "ecalSimHitColl" );
        ecalSimHitPass_ = ps.getParameter<std::string>( "ecalSimHitPass" );
        ecalRecHitColl_ = ps.getParameter<std::string>( "ecalRecHitColl" );
        ecalRecHitPass_ = ps.getParameter<std::string>( "ecalRecHitPass" );

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

            histograms_.fill( "num_sim_hits_per_cell"   , numSimHits );
            histograms_.fill( "sim_edep__rec_amplitude" , totalSimEDep , recHit.getAmplitude() );

            totalRecEnergy += recHit.getEnergy();
        }

        histograms_.fill( "total_rec_energy" , totalRecEnergy );

        if ( totalRecEnergy > 6000. ) {
            setStorageHint( hint_shouldKeep );
        } else {
            setStorageHint( hint_shouldDrop );
        }

        return;
    }

}

DECLARE_ANALYZER_NS(ldmx, EcalDigiVerifier);
