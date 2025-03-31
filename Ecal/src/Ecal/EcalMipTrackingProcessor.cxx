#include "Ecal/EcalMipTrackingProcessor.h"
#include "Ecal/EcalVetoProcessor.h"

// LDMX 

#include "DetDescr/EcalGeometry.h"
#include "DetDescr/SimSpecialID.h"
#include "Ecal/Event/EcalHit.h"
#include "Recon/Event/EventConstants.h"
#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"

/*~~~~~~~~~~~*/
/*   Tools   */
/*~~~~~~~~~~~*/
#include "Tools/AnalysisUtils.h"

// C++
#include <stdlib.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>

// ROOT (MIP tracking)
#include "TDecompSVD.h"
#include "TMatrixD.h"
#include "TVector3.h"

namespace ecal {


void EcalMipTrackingProcessor::onNewRun(const ldmx::RunHeader &rh) {
  profiling_map_["straight_tracks"] = 0.;
  profiling_map_["linreg_tracks"] = 0.;
  profiling_map_["processing_time_"] = 0;
}
void EcalMipTrackingProcessor::configure(framework::config::Parameters &parameters) {
  verbose_ = parameters.getParameter<bool>("verbose");
  nEcalLayers_ = parameters.getParameter<int>("num_ecal_layers");
  linreg_radius_ = parameters.getParameter<double>("linreg_radius");
  mip_collection_name_ = parameters.getParameter<std::string>("mip_collection_name");
}

void EcalMipTrackingProcessor::clearProcessor() {
   // MIP tracking
  nStraightTracks_ = 0;
  nLinregTracks_ = 0;
  firstNearPhLayer_ = 0;
  nNearPhHits_ = 0;
  epAng_ = 0;
  epSep_ = 0;
  epDot_ = 0;
  photonTerritoryHits_ = 0;
}

void EcalMipTrackingProcessor::produce(framework::Event &event) {

auto start = std::chrono::high_resolution_clock::now();

clearProcessor();


// Read in hits near photon from EcalVetoProcessor

auto ecal_mip_collection = event.getCollection<ldmx::EcalMipCollection>(mip_collection_name_);
std::vector<XYCoords> ele_trajectory;
std::vector<XYCoords> photon_trajectory;
std::vector<ldmx::HitData> trackingHitList;
if (nevents_ <= ecal_mip_collection.size()) {
  ele_trajectory = ecal_mip_collection[nevents_].getEleTrajectory();
  photon_trajectory = ecal_mip_collection[nevents_].getPhotonTrajectory();
  trackingHitList = ecal_mip_collection[nevents_].getTrackingHitList();
};
nevents_++;
// Now inputting Lines 753-1178 of the original EcalVetoProcessor
// ------------------------------------------------------
  // MIP tracking starts here

  /* Goal:  Calculate
   *  nStraightTracks (self-explanatory),
   *  nLinregTracks (tracks found by linreg algorithm),
   */

  // Find epAng and epSep, and prepare EP trajectory vectors:
  TVector3 e_traj_start;
  TVector3 e_traj_end;
  TVector3 p_traj_start;
  TVector3 p_traj_end;
  if (!ele_trajectory.empty() && !photon_trajectory.empty()) {
    // Create TVector3s marking the start and endpoints of each projected
    // trajectory
    e_traj_start.SetXYZ(ele_trajectory[0].first, ele_trajectory[0].second,
                        geometry_->getZPosition(0));
    e_traj_end.SetXYZ(ele_trajectory[(nEcalLayers_ - 1)].first,
                      ele_trajectory[(nEcalLayers_ - 1)].second,
                      geometry_->getZPosition((nEcalLayers_ - 1)));
    p_traj_start.SetXYZ(photon_trajectory[0].first, photon_trajectory[0].second,
                        geometry_->getZPosition(0));
    p_traj_end.SetXYZ(photon_trajectory[(nEcalLayers_ - 1)].first,
                      photon_trajectory[(nEcalLayers_ - 1)].second,
                      geometry_->getZPosition((nEcalLayers_ - 1)));

    TVector3 evec = e_traj_end - e_traj_start;
    TVector3 e_norm = evec.Unit();
    TVector3 pvec = p_traj_end - p_traj_start;
    TVector3 p_norm = pvec.Unit();
    epDot_ = e_norm.Dot(p_norm);
    epAng_ = acos(epDot_) * 180.0 / M_PI;
    epSep_ = sqrt(pow(e_traj_start.X() - p_traj_start.X(), 2) +
                  pow(e_traj_start.Y() - p_traj_start.Y(), 2));
  } else {
    // Electron trajectory is missing, so all hits in the Ecal are fair game.
    // Pick e/ptraj so that they won't restrict the tracking algorithm (place
    // them far outside the ECal).
    e_traj_start = TVector3(999, 999, geometry_->getZPosition(0));  // 0);
    e_traj_end = TVector3(
        999, 999, geometry_->getZPosition((nEcalLayers_ - 1)));       // 999);
    p_traj_start = TVector3(1000, 1000, geometry_->getZPosition(0));  // 0);
    p_traj_end = TVector3(
        1000, 1000, geometry_->getZPosition((nEcalLayers_ - 1)));  // 1000);
    /*ensures event will not be vetoed by angle/separation cut */
    epAng_ = 999.;
    epSep_ = 999.;
    epDot_ = 999.;
  }

  // Near photon step:  Find the first layer of the ECal where a hit near the
  // projected photon trajectory is found Currently unusued pending further
  // study; performance has dropped between v9 and v12. Currently used in
  // segmipBDT
  firstNearPhLayer_ = nEcalLayers_ - 1;

  // If no photon trajectory, leave this at the default (ECal back)
  if (!photon_trajectory.empty()) {
    for (std::vector<ldmx::HitData>::iterator it = trackingHitList.begin();
         it != trackingHitList.end(); ++it) {
      float ehDist =
          sqrt(pow((*it).pos.X() - photon_trajectory[(*it).layer].first, 2) +
               pow((*it).pos.Y() - photon_trajectory[(*it).layer].second, 2));
      if (ehDist < 8.7) {
        nNearPhHits_++;
        if ((*it).layer < firstNearPhLayer_) {
          firstNearPhLayer_ = (*it).layer;
        }
      }
    }
  }

  // Territories limited to trackingHitList
  TVector3 gToe = (e_traj_start - p_traj_start).Unit();
  TVector3 origin = p_traj_start + 0.5 * 8.7 * gToe;
  if (!ele_trajectory.empty()) {
    for (auto &hitData : trackingHitList) {
      TVector3 hitPos = hitData.pos;
      TVector3 hitPrime = hitPos - origin;
      if (hitPrime.Dot(gToe) <= 0) {
        photonTerritoryHits_++;
      }
    }
  } else {
    photonTerritoryHits_ = nReadoutHits_;
  }



  // ------------------------------------------------------
  // Find straight MIP tracks:

  std::sort(trackingHitList.begin(), trackingHitList.end(),
            [](ldmx::HitData ha, ldmx::HitData hb) { return ha.layer > hb.layer; });
  // For merging tracks:  Need to keep track of existing tracks
  // Candidate tracks to merge in will always be in front of the current track
  // (lower z), so only store the last hit 3-layer vector:  each track = vector
  // of 3-tuples (xy+layer).
  std::vector<std::vector<ldmx::HitData>> track_list;

  // print trackingHitList
  if (verbose_) {
    ldmx_log(debug) << "====== Tracking hit list (original) length "
                    << trackingHitList.size() << " ======";
    for (int i = 0; i < trackingHitList.size(); i++) {
      std::cout << "[" << trackingHitList[i].pos.X() << ", "
                << trackingHitList[i].pos.Y() << ", "
                << trackingHitList[i].layer << "], ";
    }
    std::cout << std::endl;
    ldmx_log(debug) << "====== END OF Tracking hit list ======";
  }

  // in v14 minR is 4.17 mm
  // while maxR is 4.81 mm
  float cellWidth = 2 * geometry_->getCellMaxR();
  for (int iHit = 0; iHit < trackingHitList.size(); iHit++) {
    // list of hit numbers in track (34 = maximum theoretical length)
    int track[34];
    int currenthit{iHit};
    int trackLen{1};

    track[0] = iHit;

    // Search for hits to add to the track:
    // repeatedly find hits in the front two layers with same x & y positions
    // but since v14 the odd layers are offset, so we allow half a cellWidth
    // deviation and then add to track until no more hits are found
    int jHit = iHit;
    while (jHit < trackingHitList.size()) {
      if ((trackingHitList[jHit].layer ==
               trackingHitList[currenthit].layer - 1 ||
           trackingHitList[jHit].layer ==
               trackingHitList[currenthit].layer - 2) &&
          abs(trackingHitList[jHit].pos.X() -
              trackingHitList[currenthit].pos.X()) <= 0.5 * cellWidth &&
          abs(trackingHitList[jHit].pos.Y() -
              trackingHitList[currenthit].pos.Y()) <= 0.5 * cellWidth) {
        track[trackLen] = jHit;
        trackLen++;
        currenthit = jHit;
      }
      jHit++;
    }

    // Confirm that the track is valid:
    if (trackLen < 2) continue;  // Track must contain at least 2 hits
    float closest_e = distTwoLines(trackingHitList[track[0]].pos,
                                   trackingHitList[track[trackLen - 1]].pos,
                                   e_traj_start, e_traj_end);
    float closest_p = distTwoLines(trackingHitList[track[0]].pos,
                                   trackingHitList[track[trackLen - 1]].pos,
                                   p_traj_start, p_traj_end);
    // Make sure that the track is near the photon trajectory and away from the
    // electron trajectory Details of these constraints may be revised
    if (closest_p > cellWidth and closest_e < 2 * cellWidth) continue;
    if (trackLen < 4 and closest_e > closest_p) continue;
    if (verbose_) {
      ldmx_log(debug) << "====== After rejection for MIP tracking ======";
      ldmx_log(debug) << "current hit: [" << trackingHitList[iHit].pos.X()
                      << ", " << trackingHitList[iHit].pos.Y() << ", "
                      << trackingHitList[iHit].layer << "]";

      for (int k = 0; k < trackLen; k++) {
        ldmx_log(debug) << "track[" << k << "] position = ["
                        << trackingHitList[track[k]].pos.X() << ", "
                        << trackingHitList[track[k]].pos.Y() << ", "
                        << trackingHitList[track[k]].layer << "]";
      }
    }

    // if track found, increment nStraightTracks and remove all hits in track
    // from future consideration
    if (trackLen >= 2) {
      std::vector<ldmx::HitData> temp_track_list;
      int n_remove = 0;
      for (int kHit = 0; kHit < trackLen; kHit++) {
        temp_track_list.push_back(trackingHitList[track[kHit] - n_remove]);
        trackingHitList.erase(trackingHitList.begin() + track[kHit] - n_remove);
        n_remove++;
      }
      // print trackingHitList
      if (verbose_) {
        ldmx_log(debug) << "====== Tracking hit list (after erase) length "
                        << trackingHitList.size() << " ======";
        for (int i = 0; i < trackingHitList.size(); i++) {
          std::cout << "[" << trackingHitList[i].pos.X() << ", "
                    << trackingHitList[i].pos.Y() << ", "
                    << trackingHitList[i].layer << "] ";
        }
        std::cout << std::endl;
        ldmx_log(debug) << "====== END OF Tracking hit list ======";
      }

      track_list.push_back(temp_track_list);
      // The *current* hit will have been removed, so iHit is currently pointing
      // to the next hit. Decrement iHit so no hits will get skipped by iHit++
      iHit--;
    }
  }

  ldmx_log(debug) << "Straight tracks found (before merge): "
                  << track_list.size();
  if (verbose_) {
    for (int iTrack = 0; iTrack < track_list.size(); iTrack++) {
      ldmx_log(debug) << "Track " << iTrack << ":";
      for (int iHit = 0; iHit < track_list[iTrack].size(); iHit++) {
        std::cout << "  Hit " << iHit << ": ["
                  << track_list[iTrack][iHit].pos.X() << ", "
                  << track_list[iTrack][iHit].pos.Y() << ", "
                  << track_list[iTrack][iHit].layer << "]" << std::endl;
      }
      std::cout << std::endl;
    }
  }

  // Optional addition:  Merge nearby straight tracks.  Not necessary for veto.
  // Criteria:  consider tail of track.  Merge if head of next track is 1/2
  // layers behind, within 1 cell of xy position.
  ldmx_log(debug) << "Beginning track merging using " << track_list.size()
                  << " tracks";

  for (int track_i = 0; track_i < track_list.size(); track_i++) {
    // for each track, check the remainder of the track list for compatible
    // tracks
    std::vector<ldmx::HitData> base_track = track_list[track_i];
    ldmx::HitData tail_hitdata = base_track.back();  // xylayer of last hit in track
    if (verbose_) ldmx_log(debug) << "  Considering track " << track_i;
    for (int track_j = track_i + 1; track_j < track_list.size(); track_j++) {
      std::vector<ldmx::HitData> checking_track = track_list[track_j];
      ldmx::HitData head_hitdata = checking_track.front();
      // if 1-2 layers behind, and xy within one cell...
      if ((head_hitdata.layer == tail_hitdata.layer + 1 ||
           head_hitdata.layer == tail_hitdata.layer + 2) &&
          pow(pow(head_hitdata.pos.X() - tail_hitdata.pos.X(), 2) +
                  pow(head_hitdata.pos.Y() - tail_hitdata.pos.Y(), 2),
              0.5) <= cellWidth) {
        // ...then append the second track to the first one and delete it
        // NOTE:  TO ADD:  (trackingHitList[iHit].pos -
        // trackingHitList[jHit].pos).Mag()
        if (verbose_) {
          ldmx_log(debug) << "     ** Compatible track found at index "
                          << track_j;
          ldmx_log(debug) << "     Tail xylayer: " << head_hitdata.pos.X()
                          << "," << head_hitdata.pos.Y() << ","
                          << head_hitdata.layer;
          ldmx_log(debug) << "     Head xylayer: " << tail_hitdata.pos.X()
                          << "," << tail_hitdata.pos.Y() << ","
                          << tail_hitdata.layer;
        }
        for (int hit_k = 0; hit_k < checking_track.size(); hit_k++) {
          base_track.push_back(track_list[track_j][hit_k]);
        }
        track_list[track_i] = base_track;
        track_list.erase(track_list.begin() + track_j);
        break;
      }
    }
  }
  nStraightTracks_ = track_list.size();
  // print the track list
  ldmx_log(debug) << "Straight tracks found (after merge): "
                  << nStraightTracks_;
  for (int track_i = 0; track_i < track_list.size(); track_i++) {
    ldmx_log(debug) << "Track " << track_i << ":";
    for (int hit_i = 0; hit_i < track_list[track_i].size(); hit_i++) {
      ldmx_log(debug) << "  Hit " << hit_i << ": ["
                      << track_list[track_i][hit_i].pos.X() << ", "
                      << track_list[track_i][hit_i].pos.Y() << ", "
                      << track_list[track_i][hit_i].layer << "]";
    }
  }

  auto straight_tracks = std::chrono::high_resolution_clock::now();
  profiling_map_["straight_tracks"] +=
      std::chrono::duration<double, std::milli>(straight_tracks - start).count();
  // ------------------------------------------------------
  // Linreg tracking:
  ldmx_log(info) << "Finding linreg tracks from a total of "
                 << trackingHitList.size() << " hits using a radius of "
                 << linreg_radius_ << " mm";

  for (int iHit = 0; iHit < 0; iHit++) {
  //for (int iHit = 0; iHit < trackingHitList.size(); iHit++) {
    // Hits being considered at a given time
    std::vector<int> hitsInRegion;
    TMatrixD Vm(3, 3);
    TMatrixD hdt(3, 3);
    TVector3 slopeVec;
    TVector3 hmean;
    TVector3 hpoint;
    float r_corr_best{0.0};
    // Temp array having 3 potential hits
    int hitNums[3];
    // From the above which are passing the correlation reqs
    int bestHitNums[3];

    hitsInRegion.push_back(iHit);
    // Find all hits within 2 cells of the primary hit:
    for (int jHit = 0; jHit < trackingHitList.size(); jHit++) {
      // Dont try to put hits on the same layer to the lin-reg track
      if (trackingHitList[iHit].pos(2) == trackingHitList[jHit].pos(2)) {
        continue;
      }
      float dstToHit =
          (trackingHitList[iHit].pos - trackingHitList[jHit].pos).Mag();
      // This distance optimized to give the best significance
      // it used to be 2*cellWidth, i.e. 4.81 mm
      // note, the layers in the back have a separation of 22.3
      if (dstToHit <= 2 * linreg_radius_) {
        hitsInRegion.push_back(jHit);
      }
    }
    // Found a track that passed the lin-reg reqs
    bool bestLinRegFound{false};
    if (verbose_) {
      ldmx_log(debug) << "There are " << hitsInRegion.size()
                      << " hits within a radius of " << linreg_radius_ << " mm";
    }
    // Look at combinations of hits within the region (do not consider the same
    // combination twice):
    hitNums[0] = iHit;
    for (int jHitInReg = 1; jHitInReg < hitsInRegion.size() - 1; jHitInReg++) {
      // We require (exactly) 3 hits for the lin-reg track building
      if (hitsInRegion.size() < 3) break;
      hitNums[1] = hitsInRegion[jHitInReg];
      for (int kHitReg = jHitInReg + 1; kHitReg < hitsInRegion.size();
           kHitReg++) {
        hitNums[2] = hitsInRegion[kHitReg];
        for (int hInd = 0; hInd < 3; hInd++) {
          // hmean = geometric mean, subtract off from hits to improve SVD
          // performance
          hmean(hInd) = (trackingHitList[hitNums[0]].pos(hInd) +
                         trackingHitList[hitNums[1]].pos(hInd) +
                         trackingHitList[hitNums[2]].pos(hInd)) /
                        3.0;
        }
        for (int hInd = 0; hInd < 3; hInd++) {
          for (int lInd = 0; lInd < 3; lInd++) {
            hdt(hInd, lInd) =
                trackingHitList[hitNums[hInd]].pos(lInd) - hmean(lInd);
          }
        }

        // Perform "linreg" on selected points
        // Calculate the determinant of the matrix
        double determinant =
            hdt(0, 0) * (hdt(1, 1) * hdt(2, 2) - hdt(1, 2) * hdt(2, 1)) -
            hdt(0, 1) * (hdt(1, 0) * hdt(2, 2) - hdt(1, 2) * hdt(2, 0)) +
            hdt(0, 2) * (hdt(1, 0) * hdt(2, 1) - hdt(1, 1) * hdt(2, 0));
        // Exit early if the matrix is singular (i.e. det = 0)
        if (determinant == 0) continue;
        // Perform matrix decomposition with SVD
        TDecompSVD svdObj(hdt);
        bool decomposed = svdObj.Decompose();
        if (!decomposed) continue;

        // First col of V matrix is the slope of the best-fit line
        Vm = svdObj.GetV();
        for (int hInd = 0; hInd < 3; hInd++) {
          slopeVec(hInd) = Vm[0][hInd];
        }
        // hmean, hpoint are points on the best-fit line
        hpoint = slopeVec + hmean;
        // linreg complete:  Now have best-fit line for 3 hits under
        // consideration Check whether the track is valid:  r^2 must be high,
        // and the track must plausibly originate from the photon
        float closest_e = distTwoLines(hmean, hpoint, e_traj_start, e_traj_end);
        float closest_p = distTwoLines(hmean, hpoint, p_traj_start, p_traj_end);
        // Projected track must be close to the photon; details may change after
        // future study.
        if (closest_p > cellWidth or closest_e < 1.5 * cellWidth) continue;
        // find r^2
        // ~variance
        float vrnc = (trackingHitList[hitNums[0]].pos - hmean).Mag() +
                     (trackingHitList[hitNums[1]].pos - hmean).Mag() +
                     (trackingHitList[hitNums[2]].pos - hmean).Mag();
        // sum of |errors|
        float sumerr =
            distPtToLine(trackingHitList[hitNums[0]].pos, hmean, hpoint) +
            distPtToLine(trackingHitList[hitNums[1]].pos, hmean, hpoint) +
            distPtToLine(trackingHitList[hitNums[2]].pos, hmean, hpoint);
        float r_corr = 1 - sumerr / vrnc;
        // Check whether r^2 exceeds a low minimum r_corr:  "Fake" tracks are
        // still much more common in background, so making the algorithm
        // oversensitive doesn't lower performance significantly
        if (r_corr > r_corr_best and r_corr > .6) {
          r_corr_best = r_corr;
          // Only looking for 3-hit tracks currently
          bestLinRegFound = true;
          for (int k = 0; k < 3; k++) {
            bestHitNums[k] = hitNums[k];
          }
        }
      }  // end loop on hits in the region
    }    // end 2nd loop on hits in the region

    // Continue early if not hits on track
    if (!bestLinRegFound) continue;
    // Otherwise increase the number of lin-reg tracks
    nLinregTracks_++;
    ldmx_log(debug) << " Lin-reg track " << nLinregTracks_;
    for (int finalHitIndx = 0; finalHitIndx < 3; finalHitIndx++) {
      ldmx_log(debug) << "   Hit " << finalHitIndx << " ["
                      << trackingHitList[bestHitNums[finalHitIndx]].pos(0)
                      << ", "
                      << trackingHitList[bestHitNums[finalHitIndx]].pos(1)
                      << ", "
                      << trackingHitList[bestHitNums[finalHitIndx]].pos(2)
                      << "] ";
    }

    // Exclude all hits in a found track from further consideration:
    for (int lHit = 0; lHit < 3; lHit++) {
      trackingHitList.erase(trackingHitList.begin() + bestHitNums[lHit]);
    }
    iHit--;
  }  // end loop on all hits
  ldmx_log(info) << " MIP tracking completed; found " << nStraightTracks_
                  << " straight tracks and " << nLinregTracks_
                  << " lin-reg tracks";

  auto linreg_tracks = std::chrono::high_resolution_clock::now();
  profiling_map_["linreg_tracks"] +=
      std::chrono::duration<double, std::milli>(linreg_tracks - straight_tracks)
          .count();

  auto end = std::chrono::high_resolution_clock::now();
  auto time_diff = end - start;
  processing_time_ +=
      std::chrono::duration<double, std::milli>(time_diff).count();
}

  

  

void EcalMipTrackingProcessor::onProcessEnd() {
  ldmx_log(info) << "AVG Time/Event: " << std::fixed << std::setprecision(2)
                 << processing_time_ / nevents_ << " ms";

  ldmx_log(info) << "Breakdown::";

  ldmx_log(info) << "straight_tracks        Avg Time/Event = " << std::fixed
                 << std::setprecision(3) << profiling_map_["straight_tracks"] / nevents_
                 << " ms";

  ldmx_log(info) << "linreg_tracks        Avg Time/Event = " << std::fixed
                 << std::setprecision(3)
                 << profiling_map_["linreg_tracks"] / nevents_ << " ms";
}
// MIP tracking functions:

float EcalMipTrackingProcessor::distTwoLines(TVector3 v1, TVector3 v2, TVector3 w1,
                                      TVector3 w2) {
  TVector3 e1 = v1 - v2;
  TVector3 e2 = w1 - w2;
  TVector3 crs = e1.Cross(e2);
  if (crs.Mag() == 0) {
    return 100.0;  // arbitrary large number; edge case that shouldn't cause
                   // problems.
  } else {
    return std::abs(crs.Dot(v1 - w1) / crs.Mag());
  }
}

float EcalMipTrackingProcessor::distPtToLine(TVector3 h1, TVector3 p1, TVector3 p2) {
  return ((h1 - p1).Cross(h1 - p2)).Mag() / (p1 - p2).Mag();
}






} // namespace ecal

DECLARE_PRODUCER_NS(ecal, EcalMipTrackingProcessor);