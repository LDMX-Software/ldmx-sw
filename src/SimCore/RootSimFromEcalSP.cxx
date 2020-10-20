/**
 * @file RootSimFromEcalSP.cxx
 * @brief Primary generator used to generate primaries from SimParticles. 
 * @author Nhan Tran, Fermilab
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimCore/RootSimFromEcalSP.h"

//----------------//
//   C++ StdLib   //
//----------------//
#include <unordered_map>

//------------//
//   Geant4   //
//------------//
#include "G4Event.hh"
#include "G4IonTable.hh"
#include "G4PhysicalConstants.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"

//-------------//
//   ldmx-sw   //
//-------------//
#include "Event/EventConstants.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "Framework/Configure/Parameters.h"

namespace ldmx {

    RootSimFromEcalSP::RootSimFromEcalSP( const std::string& name , Parameters& parameters )
        : PrimaryGenerator( name , parameters ), ievent_( "InputReSim" )
    {
        std::string filename = parameters_.getParameter< std::string >( "filePath" );
        ifile_ = std::make_unique<EventFile>( filename );
        ifile_->setupEvent( &ievent_ );

        timeCutoff_ = parameters_.getParameter< double >( "time_cutoff" );

        ecalSPHitsCollName_ = parameters_.getParameter< std::string >( "collection_name" );
        ecalSPHitsPassName_ = parameters_.getParameter< std::string >( "pass_name" );
    }

    RootSimFromEcalSP::~RootSimFromEcalSP() { 
        ifile_->close();
        ifile_.reset(nullptr);
    }

    void RootSimFromEcalSP::GeneratePrimaryVertex(G4Event* anEvent) {

        //go to next event
        //  if there isn't another event ==> EventFile::nextEvent returns false
        if ( not ifile_->nextEvent() ) {
            std::cout << "[ RootSimFromEcalSP ]: End of file reached." << std::endl;
            G4RunManager::GetRunManager()->AbortRun(true);
            anEvent->SetEventAborted();
        }

        // In this mode, we need to loop through all ECal scoring plane hits
        // and find the subset of unique hits created by particles exiting
        // the ECal.  These particles will be stored in a container and 
        // re-fired into the HCal. 
        std::unordered_map<int, const SimTrackerHit*> spHits; 

        auto ecalSPParticles{ievent_.getCollection<SimTrackerHit>( ecalSPHitsCollName_ , ecalSPHitsPassName_ )};
        // Loop through all of the ECal scoring plane hits. 
        for (const SimTrackerHit& spHit : ecalSPParticles ) {

            // First, start by skipping all hits that were created by 
            // particles entering the ECal volume. 
            if (spHit.getLayerID() == 1 and spHit.getMomentum()[2] > 0) continue;
            if (spHit.getLayerID() == 2 and spHit.getMomentum()[2] < 0) continue; 
            if (spHit.getLayerID() == 3 and spHit.getMomentum()[1] < 0) continue; 
            if (spHit.getLayerID() == 4 and spHit.getMomentum()[1] > 0) continue; 
            if (spHit.getLayerID() == 5 and spHit.getMomentum()[0] > 0) continue; 
            if (spHit.getLayerID() == 6 and spHit.getMomentum()[0] < 0) continue;

            // Don't consider particles created outside of the HCal readout
            // window.  Currently, this is estimated to be 50 ns.  
            if (spHit.getTime() > timeCutoff_) continue; 

            if (spHits.find(spHit.getTrackID()) == spHits.end()) {
                spHits[spHit.getTrackID()] = &spHit; 
            } else {  
                    
               float currentPMag = sqrt(
                                  pow(spHit.getMomentum()[0], 2) +
                                  pow(spHit.getMomentum()[1], 2) +
                                  pow(spHit.getMomentum()[2], 2)); 
               float pMag = sqrt(
                                  pow(spHits[spHit.getTrackID()]->getMomentum()[0], 2) +
                                  pow(spHits[spHit.getTrackID()]->getMomentum()[1], 2) +
                                  pow(spHits[spHit.getTrackID()]->getMomentum()[2], 2)); 

                if (pMag < currentPMag) spHits[spHit.getTrackID()] = &spHit; 
            } 
        } 

        for (auto const& spHit : spHits) { 

            auto cVertex{new G4PrimaryVertex()};
            cVertex->SetPosition(spHit.second->getPosition()[0]*mm, spHit.second->getPosition()[1]*mm, spHit.second->getPosition()[2]*mm);
            cVertex->SetWeight(1.);

            auto primary{new G4PrimaryParticle()};
            primary->SetPDGcode(spHit.second->getPdgID());
            primary->SetMomentum(spHit.second->getMomentum()[0]*MeV, spHit.second->getMomentum()[1]*MeV, spHit.second->getMomentum()[2]*MeV);

            cVertex->SetPrimary(primary);
            anEvent->AddPrimaryVertex(cVertex);

        }   

        // Create an input stream with a copy of the event seed as content.
        // The input stream is then passed to the random engine to restore 
        // the state.
        std::istringstream iss(ievent_.getEventHeader().getStringParameter("eventSeed"));
        G4Random::restoreFullState(iss); 
    }

} //ldmx

DECLARE_GENERATOR( ldmx , RootSimFromEcalSP )
