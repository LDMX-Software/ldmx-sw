
#include "DQM/EcalDigiVerifier.h"

#include "DetDescr/EcalID.h"
#include "Ecal/Event/EcalHit.h"
#include "SimCore/Event/SimCalorimeterHit.h"

namespace dqm {

void EcalDigiVerifier::configure(framework::config::Parameters &ps) {
  ecalSimHitColl_ = ps.getParameter<std::string>("ecalSimHitColl");
  ecalSimHitPass_ = ps.getParameter<std::string>("ecalSimHitPass");
  ecalRecHitColl_ = ps.getParameter<std::string>("ecalRecHitColl");
  ecalRecHitPass_ = ps.getParameter<std::string>("ecalRecHitPass");

  return;
}

void EcalDigiVerifier::analyze(const framework::Event &event) {
  // get truth information sorted into an ID based map
  std::vector<ldmx::SimCalorimeterHit> ecalSimHits =
      event.getCollection<ldmx::SimCalorimeterHit>(ecalSimHitColl_,
                                                   ecalSimHitPass_);

  // sort sim hits by ID
  std::sort(ecalSimHits.begin(), ecalSimHits.end(),
            [](const ldmx::SimCalorimeterHit &lhs,
               const ldmx::SimCalorimeterHit &rhs) {
              return lhs.getID() < rhs.getID();
            });

  std::vector<ldmx::EcalHit> ecalRecHits =
      event.getCollection<ldmx::EcalHit>(ecalRecHitColl_, ecalRecHitPass_);

  // sort rec hits by ID
  std::sort(ecalRecHits.begin(), ecalRecHits.end(),
            [](const ldmx::EcalHit &lhs, const ldmx::EcalHit &rhs) {
              return lhs.getID() < rhs.getID();
            });

  int numRecHits{0};
  int numNoiseHits{0};
  double totalRecEnergy{0.};
  int numModWith0Hits{0};
  int numModWith1Hits{0};
  int numModWith2Hits{0};
  int numModWithMoreThan2Hits{0};
  std::vector<int> myCostumModIDs;
  // I need a set for the case when there are repeated elements
  std::set<int> myCostumModIDsSet;

  // Loop on the ecal rechits
  for (const ldmx::EcalHit &recHit : ecalRecHits) {
    numRecHits++;

    // Building up an ID that has layer + module information
    ldmx::EcalID ecal_id(recHit.getID());
    int layer = ecal_id.layer() + 1;
    int moduleID = ecal_id.getModuleID() + 1;
    int myModConstumID = layer * 100 + moduleID;

    myCostumModIDs.push_back(myModConstumID);
    myCostumModIDsSet.insert(myModConstumID);

    // Measure the sum energy of all rechits (inc noise)
    totalRecEnergy += recHit.getEnergy();

    // skip anything that digi flagged as noise
    if (recHit.isNoise()) {
      numNoiseHits++;
      histograms_.fill("is_noise_hit", 1.);
      continue;
    }
    histograms_.fill("is_noise_hit", 0.);

    int rawID = recHit.getID();

    // energy weighted positions
    float sim_pos_x_weighted = 0.;
    float sim_pos_y_weighted = 0.;
    float sim_pos_z_weighted = 0.;

    // get information for this hit
    int numSimHits = 0;
    double totalSimEDep = 0.;
    for (const ldmx::SimCalorimeterHit &simHit : ecalSimHits) {
      if (rawID == simHit.getID()) {
        numSimHits += simHit.getNumberOfContribs();
        totalSimEDep += simHit.getEdep();
        sim_pos_x_weighted += simHit.getPosition()[0] * simHit.getEdep();
        sim_pos_y_weighted += simHit.getPosition()[1] * simHit.getEdep();
        sim_pos_z_weighted += simHit.getPosition()[2] * simHit.getEdep();

      } else if (rawID < simHit.getID()) {
        // later sim hits - all done
        break;
      }
    } // end loop on sim hits

    sim_pos_x_weighted /= totalSimEDep;
    sim_pos_y_weighted /= totalSimEDep;
    sim_pos_z_weighted /= totalSimEDep;
    auto residualX = recHit.getXPos() - sim_pos_x_weighted;
    auto residualY = recHit.getYPos() - sim_pos_y_weighted;
    auto residualZ = recHit.getZPos() - sim_pos_z_weighted;
    histograms_.fill("rec_sim_hit_residual_x", residualX);
    histograms_.fill("rec_sim_hit_residual_y", residualY);
    histograms_.fill("rec_sim_hit_residual_z", residualZ);
    histograms_.fill("rec_sim_hit_residual_x:layer", residualX, layer);
    histograms_.fill("rec_sim_hit_residual_y:layer", residualY, layer);
    histograms_.fill("rec_sim_hit_residual_z:layer", residualZ, layer);
    histograms_.fill("num_sim_hits_per_cell", numSimHits);
    histograms_.fill("sim_edep:rec_amplitude", totalSimEDep,
                     recHit.getAmplitude());
    histograms_.fill("sim_edep:rec_energy", totalSimEDep,
                     recHit.getEnergy());
  }

  std::map<int, int> moduleHits;
  for (const int &myCostumModID : myCostumModIDs) {
    moduleHits[myCostumModID]++;
  }

  // all modules is 34*7 = 238
  // this would be nice if not hardcoded...
  numModWith0Hits = 34 * 7 - myCostumModIDsSet.size();

  for (const auto &moduleHit : moduleHits) {
    if (moduleHit.second == 1) {
      numModWith1Hits++;
    } else if (moduleHit.second == 2) {
      numModWith2Hits++;
    } else if (moduleHit.second > 2) {
      histograms_.fill("num_hit_if_more_than_2hits", moduleHit.second);
      numModWithMoreThan2Hits++;
    }
  }

  histograms_.fill("num_rec_hits", numRecHits);
  histograms_.fill("num_noise_hits", numNoiseHits);
  histograms_.fill("total_rec_energy", totalRecEnergy);

  histograms_.fill("num_mod_with_0hits", numModWith0Hits);
  // only fill the histograms in the case there are hits, otherwise it goes to
  // the other categories
  if (numModWith1Hits > 0)
    histograms_.fill("num_mod_with_1hits", numModWith1Hits);
  if (numModWith2Hits > 0)
    histograms_.fill("num_mod_with_2hits", numModWith2Hits);
  if (numModWithMoreThan2Hits > 0)
    histograms_.fill("num_mod_with_more_than_2hits", numModWithMoreThan2Hits);

  if (totalRecEnergy > 6000.) {
    setStorageHint(framework::hint_should_keep);
  } else {
    setStorageHint(framework::hint_should_drop);
  }

  return;
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::EcalDigiVerifier);
