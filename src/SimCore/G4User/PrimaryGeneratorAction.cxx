/**
 * @file PrimaryGeneratorAction.cxx
 * @brief Class implementing the Geant4 primary generator action
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimCore/G4User/PrimaryGeneratorAction.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4Event.hh"
#include "G4RunManager.hh"  // Needed for CLHEP

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/PrimaryGenerator.h"
#include "SimCore/UserEventInformation.h"
#include "SimCore/UserPrimaryParticleInformation.h"

/*~~~~~~~~~~*/
/*   ROOT   */
/*~~~~~~~~~~*/
#include "TRandom3.h"

namespace simcore::g4user {

PrimaryGeneratorAction::PrimaryGeneratorAction(
    const framework::config::Parameters& parameters)
    : G4VUserPrimaryGeneratorAction() {
  // Check whether a beamspot should be used or not.
  auto beamSpot{
      parameters.getParameter<std::vector<double> >("beamSpotSmear", {})};
  if (!beamSpot.empty()) {
    useBeamspot_ = true;
    beamspotXSize_ = beamSpot[0];
    beamspotYSize_ = beamSpot[1];
    beamspotZSize_ = beamSpot[2];
  }

  time_shift_primaries_ = parameters.getParameter<bool>("time_shift_primaries");

  auto generators{
      parameters.getParameter<std::vector<framework::config::Parameters> >(
          "generators", {})};
  if (generators.empty()) {
    EXCEPTION_RAISE("MissingGenerator",
                    "Need to define some generator of primaries.");
  }

  for (auto& generator : generators) {
    PrimaryGenerator::Factory::get().make(
        generator.getParameter<std::string>("class_name"),
        generator.getParameter<std::string>("instance_name"), generator);
  }
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event) {
  /*
   * Create our Event information first so that it
   * can be accessed by everyone from now on.
   */
  // Make sure we aren't overwriting a different information container
  if (event->GetUserInformation()) {
    EXCEPTION_RAISE(
        "Misconfig",
        "There was a UserEventInformation attached before beginning event."
        "\nI don't know how this happend!!");
  }

  // Make our information container and give it to geant4
  //    G4Event owns the event information and will delete it
  auto event_info = new UserEventInformation;
  event->SetUserInformation(event_info);

  PrimaryGenerator::Factory::get().apply([event](const auto& generator) {
        generator->GeneratePrimaryVertex(event);
      });

  // smear all primary vertices (if activated)
  int nPV = event->GetNumberOfPrimaryVertex();
  if (nPV > 0) {
    // loop over all vertices generated
    for (int iPV = 0; iPV < nPV; ++iPV) {
      G4PrimaryVertex* primary_vertex = event->GetPrimaryVertex(iPV);

      // Loop over all particle associated with the primary vertex and
      // set the generator status to 1.
      for (int iparticle = 0; iparticle < primary_vertex->GetNumberOfParticle();
           ++iparticle) {
        G4PrimaryParticle* primary = primary_vertex->GetPrimary(iparticle);

        auto primary_info{dynamic_cast<UserPrimaryParticleInformation*>(
            primary->GetUserInformation())};
        if (not primary_info) {
          // no user info defined
          //  ==> make a new one
          primary_info = new UserPrimaryParticleInformation;
          primary->SetUserInformation(primary_info);
        }  // check if primaryinfo is defined

        int hepStatus = primary_info->getHepEvtStatus();
        if (hepStatus <= 0) {
          // undefined hepStatus ==> set to 1
          primary_info->setHepEvtStatus(1);
        }  // check if hepStatus defined

      }  // iparticle - loop over primary particles from this vertex

      // include the weight of this primary vertex in the event weight
      event_info->incWeight(primary_vertex->GetWeight());

      // smear beamspot if it is turned on
      if (useBeamspot_) {
        double x0_i = primary_vertex->GetX0();
        double y0_i = primary_vertex->GetY0();
        double z0_i = primary_vertex->GetZ0();
        /*
         * G4UniformRand returns a number in [0,1]
         *  - we shift this range so that it is [-0.5,0.5]
         *  - multiply by the width to get [-0.5*size,0.5*size]
         *  - add the initial point (in case its off center) to get
         *    [init-0.5*size, init+0.5*size]
         */
        double x0_f = beamspotXSize_ * (G4UniformRand() - 0.5) + x0_i;
        double y0_f = beamspotYSize_ * (G4UniformRand() - 0.5) + y0_i;
        double z0_f = beamspotZSize_ * (G4UniformRand() - 0.5) + z0_i;
        primary_vertex->SetPosition(x0_f, y0_f, z0_f);
      }

      // shift so that t=0 coincides with primaries arriving at (or coming from)
      // the target
      if (time_shift_primaries_) {
        primary_vertex->SetT0(primary_vertex->GetT0() +
                              primary_vertex->GetZ0() / 299.702547);
      }

    }  // iPV - loop over primary vertices
  } else {
    EXCEPTION_RAISE(
        "NoPrimaries",
        "No primary vertices were produced by any of the generators.");
  }
}
}  // namespace simcore::g4user
