/**
 * @file RootCompleteReSim.cxx
 * @brief Primary generator used to generate primaries from SimParticles. 
 * @author Nhan Tran, Fermilab
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimCore/RootCompleteReSim.h"

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
#include "Event/SimParticle.h"
#include "Framework/Parameters.h"

namespace ldmx {

    RootCompleteReSim::RootCompleteReSim( const std::string& name , Parameters& parameters )
        : PrimaryGenerator( name , parameters ), ievent_( "InputReSim" )
    {
        std::string filename = parameters_.getParameter< std::string >( "filePath" );
        ifile_ = std::make_unique<EventFile>( filename );
        ifile_->setupEvent( &ievent_ );

        simParticleCollName_ = parameters.getParameter<std::string>( "simParticleCollName" );
        simParticlePassName_ = parameters.getParameter<std::string>( "simParticlePassName" );
    }

    RootCompleteReSim::~RootCompleteReSim() { 
        ifile_->close();
        ifile_.reset(nullptr);
    }

    void RootCompleteReSim::GeneratePrimaryVertex(G4Event* anEvent) {

        //go to next event
        //  if there isn't another event ==> EventFile::nextEvent returns false
        if ( not ifile_->nextEvent() ) {
            std::cout << "[ RootSimFromEcalSP ]: End of file reached." << std::endl;
            G4RunManager::GetRunManager()->AbortRun(true);
            anEvent->SetEventAborted();
        }

        auto simParticles{ievent_.getMap<int,SimParticle>( simParticleCollName_ , simParticlePassName_ )};
        std::vector<G4PrimaryVertex*> vertices; //vertices already put into Geant4
        for (const auto& particle : simParticles ) {

            // check if particle has status 1
            // skip if not primary gen status
            if (particle.second.getGenStatus() != 1) continue;

            bool vertexExists = false;
            G4PrimaryVertex* curvertex = new G4PrimaryVertex();
            for (unsigned int iV = 0; iV < vertices.size(); ++iV) {
                double cur_vx = particle.second.getVertex()[0];
                double cur_vy = particle.second.getVertex()[1];
                double cur_vz = particle.second.getVertex()[2];
                double i_vx = vertices.at(iV)->GetX0();
                double i_vy = vertices.at(iV)->GetY0();
                double i_vz = vertices.at(iV)->GetZ0();
                if ((cur_vx == i_vx) && (cur_vy == i_vy) && (cur_vz == i_vz)) {
                    vertexExists = true;
                    curvertex = vertices.at(iV);
                }
            }
            if (vertexExists == false) {
                curvertex->SetPosition(particle.second.getVertex()[0], 
                                       particle.second.getVertex()[1], 
                                       particle.second.getVertex()[2]);
                curvertex->SetWeight(1.);
                anEvent->AddPrimaryVertex(curvertex);
            }

            G4PrimaryParticle* primary = new G4PrimaryParticle();
            primary->SetPDGcode(particle.second.getPdgID());
            primary->SetMomentum(particle.second.getMomentum()[0] * MeV, 
                                 particle.second.getMomentum()[1] * MeV, 
                                 particle.second.getMomentum()[2] * MeV);
            primary->SetMass(particle.second.getMass() * MeV);

            curvertex->SetPrimary(primary);

        }

        //forces Geant4 to use the evenSeed from the last time by writing it to the file it reads by default
        //TODO remove this I/O bottleneck
        std::ofstream tmpout("tmpEvent.rndm");
        std::string eventSeed = ievent_.getEventHeader().getStringParameter("eventSeed");
        tmpout << eventSeed;
        tmpout.close();

    }

} //ldmx

DECLARE_GENERATOR( ldmx , RootCompleteReSim )
