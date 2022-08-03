/**
 * @file RootCompleteReSim.cxx
 * @brief Primary generator used to generate primaries from SimParticles.
 * @author Nhan Tran, Fermilab
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimCore/Generators/RootCompleteReSim.h"

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
#include "Framework/Configure/Parameters.h"
#include "SimCore/Event/SimParticle.h"

namespace simcore {
namespace generators {

RootCompleteReSim::RootCompleteReSim(const std::string& name,
                                     const framework::config::Parameters& parameters)
    : PrimaryGenerator(name, parameters), ievent_("InputReSim") {
  framework::config::Parameters file_params;
  file_params.addParameter<std::string>("tree_name","LDMX_Events");
  std::string filename = parameters_.getParameter<std::string>("filePath");
  ifile_ = std::make_unique<framework::EventFile>(file_params,filename);
  ifile_->setupEvent(&ievent_);

  simParticleCollName_ =
      parameters.getParameter<std::string>("collection_name");
  simParticlePassName_ = parameters.getParameter<std::string>("pass_name");
}

RootCompleteReSim::~RootCompleteReSim() {
  ifile_->close();
  ifile_.reset(nullptr);
}

void RootCompleteReSim::GeneratePrimaryVertex(G4Event* anEvent) {
  // go to next event
  //  if there isn't another event ==> EventFile::nextEvent returns false
  if (not ifile_->nextEvent()) {
    std::cout << "[ RootSimFromEcalSP ]: End of file reached." << std::endl;
    G4RunManager::GetRunManager()->AbortRun(true);
    anEvent->SetEventAborted();
  }

  auto simParticles{ievent_.getMap<int, ldmx::SimParticle>(simParticleCollName_,
                                                     simParticlePassName_)};
  std::vector<G4PrimaryVertex*> vertices;  // vertices already put into Geant4
  for (const auto& [trackID, sp] : simParticles) {
    // check if particle has status 1
    // skip if not primary gen status
    if (sp.getGenStatus() != 1) continue;

    bool vertexExists = false;
    G4PrimaryVertex* curvertex = new G4PrimaryVertex();
    for (unsigned int iV = 0; iV < vertices.size(); ++iV) {
      double cur_vx = sp.getVertex()[0];
      double cur_vy = sp.getVertex()[1];
      double cur_vz = sp.getVertex()[2];
      double i_vx = vertices.at(iV)->GetX0();
      double i_vy = vertices.at(iV)->GetY0();
      double i_vz = vertices.at(iV)->GetZ0();
      if ((cur_vx == i_vx) && (cur_vy == i_vy) && (cur_vz == i_vz)) {
        vertexExists = true;
        curvertex = vertices.at(iV);
      }
    }
    if (vertexExists == false) {
      curvertex->SetPosition(sp.getVertex()[0], sp.getVertex()[1],
                             sp.getVertex()[2]);
      curvertex->SetWeight(1.);
      anEvent->AddPrimaryVertex(curvertex);
    }

    G4PrimaryParticle* primary = new G4PrimaryParticle();
    primary->SetPDGcode(sp.getPdgID());
    primary->SetMomentum(sp.getMomentum()[0] * MeV, sp.getMomentum()[1] * MeV,
                         sp.getMomentum()[2] * MeV);
    primary->SetMass(sp.getMass() * MeV);

    curvertex->SetPrimary(primary);
  }

  // Create an input stream with a copy of the event seed as content.
  // The input stream is then passed to the random engine to restore
  // the state.
  std::istringstream iss(
      ievent_.getEventHeader().getStringParameter("eventSeed"));
  G4Random::restoreFullState(iss);
}

}  // namespace generators
}  // namespace simcore

DECLARE_GENERATOR(simcore::generators, RootCompleteReSim)
