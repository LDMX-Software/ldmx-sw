#include "Hcal/VisiblesCutflow.h"

// LDMX
#include "DetDescr/HcalID.h"
#include "DetDescr/SimSpecialID.h"
#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "Hcal/Event/HcalHit.h"
#include "Ecal/Event/EcalHit.h"
#include "Tracking/Event/Track.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "Ecal/Event/EcalVetoResult.h"

#include "DetDescr/EcalID.h"

// C++
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <numeric>

namespace hcal {

  void VisiblesCutflow::configure(framework::config::Parameters &parameters) {
    rt_ = std::make_unique<ldmx::Ort::ONNXRuntime>(
        parameters.getParameter<std::string>("bdt_file"));
    featureListName_ = parameters.getParameter<std::string>("feature_list_name");
    bdtCutVal_ = parameters.getParameter<double>("disc_cut");
    ecalbdtCutVal_ = parameters.getParameter<double>("ecal_disc_cut");

    beamEnergyMeV_ = parameters.getParameter<double>("beam_energy");

    // collection names
    hcal_rec_collection_ = parameters.getParameter<std::string>("hcal_rec_coll_name");
    hcal_rec_pass_name_ = parameters.getParameter<std::string>("hcal_rec_pass_name");

    ecal_rec_collection_ = parameters.getParameter<std::string>("ecal_rec_coll_name");
    ecal_rec_pass_name_ = parameters.getParameter<std::string>("ecal_rec_pass_name");

    recoil_from_tracking_ = parameters.getParameter<bool>("recoil_from_tracking");
    track_collection_ = parameters.getParameter<std::string>("track_collection");
    track_pass_name_ = parameters.getParameter<std::string>("track_pass_name");

    sp_collection_ = parameters.getParameter<std::string>("sp_coll_name");
    sp_pass_name_ = parameters.getParameter<std::string>("sp_pass_name");
    sim_particles_pass_name_ = parameters.getParameter<std::string>("sim_particles_pass_name");

    ecal_veto_collection_ = parameters.getParameter<std::string>("ecal_veto_coll_name");
    ecal_veto_pass_ = parameters.getParameter<std::string>("ecal_veto_pass_name");
    
  }

  bool VisiblesCutflow::in_list(std::vector<int> parents, int a) {
    bool inlist = false;
    for (const int &i : parents) {
      if (i == a) {
	inlist = true;
      }
    }
    return inlist;
  }


