#include "Biasing/NonFiducialFilter.h"
#include <fstream>
#include <stdio.h> 
#include <math.h>
#include <cmath>


/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4EventManager.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4String.hh"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserEventInformation.h"
#include "SimCore/UserTrackInformation.h"

namespace biasing {

NonFiducialFilter::NonFiducialFilter(const std::string& name,framework::config::Parameters& parameters)
  : simcore::UserAction(name, parameters) {
  recoilMaxPThreshold_ =
      parameters.getParameter<double>("recoil_max_p_threshold");
  abortFiducialEvents_ =
      parameters.getParameter<bool>("abort_fiducial_events");
      }
  

NonFiducialFilter::~NonFiducialFilter() {}

void NonFiducialFilter::stepping(const G4Step* step) {
  // Get the track associated with this step.
  auto track{step->GetTrack()};

  // Only process the primary electron track
  if (int parentID{step->GetTrack()->GetParentID()}; parentID != 0) return;

  // Get the PDG ID of the track and make sure it's an electron.
  if (auto pdgID{track->GetParticleDefinition()->GetPDGEncoding()}; pdgID != 11) {
    return;
  }   

  double Xpos{track->GetPosition().getX()}; // x coordinate of the recoil electron
  double Ypos{track->GetPosition().getY()}; // y coordinate of the recoil electron
  double Zpos{track->GetPosition().getZ()}; // z coordinate of the recoil electron

  // Make sure the recoil electron is at the ECal Face when checking 
  if (Zpos > 248.34 && Zpos < 248.36){
    // Record the ECal face cell-module center (x,y) positions from the .txt file
    ifstream inFile;
    inFile.open("/home/dgj1118/LDMX_BASE/production/cellmodule_v13.txt");
    if (!inFile) {
      cerr << "Unable to open txt file!";
      exit(1);   // call system to stop
    }

    std::string row;
    while (inFile.good()) {
      getline(inFile, row, '\n');
      //std::cout << row << endl;

      std::stringstream rowStream(row);
      std::string item;
      std::string x = "";  // x coordinate of a ECal face cell-module center
      std::string y = "";  // y coordinate of an ECal face cell-module center

      getline(rowStream, item, ' ');
      getline(rowStream, x, ' ');
      getline(rowStream, y, ' ');
      //std::cout << x << " " << y << endl;

      std::stringstream ssX;
      ssX << x;
      std::stringstream ssY;
      ssY << y;

      double X = 0.0;
      double Y = 0.0;

      ssX >> X;
      ssY >> Y;

      double result = sqrt(pow(Xpos - X, 2)  + pow(Ypos - Y, 2)); // distance between (x,y) of recoil electron and (x,y) of cell-module center
      // ABORT events that are within 5 mm of any cell-module center at the ECal Face
      if (result <= 5){
        track->SetTrackStatus(fKillTrackAndSecondaries);
        G4RunManager::GetRunManager()->AbortEvent();
      }
    }
  }

}

void NonFiducialFilter::EndOfEventAction(const G4Event*) {}
}  // namespace biasing

DECLARE_ACTION(biasing, NonFiducialFilter)