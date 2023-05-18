#include "Tracking/Reco/TruthMatchingTool.h"

namespace tracking {
namespace sim{

TruthMatchingTool::TruthInfo TruthMatchingTool::Evaluate(const std::unordered_map<unsigned int,
                                                         unsigned int>& trk_trackIDs, int n_meas) {

  TruthInfo ti;
  ti.truthProb = 0.;
  ti.trackID = -1;
  ti.pdgID = 0;
  
  for (std::unordered_map<unsigned int, unsigned int>::const_iterator it = trk_trackIDs.begin();
       it != trk_trackIDs.end(); it++) {
    
    double currentTruthProb = (double) it->second / (double) n_meas;
    if (currentTruthProb > ti.truthProb) {
      ti.truthProb = currentTruthProb;
      ti.trackID = it->first;
    }
  }
  
  if (ti.trackID > 0)
    ti.pdgID = map_[ti.trackID].getPdgID();
  
  return ti;
}

TruthMatchingTool::TruthInfo TruthMatchingTool::TruthMatch(const std::vector<ldmx::Measurement>& vmeas) {

  std::unordered_map<unsigned int, unsigned int> trk_trackIDs;
  
  for (auto meas : vmeas) {

    for (auto trkId : meas.getTrackIds()) {
      if (trk_trackIDs.find(trkId) != trk_trackIDs.end()) 
        trk_trackIDs[trkId]++;
      else
        trk_trackIDs[trkId] = 1;
      
    }
  } // loop on measurements

  return Evaluate(trk_trackIDs, vmeas.size());
  
}


/**
 * Performs the truth matching by checking all the trackIDs associated to the measurements on track
 * @param trk The track that needs to be truth matched
 * @return TruthInfo a struct containing the truth information regarding the track
 * Each measurement weights the same. 
 *
 */

TruthMatchingTool::TruthInfo TruthMatchingTool::TruthMatch(const ldmx::Track& trk) {
    
  //Map holding all tracksIds and their frequency
  std::unordered_map<unsigned int, unsigned int> trk_trackIDs;
  
  for (auto measID : trk.getMeasurementsIdxs()) {

    auto meas = measurements_.at(measID);
    if (debug_) {
      std::cout<<"Getting measurement at ID:"<<measID<<std::endl;
      std::cout<<meas<<std::endl;
      std::cout<<meas.getTrackIds().size()<<std::endl;
    }
    
    for (auto trkId : meas.getTrackIds()) {
      
      if (trk_trackIDs.find(trkId) != trk_trackIDs.end()) {
        trk_trackIDs[trkId]++;
      }
      else {
        trk_trackIDs[trkId] = 1;
      }
      
    }//loop on measurements trackIDs


    if (debug_) {
      std::cout<<"The trackIDs maps look like:"<<std::endl;
      for (std::unordered_map<unsigned int, unsigned int>::iterator it = trk_trackIDs.begin();
           it != trk_trackIDs.end(); it++) {
        std::cout<<it->first<<" " <<it->second<<std::endl;
      }
    }
  }//loop on measurements
  
  return Evaluate(trk_trackIDs, trk.getMeasurementsIdxs().size());
  
  
}//Match Track



}
}