  void VisiblesCutflow::analyze(const framework::Event &event) {

    std::vector<float> bdtFeatures_;

    double decayz_;
    
    auto particle_map{event.getMap<int, ldmx::SimParticle>("SimParticles", sim_particles_pass_name_)};

    for (auto const &it : particle_map) {
      std::vector<int> parents = it.second.getParents();
      for (int trackid : parents) {
	if (trackid == 0 && it.second.getPdgID() == -11) {
	  decayz_ = it.second.getVertex()[2];
	}
      }
    }

    histograms_.fill("totalevents", decayz_);
    
    // Get target scoring plane hits for recoil electron
    // Use this to calculate the projected photon line vector
    // This currently uses truth-level information, but it should be replaced
    // by reconstructed tracker information, when available
    std::vector<double> gamma_p(3);
    double gamma_E = 90000.;
    std::vector<double> gamma_x0(3);

    std::vector<double> recoil_p(3);
    bool foundRecoilE = false;
    
    if (recoil_from_tracking_) {
      auto recoilTracks{event.getCollection<ldmx::Track>(track_collection_, track_pass_name_)};
      // Fill this in later when you know how to use it
      for (auto &track : recoilTracks) {
	// need to figure out how to best isolate candidate electron track
	if (track.q() == 1 && track.getNhits() == 5) {
	  gamma_x0 = track.getPosition();
	  gamma_p[0] = -1.*track.getMomentum()[0];
	  gamma_p[1] = -1.*track.getMomentum()[1];
	  gamma_p[2] = 8000. - track.getMomentum()[2];
	}
      }
    }
    else{
      if (event.exists(sp_collection_, sp_pass_name_)) {
	std::vector<ldmx::SimTrackerHit> targetSPHits =
	  event.getCollection<ldmx::SimTrackerHit>(sp_collection_, sp_pass_name_);
	bool foundRec = false;
	for (auto const &it : particle_map) {
	  for (auto const &sphit : targetSPHits) {
	    if (sphit.getPosition()[2] > 0) {
	      if (it.first == sphit.getTrackID()) {
		if (it.second.getPdgID() == 622) {
		  std::vector<float> x0f = sphit.getPosition();
		  std::vector<double> x0d(x0f.begin(), x0f.end());
		  gamma_x0 = x0d;
		  gamma_p = sphit.getMomentum();
		  gamma_E = sphit.getEnergy();
		  foundRec = true;
		}
		if (it.second.getPdgID() == 11 && in_list(it.second.getParents(), 0)) {
		  if (!foundRec) {
		    std::vector<float> x0f = sphit.getPosition();
		    std::vector<double> x0d(x0f.begin(), x0f.end());
		    gamma_x0 = x0d;
		    gamma_p[0] = -1.*sphit.getMomentum()[0];
		    gamma_p[1] = -1.*sphit.getMomentum()[1];
		    gamma_p[2] = beamEnergyMeV_ - sphit.getMomentum()[2];
		    gamma_E = beamEnergyMeV_ - sphit.getEnergy();
		    foundRec = true;
		  }
		  recoil_p = sphit.getMomentum();
		  foundRecoilE = true;
		}
	      }
	    }
	  }
	}
      }
    }
    histograms_.fill("beamEnergyFrac", gamma_E/8000.);
    histograms_.fill("beamAngle", std::acos(gamma_p[2]/std::sqrt(gamma_p[0]*gamma_p[0] + gamma_p[1]*gamma_p[1] + gamma_p[2]*gamma_p[2])));;

    double pMag = 0.;
    if (foundRecoilE) {
      pMag = std::sqrt(recoil_p[0]*recoil_p[0] + recoil_p[1]*recoil_p[1] + recoil_p[2]*recoil_p[2]);
    }

    if (pMag < 50.) {
      return;
    }
    histograms_.fill("acceptance", decayz_);

    // Get EcalRecHits, check that trigger is passed
    std::vector<ldmx::EcalHit> ecalRecHits = event.getCollection<ldmx::EcalHit>(ecal_rec_collection_, ecal_rec_pass_name_);
    std::vector<ldmx::HcalHit> hcalRecHits = event.getCollection<ldmx::HcalHit>(hcal_rec_collection_, hcal_rec_pass_name_);

    double trigger = 0.;
    double ecalE = 0.;
    double hcalE = 0.;
    bool hcalContainment = true;

    for (const ldmx::EcalHit & hit : ecalRecHits) {
      if (hit.getEnergy() > 0.) {
	ecalE += hit.getEnergy();
	if (hit.getZPos() <= 541.722) {
	  trigger += hit.getEnergy();
	}
      }
    }
    for (const ldmx::HcalHit &hit : hcalRecHits) {
      if (hit.getEnergy() > 0.) {
	ldmx::HcalID detID(hit.getID());
	if (detID.getSection() != 0) {
            continue;
	}
	if (detID.getLayerID() == 1 && hit.getPE() > 5) {
	  hcalContainment = false;
	}
	hcalE += 12.*hit.getEnergy();
      }
    }

    if (trigger > 3160.) {
      return;
    }
    histograms_.fill("passtrigger", decayz_);

    if (ecalE > 3160.) {
      return;
    }
    histograms_.fill("ecalenergy", decayz_);

    if (pMag > 2400.) {
      return;
    }
    histograms_.fill("passTrackerVeto", decayz_);

    auto ecalVeto{
		  event.getObject<ldmx::EcalVetoResult>(ecal_veto_collection_, ecal_veto_pass_)};
    if (ecalVeto.getDisc() < ecalbdtCutVal_) {
      return;
    }
    histograms_.fill("passEcalBDT", decayz_);

    if (hcalE < 4840) {
      return;
    }
    histograms_.fill("hcalEnergyReq", decayz_);

    if (!hcalContainment) {
      return;
    }
    histograms_.fill("containment", decayz_);


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
    double rMeanFromPhotonProj_ = 0.;

    double zMean = 0.; // need this when calculating zStd_
    std::vector<int> layersHit;
    for (const ldmx::HcalHit & hit : hcalRecHits) {
      if (hit.getEnergy() > 0.) {
	ldmx::HcalID detID(hit.getID());
	if (detID.getSection() != 0) { // skip hits that aren't in main Hcal
	  continue;
	}
	if (fabs(hit.getXPos()) > 1000 || fabs(hit.getYPos()) > 1000) {
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

	double x_proj = gamma_x0[0] + (z - gamma_x0[2])*gamma_p[0]/gamma_p[2];
	double y_proj = gamma_x0[1] + (z - gamma_x0[2])*gamma_p[1]/gamma_p[2];

	rMeanFromPhotonProj_ += hit.getEnergy()*sqrt(pow(x-x_proj,2) + pow(y-y_proj,2));

	// Calculate isolated hits
	double closestpoint = 9999.;
	for (const ldmx::HcalHit &hit2 : hcalRecHits) {
	  if (hit2.getEnergy() > 0.) {
	    ldmx::HcalID detID2(hit2.getID());
	    if (fabs(hit2.getXPos()) > 1000 || fabs(hit2.getYPos()) > 1000) {
	      continue;
	    }
	    if (detID2.getLayerID() == detID.getLayerID()) {
	      // Determine if a bar is vertical (along y-axis) or horizontal (along x-axis)
	      // Odd layers have horizontal strips
	      // Even layers have vertical strips
	      if (detID2.getLayerID() % 2 == 0) {
		if (fabs(hit2.getYPos() - y) > 0) {
		  if (fabs(hit2.getYPos() - y) < closestpoint) {
		    closestpoint = abs(hit2.getYPos() - y);
		  }
		}
	      }
	      else {
		if (fabs(hit2.getXPos() - x) > 0) {
		  if (fabs(hit2.getXPos() - x) < closestpoint) {
		    closestpoint = fabs(hit2.getXPos() - x);
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

      rMeanFromPhotonProj_ /= summedDet_;
    }
    
    for (const ldmx::HcalHit &hit : hcalRecHits) {
      if (hit.getEnergy() > 0.) {
	if (fabs(hit.getXPos()) > 1000 || fabs(hit.getYPos()) > 1000) {
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
    histograms_.fill("photonProj", rMeanFromPhotonProj_);

    bdtFeatures_.push_back(nLayersHit_);
    bdtFeatures_.push_back(xStd_);
    bdtFeatures_.push_back(yStd_);
    bdtFeatures_.push_back(zStd_);
    bdtFeatures_.push_back(xMean_);
    bdtFeatures_.push_back(yMean_);
    bdtFeatures_.push_back(rMean_);
    bdtFeatures_.push_back(isoHits_);
    bdtFeatures_.push_back(isoEnergy_);
    bdtFeatures_.push_back(nReadoutHits_);
    bdtFeatures_.push_back(hcalE);
    bdtFeatures_.push_back(rMeanFromPhotonProj_);

    ldmx::Ort::FloatArrays inputs({bdtFeatures_});
    float pred = rt_->run({featureListName_}, inputs, {"probabilities"})[0].at(1);

    histograms_.fill("visiblesDisc", pred);

    for (int i = 0; i < 10000; i++) {
      double disc = 0.999 + (double(i)/10000.)*(1. - 0.999);
      histograms_.fill("visiblesDiscHighNorm", 0.999 + ((double(i)+0.5)/10000.)*(1. - 0.999));
      double discroc = 0.99 + (double(i)/10000.)*(1. - 0.99);
      if (pred >= disc) {
	histograms_.fill("visiblesDiscHigh", 0.999 + ((double(i)+0.5)/10000.)*(1.-0.999));
      }
      if (pred >= discroc) {
	histograms_.fill("ROC", discroc);
      }
    }
    histograms_.fill("nevents", 1.);

    histograms_.fill("ecalDiscvsVisDisc", pred, ecalVeto.getDisc());
    
    if (pred < bdtCutVal_) {
      return;
    }
    histograms_.fill("passVisiblesBDT", decayz_);

    return;
    
  }
  
} // namespace hcal

DECLARE_ANALYZER(hcal::VisiblesCutflow);
