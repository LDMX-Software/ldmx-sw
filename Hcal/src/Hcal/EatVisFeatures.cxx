#include "Hcal/EaTVisFeatures.h"

// LDMX
#include "DetDescr/HcalID.h"
#include "DetDescr/SimSpecialID.h"
#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "Hcal/Event/HcalHit.h"
#include "Ecal/Event/EcalHit.h"
#include "Tracking/Event/Track.h"
#include "SimCore/Event/SimCalorimeterHit.h"

// C++
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <numeric>

namespace hcal {

  void EaTVisFeatures::configure(framework::config::Parameters &parameters) {
    training_ = parameters.getParameter<bool>("training");
    trainingFile_ = parameters.getParameter<std::string>("training_file");

    beamEnergyMeV_ = parameters.getParameter<double>("beam_energy");

    // collection names
    hcal_rec_collection_ = parameters.getParameter<std::string>("hcal_rec_coll_name");
    ecal_rec_collection_ = parameters.getParameter<std::string>("ecal_rec_coll_name");
    recoil_from_tracking_ = parameters.getParameter<bool>("recoil_from_tracking");
    track_collection_ = parameters.getParameter<std::string>("track_collection");
    sp_collection_ = parameters.getParameter<std::string>("sp_coll_name");
    default_pass_name_ = parameters.getParameter<std::string>("default_pass_name");
    //sim_collection_ = parameters.getParameter<std::string>("sim_collection");
    
  }

