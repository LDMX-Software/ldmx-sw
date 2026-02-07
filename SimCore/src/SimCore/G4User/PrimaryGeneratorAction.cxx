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
#include "SimCore/G4User/UserEventInformation.h"
#include "SimCore/G4User/UserPrimaryParticleInformation.h"
#include "SimCore/Generators/PrimaryGenerator.h"

/*~~~~~~~~~~*/
/*   ROOT   */
/*~~~~~~~~~~*/
#include "TRandom3.h"

namespace simcore::g4user {

PrimaryGeneratorAction::PrimaryGeneratorAction(
    const framework::config::Parameters& parameters)
    : G4VUserPrimaryGeneratorAction() {
  time_shift_primaries_ = parameters.get<bool>("time_shift_primaries");

  auto generators{parameters.get<std::vector<framework::config::Parameters> >(
      "generators", {})};
  if (generators.empty()) {
    EXCEPTION_RAISE("MissingGenerator",
                    "Need to define some generator of primaries.");
  }

  for (auto& generator : generators) {
    if (not PrimaryGenerator::Factory::get().make(
            generator.get<std::string>("class_name"),
            generator.get<std::string>("instance_name"), generator)) {
      EXCEPTION_RAISE("UnableToCreate",
                      "Unable to create a PrimaryGenerator of type " +
                          generator.get<std::string>("class_name"));
    }
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
  // G4Event owns the event information and will delete it
  auto event_info = new UserEventInformation;
  event->SetUserInformation(event_info);

  PrimaryGenerator::Factory::get().apply([event](const auto& generator) {
    generator->GeneratePrimaryVertex(event);
  });

  // All beam spot smearing is handled by individual generators
  int n_pv = event->GetNumberOfPrimaryVertex();
  if (n_pv > 0) {
    // loop over all vertices generated
    for (int i_pv = 0; i_pv < n_pv; ++i_pv) {
      G4PrimaryVertex* primary_vertex = event->GetPrimaryVertex(i_pv);

      if (not primary_vertex) {
        EXCEPTION_RAISE(
            "BadGen",
            "One of the primary generators created a NULL primary vertex.");
      }

      // Loop over all particle associated with the primary vertex and
      // set the generator status to 1.
      for (int iparticle = 0; iparticle < primary_vertex->GetNumberOfParticle();
           ++iparticle) {
        G4PrimaryParticle* primary = primary_vertex->GetPrimary(iparticle);

        if (not primary) {
          EXCEPTION_RAISE(
              "BadGen",
              "One of the primary generators created a NULL primary particle.");
        }

        auto primary_info{dynamic_cast<UserPrimaryParticleInformation*>(
            primary->GetUserInformation())};
        if (not primary_info) {
          // no user info defined
          //  ==> make a new one
          primary_info = new UserPrimaryParticleInformation;
          primary->SetUserInformation(primary_info);
        }  // check if primaryinfo is defined

        int hep_status = primary_info->getHepEvtStatus();
        if (hep_status <= 0) {
          // undefined hepStatus ==> set to 1
          primary_info->setHepEvtStatus(1);
        }  // check if hepStatus defined

      }  // iparticle - loop over primary particles from this vertex

      // include the weight of this primary vertex in the event weight
      event_info->incWeight(primary_vertex->GetWeight());

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
