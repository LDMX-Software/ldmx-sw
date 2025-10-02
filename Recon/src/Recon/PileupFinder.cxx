#include "Recon/PileupFinder.h"

#include <vector>

namespace recon {

void PileupFinder::configure(framework::config::Parameters& ps) {
  // I/O
  rec_hit_coll_name_ = ps.getParameter<std::string>("rec_hit_coll_name");
  rec_hit_pass_name_ = ps.getParameter<std::string>("rec_hit_pass_name");
  pf_cand_coll_name_ = ps.getParameter<std::string>("pf_cand_coll_name");
  pf_cand_pass_name_ = ps.getParameter<std::string>("pf_cand_pass_name");
  cluster_coll_name_ = ps.getParameter<std::string>("cluster_coll_name");
  cluster_pass_name_ = ps.getParameter<std::string>("cluster_pass_name");
  output_rec_hit_coll_name_ =
      ps.getParameter<std::string>("output_rec_hit_coll_name");
  // Algorithm configuration
  min_mom_ = ps.getParameter<double>("min_momentum");
}

// get pileup candidates from PFlow and make a cleaned-up hit collection
void PileupFinder::produce(framework::Event& event) {
  if (!event.exists(rec_hit_coll_name_, rec_hit_pass_name_)) {  // ecal rechits
    ldmx_log(error) << "Unable to find (one) collection named "
                    << rec_hit_coll_name_ << "_" << rec_hit_pass_name_;
    return;
  }
  if (!event.exists(pf_cand_coll_name_, pf_cand_pass_name_)) {
    ldmx_log(error) << "Unable to find (one) collection named "
                    << pf_cand_coll_name_ << "_" << pf_cand_pass_name_;
    return;
  }
  if (!event.exists(cluster_coll_name_, cluster_pass_name_)) {
    ldmx_log(error) << "Unable to find (one) collection named "
                    << cluster_coll_name_ << "_" << cluster_pass_name_;
    return;
  }

  const auto& ecal_hits{event.getCollection<ldmx::EcalHit>(rec_hit_coll_name_,
                                                           rec_hit_pass_name_)};

  const auto& pf_cands{event.getCollection<ldmx::PFCandidate>(
      pf_cand_coll_name_, pf_cand_pass_name_)};

  const auto& clusters{event.getCollection<ldmx::CaloCluster>(
      cluster_coll_name_, cluster_pass_name_)};
  // get PID 3 and 7 -- the ones with track and ecal matching
  // get the high-momentum track ones from there -- pileup candidate!
  // get the list of hits associated with pileup candidates
  // if a rechit is not on that list, add to output collection.
  std::vector<ldmx::EcalHit> output_hits;
  std::vector<unsigned int> pileup_hitIDs;

  // this needs to be a two-step procedure: loop over all clusters deemed to be
  // pileup to find all their associated hits
  //  then loop over that list to make a collection of output hits that doesn't
  //  contain them
  for (const auto& pf_cand : pf_cands) {
    if (pf_cand.getPID() == 3 || pf_cand.getPID() == 7) {
      // we have both ecal cluster and track
      std::vector<float> mom_vec = pf_cand.getTrackPxPyPz();
      float mom = mom_vec[0] * mom_vec[0] + mom_vec[1] * mom_vec[1] +
                  mom_vec[2] * mom_vec[2];
      mom = sqrt(mom);

      if (mom < min_mom_) continue;
      ldmx_log(trace) << "Got pileup candidate with PID = " << pf_cand.getPID()
                      << " and momentum = " << mom << " MeV.";

      // now! use the hit-candidate association to get the associated ecal hits.
      int pf_cl_idx = pf_cand.getEcalIndex();
      ldmx_log(trace) << "Got Ecal cluster with index " << pf_cl_idx
                      << " while cluster array length is " << clusters.size();
      if (pf_cl_idx < 0)  // was never set
        continue;
      auto cl = clusters[pf_cl_idx];
      auto hitIDs = cl.getHitIDs();
      // add to collection of pileup hits
      pileup_hitIDs.insert(pileup_hitIDs.end(), hitIDs.begin(), hitIDs.end());
    }  // if trk/ecal matched
  }  // over PF objects

  for (auto hit : ecal_hits) {
    auto foundIndex = std::find(std::begin(pileup_hitIDs),
                                std::end(pileup_hitIDs), hit.getID());
    // When the element is not found, std::find returns the end of the range
    if (foundIndex ==
        std::end(pileup_hitIDs)) {    // hit not part of any pileup cluster
      output_hits.emplace_back(hit);  // keep it
      ldmx_log(trace) << "Got no-pileup hit! ";
    }
  }
  event.add(output_rec_hit_coll_name_, output_hits);
}
void PileupFinder::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";

  return;
}

}  // namespace recon

DECLARE_PRODUCER(recon::PileupFinder);