  bool EaTVisFeatures::in_list(std::vector<int> parents, int a) {
    bool inlist = false;
    for (const int &i : parents) {
      if (i == a) {
	inlist = true;
      }
    }
    return inlist;
  }

  
  void EaTVisFeatures::analyze(const framework::Event &event) { //looping over events

    std::vector<double> bdtFeatures_;

    //Going to bake in a routine to try and ID the A' so we can grab the vertex. Will obv only fill for signal. We also grab the endpoint here, but given the uniform decay there isn't a ton of info to be gleaned. 
    
    const auto &particle_map{event.getMap<int, ldmx::SimParticle>("SimParticles", default_pass_name_)};
    //printf("Size of particle map: %ld\n", particle_map.size());
    double ap_energy;
    double ap_vertex_z;
    double ap_endpoint_z;
    double ap_flight_dist;
    double number_aps = 0;
    for (auto const& it: particle_map){ //looping over all particles
      int trackid = it.first;
      ldmx::SimParticle p = it.second;
      int pdgid = p.getPdgID();
      if(pdgid == 622) { //selecting ones who are A'
	number_aps += 1;
	std::vector<int> ap_parent_track_ids = p.getParents();
	std::vector<int> ap_daughter_track_ids = p.getDaughters();
	ap_energy = p.getEnergy();
	ap_vertex_z = p.getVertex()[2];
	ap_endpoint_z = p.getEndPoint()[2];
	//generally very forward but for full consistency should use full magnitude
	ap_flight_dist = sqrt(pow((p.getVertex()[0]-p.getEndPoint()[0]),2) + pow((p.getVertex()[1]-p.getEndPoint()[1]),2) + pow((p.getVertex()[2]-p.getEndPoint()[2]),2));
	//Fill info about the A' we've found. Scripting of this technically allows for multiple A's per event to be found (very rare if not impossible)
	histograms_.fill("ap_energy", ap_energy);
	histograms_.fill("ap_vertex_z", ap_vertex_z);
	histograms_.fill("ap_endpoint_z", ap_endpoint_z);
	histograms_.fill("ap_flight_dist_all", ap_flight_dist);

	//Now lets do some quick validation -> determine if the DB came from beam electron (trackid = 1, else, from some other electron)
	for (auto const& ap_parent_track_id : ap_parent_track_ids) {
	  histograms_.fill("ap_parent_track_id", ap_parent_track_id);
	  
	  if (ap_parent_track_id != 1) {
	    histograms_.fill("non_beam_parents", ap_parent_track_id);
	    histograms_.fill("non_beam_parents_ap_vertex", ap_vertex_z);
	    histograms_.fill("non_beam_parents_ap_energy", ap_energy);
	  }
	  bool found = false;
	  for (auto const& parent_candidate: particle_map){ //looping over all particles
	    int particle_track_id = parent_candidate.first;
	    ldmx::SimParticle parent_cand_particle = parent_candidate.second;
	    if (particle_track_id == ap_parent_track_id) {
	      found = true;
	      int parent_pdgid;
	      double parent_energy;
	      parent_pdgid = parent_cand_particle.getPdgID(); //this is mildly silly but make for certain it is electrons we are grabbing for the parents
	      parent_energy = parent_cand_particle.getEnergy(); //now grab the energy of the parent. Should expect distribution from 4 GeV - 8 GeV if upstream effects are occuring
	      
	      histograms_.fill("ap_parent_pdgids", parent_pdgid);
	      histograms_.fill("ap_parent_energies", parent_energy);

	      //Now lets grab the other daughters of the parent, and see if one is an electron. One will be A'
	      std::vector<int> ap_parent_all_daughters = parent_cand_particle.getDaughters();

	      //Loop over these other daughters
	      //printf("\n\n Scanning a new event\n");
	      //printf("A' Parent Trackid -> %d\n", particle_track_id);
	      double energy_sum = 0.;
	      for (auto const& ap_parent_daughter : ap_parent_all_daughters) {
		//Loop over all particles again (smh) and check the pdgids of the other daughters of the A' parent
		for (auto const& daughter_candidate: particle_map){ //looping over all particles
		  int daughter_cand_track_id = daughter_candidate.first;
		  ldmx::SimParticle daughter_candidate_particle = daughter_candidate.second;
		  if (daughter_cand_track_id == ap_parent_daughter) {
		    //printf("found a ap parent daughter pdgid -> %d, energy -> %lf \n", daughter_candidate_particle.getPdgID(), daughter_candidate_particle.getEnergy());
		    energy_sum += daughter_candidate_particle.getEnergy();
		  }		
		}
	      }
	      if (energy_sum > 0) {
		//printf("\nenergy sum = %lf\n", energy_sum);
		histograms_.fill("ap_parent_all_daught_esum", energy_sum);
	      }
	      //Lets try and do something similar with the target scoring plane hits (specifically for TaT)
	      const auto &target_sp_hits = event.getCollection<ldmx::SimTrackerHit>(sp_collection_, default_pass_name_);
	      for (auto const &sphit : target_sp_hits) {
		if (sphit.getTrackID() == particle_track_id) {
		  if (sphit.getPosition()[2] < 0) {
		    histograms_.fill("pre_target_energy_ap_parent", sphit.getEnergy());
		    //printf("Found a scoring plane hit for a A' parent\n");
		    //printf("Sphit energy -> %lf\n", sphit.getEnergy());
		    if (sphit.getEnergy() < 7000) {
		      printf("Found a case of < 7000\n");
		      printf("A' vertex in this case: %lf\n", ap_vertex_z);
		    }
		  }
		}
	      }
	    } 
	  }
	}
	//Checking that the daughters of the A' are e+e- (pdgid -11 and 11) 
	for (auto const& ap_daughter_track_id : ap_daughter_track_ids) {
	  for (auto const& it_: particle_map){
	    int trackid_ = it_.first;
	    ldmx::SimParticle p_ = it_.second;
	    int pdgid_ = p_.getPdgID();
	    if (trackid_ == ap_daughter_track_id) {
	      histograms_.fill("ap_daughter_pdgids", pdgid_);
	    }
	  }
	}
      }
    }
    histograms_.fill("number_aps", number_aps);
  
    //Usually have mean distance feature here, no such feature!
    
    // Get EcalRecHits, check that trigger is passed, make sure these are proper collection
    std::vector<ldmx::EcalHit> ecalRecHits = event.getCollection<ldmx::EcalHit>(ecal_rec_collection_, default_pass_name_);
    std::vector<ldmx::HcalHit> hcalRecHits = event.getCollection<ldmx::HcalHit>(hcal_rec_collection_, default_pass_name_);

    double ecalE_up = 0.; //Ecal Upstream Energy
    double ecalE_tot = 0.; //Ecal Total Energy 
    double hcalE = 0.; //Hcal Total Energy 
    
    //Here we calculate Ecal Upstream Energy (z<500 or up to layer 20), for triggering purposes.
    for (const ldmx::EcalHit & hit : ecalRecHits) {
      if (hit.getEnergy() > 0. && hit.getZPos() <= 500.) {
	ecalE_up += hit.getEnergy();
	histograms_.fill("ecalHitz",hit.getZPos());
      }
    }
    histograms_.fill("ecalE_up", ecalE_up);
    
    //Here we calculate Ecal total energy (for debugging etc)
    for (const ldmx::EcalHit &hit : ecalRecHits) {
      if (hit.getEnergy() > 0.) {
	ecalE_tot += hit.getEnergy();
      }
    }
    histograms_.fill("ecalE_tot", ecalE_tot);
    
    //Here we calculate the total Hcal Energy (no containment cut)
    for (const ldmx::HcalHit &hit : hcalRecHits) {
      if (hit.getEnergy() > 0.) {
	ldmx::HcalID detID(hit.getID());
	if (detID.getSection() != 0) {
            continue;
	}
	hcalE += 12.*hit.getEnergy();
      }
    }
    histograms_.fill("hcalE", hcalE);

    //Trigger + Basic Cuts Analysis (plots decay dist for signal and energy for background, will want to implement some other method too)
    if (ecalE_up < 3160) {
      histograms_.fill("ap_flight_dist_trigger",ap_flight_dist);
      histograms_.fill("trigger_ecalE", ecalE_tot);
      if (ecalE_tot < 3160) {
	histograms_.fill("ap_flight_dist_ecalE", ap_flight_dist);
	histograms_.fill("ecal_req_ecalE", ecalE_tot);
	if (hcalE > 4840) {
	  histograms_.fill("ap_flight_dist_hcalE", ap_flight_dist);
	  histograms_.fill("hcal_req_hcalE", hcalE);
	  //for (auto const& ap_parent_track_id : ap_parent_track_ids) {
	  //  histograms_.fill("pass_cut_ap_parent_track_id", ap_parent_track_id);
	  //}
	}	  
      }
    }

    if (ecalE_tot < 3160 && hcalE > 4840) { //contains trigger, ecal energy, and hcal energy
      // initialize all of the features
      int nLayersHit_ = 0;
      double xStd_ = 0.;
      double yStd_ = 0.;
      double zStd_ = 0.;
      double xMean_ = 0.;
      double yMean_ = 0.;
      double rMean_ = 0.;
      int isoHits_ = 0;
      double isoEnergy_ = 0.;
      int nReadoutHits_ = 0;
      double summedDet_ = 0.;

      double zMean = 0.; // need this when calculating zStd_
      std::vector<int> layersHit;
      for (const ldmx::HcalHit & hit : hcalRecHits) {
	if (hit.getEnergy() > 0.) {
	  ldmx::HcalID detID(hit.getID());
	  if (detID.getSection() != 0) { // skip hits that aren't in main Hcal
	    continue;
	  }
	  if (abs(hit.getXPos()) > 1000 || abs(hit.getYPos()) > 1000) {
	    continue;
	  }
	  nReadoutHits_ += 1;
	  double x = hit.getXPos();
	  double y = hit.getYPos();
	  double z = hit.getZPos();
	  double r = sqrt(pow(x,2) + pow(y,2));
	  
	  summedDet_ += hit.getEnergy();
	  
	  xMean_ += x*hit.getEnergy();
	  yMean_ += y*hit.getEnergy();
	  zMean  += z*hit.getEnergy();
	  rMean_ += r*hit.getEnergy();
	  
	  // check if this is a new layer in the collection
	  if (!(std::find(layersHit.begin(), layersHit.end(), detID.getLayerID()) != layersHit.end())) {
	    layersHit.push_back(detID.getLayerID());
	  }

	  // Calculate isolated hits
	  double closestpoint = 9999.;
	  for (const ldmx::HcalHit &hit2 : hcalRecHits) {
	    if (hit2.getEnergy() > 0.) {
	      ldmx::HcalID detID2(hit2.getID());
	      if (abs(hit2.getXPos()) > 1000 || abs(hit2.getYPos()) > 1000) {
		continue;
	      }
	      if (detID2.getLayerID() == detID.getLayerID()) {
		// Determine if a bar is vertical (along y-axis) or horizontal (along x-axis)
		// Odd layers have horizontal strips
		// Even layers have vertical strips
		if (detID2.getLayerID() % 2 == 0) {
		  if (abs(hit2.getYPos() - y) > 0) {
		    if (abs(hit2.getYPos() - y) < closestpoint) {
		      closestpoint = abs(hit2.getYPos() - y);
		    }
		  }
		}
		else {
		  if (abs(hit2.getXPos() - x) > 0) {
		    if (abs(hit2.getXPos() - x) < closestpoint) {
		      closestpoint = abs(hit2.getXPos() - x);
		    }
		  }
		}
	      }
	    }
	  }
	  if (closestpoint > 50.) {
	    isoHits_ += 1;
	    isoEnergy_ += hit.getEnergy();
	  }
	}
      }
  
      nLayersHit_ = layersHit.size();

      if (summedDet_ > 0.) {
	xMean_ /= summedDet_;
	yMean_ /= summedDet_;
	zMean  /= summedDet_;
	rMean_ /= summedDet_;
      }

      for (const ldmx::HcalHit &hit : hcalRecHits) {
	if (hit.getEnergy() > 0.) {
	  if (abs(hit.getXPos()) > 1000 || abs(hit.getYPos()) > 1000) {
            continue;
          }
	  ldmx::HcalID detID(hit.getID());
	  if (detID.getSection() == 0) {
	    xStd_ += hit.getEnergy()*pow(hit.getXPos()-xMean_,2);
	    yStd_ += hit.getEnergy()*pow(hit.getYPos()-yMean_,2);
	    zStd_ += hit.getEnergy()*pow(hit.getZPos()-zMean ,2);
	  }
	}
      }
      
      if (summedDet_ > 0.) {
	xStd_ = sqrt(xStd_/summedDet_);
	yStd_ = sqrt(yStd_/summedDet_);
	zStd_ = sqrt(zStd_/summedDet_);
      }
      // Fill histograms
      histograms_.fill("layershit", nLayersHit_);
      histograms_.fill("xStd", xStd_);
      histograms_.fill("yStd", yStd_);
      histograms_.fill("zStd", zStd_);
      histograms_.fill("xMean", xMean_);
      histograms_.fill("yMean", yMean_);
      histograms_.fill("rMean", rMean_);
      histograms_.fill("isoHits", isoHits_);
      histograms_.fill("isoE", isoEnergy_);
      histograms_.fill("nHits", nReadoutHits_);
      histograms_.fill("Etot", hcalE);
      return;
    }
  }
  
} // namespace hcal

DECLARE_ANALYZER(hcal::EaTVisFeatures);
