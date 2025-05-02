#include "Hcal/VisiblesVetoProcessor.h"

// LDMX
#include "DetDescr/HcalID.h"
#include "DetDescr/SimSpecialID.h"
#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "Hcal/Event/HcalHit.h"

// C++
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <numeric>

namespace hcal {
  void VisiblesVetoProcessor::buildBDTFeatureVector(const ldmx::VisiblesVetoResult &result) {
    bdtFeatures_.push_back(result.getNLayersHit());
    bdtFeatures_.push_back(result.getXStd());
    bdtFeatures_.push_back(result.getYStd());
    bdtFeatures_.push_back(result.getZStd());
    bdtFeatures_.push_back(result.getXMean());
    bdtFeatures_.push_back(result.getYMean());
    bdtFeatures_.push_back(result.getRMean());
    bdtFeatures_.push_back(result.getIsoHits());
    bdtFeatures_.push_back(result.getIsoEnergy());
    bdtFeatures_.push_back(result.getNReadoutHits());
    bdtFeatures_.push_back(result.getSummedDet());
    bdtFeatures_.push_back(result.getDistFromPhotonProj());
  }

  void VisiblesVetoProcessor::configure(framework::config::Parameters &parameters) {
    verbose_ = parameters.getParameter<bool>("verbose");
    featureListName_ = parameters.getParameter<std::string>("feature_list_name");
    // Load BDT ONNX file
    rt_ = std::make_unique<ldmx::Ort::ONNXRuntime>(
	parameters.getParameter<std::string>("bdt_file"));

    bdtCutVal_ = parameters.getParameter<double>("disc_cut");

    beamEnergyMeV_ = parameters.getParameter<double>("beam_energy");

    // collection and pass names
    collectionName_ = parameters.getParameter<std::string>("collection_name");
    rec_pass_name_ = parameters.getParameter<std::string>("rec_pass_name");
    rec_coll_name_ = parameters.getParameter<std::string>("rec_coll_name");
    recoil_from_tracking_ = parameters.getParameter<bool>("recoil_from_tracking");
    track_pass_name_ = parameters.getParameter<std::string>("track_pass_name");
    track_collection_ = parameters.getParameter<std::string>("track_collection");
    sp_coll_name_ = parameters.getParameter<std::string>("sp_coll_name");
    sp_pass_name_ = parameters.getParameter<std::string>("sp_pass_name");
  }

  bool VisiblesVetoProcessor::in_list(std::vector<int> parents, int a) {
    bool inlist = false;
    for (const int &i : parents) {
      if (i==a) {
	inlist = true;
      }
    }
    return inlist;
  }

  void VisiblesVetoProcessor::clearProcessor() {
    bdtFeatures_.clear();

    nLayersHit_ = 0;
    xStd_ = 0.;
    yStd_ = 0.;
    zStd_ = 0.;
    xMean_ = 0.;
    yMean_ = 0.;
    rMean_ = 0.;
    isoHits_ = 0;
    isoEnergy_ = 0.;
    nReadoutHits_ = 0;
    summedDet_ = 0.;
    rMeanFromPhotonProj_ = 0.;
  }

  void VisiblesVetoProcessor::produce(framework::Event &event) {

    ldmx::VisiblesVetoResult result;

    clearProcessor();

    auto particle_map{event.getMap<int, ldmx::SimParticle>("SimParticles")};
    
    // Get target scoring plane hits for recoil electron
    // Use this to calculate the projected photon line vector
    // This currently uses truth-level information, but it should be replaced
    // by reconstructed tracker information, when available
    std::vector<double> gamma_p(3);
    std::vector<double> gamma_x0(3);
    if (recoil_from_tracking_) {
      auto recoilTracks{event.getCollection<ldmx::Track>(track_collection_)};
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
    else {
      if (event.exists(sp_coll_name_)) {
	std::vector<ldmx::SimTrackerHit> targetSPHits =
	  event.getCollection<ldmx::SimTrackerHit>(sp_coll_name_);
	for (auto const &it : particle_map) {
	  for (auto const &sphit : targetSPHits) {
	    if (sphit.getPosition()[2] > 0) {
	      if (it.first == sphit.getTrackID()) {
		if (it.second.getPdgID() == 11 && in_list(it.second.getParents(), 0)) {
		  /* Since SP hit positions are stored as floats and gamma_x0 is
		   a double vector, the conversion here is a little convolcuted. */
		  std::vector<float> x0f = sphit.getPosition();
		  std::vector<double> x0d(x0f.begin(), x0f.end());
		  gamma_x0 = x0d;
		  gamma_p[0] = -1.*sphit.getMomentum()[0];
		  gamma_p[1] = -1.*sphit.getMomentum()[1];
		  gamma_p[2] = beamEnergyMeV_ - sphit.getMomentum()[2];
		}
	      }
	    }
	  }
	}
      }
    }

    // Get Hcal reconstructed hits and loop through them to build features
    const std::vector<ldmx::HcalHit> hcalRecHits =
      event.getCollection<ldmx::HcalHit>(rec_coll_name_, rec_pass_name_);

    double zMean = 0.; // need this when calculating zStd_
    std::vector<int> layersHit;
    for (const ldmx::HcalHit & hit : hcalRecHits) {
      if (hit.getEnergy() > 0.) {
	ldmx::HcalID detID(hit.getID());
	if (detID.getSection() != 0) { // skip hits that aren't in main Hcal
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

      rMeanFromPhotonProj_ /= summedDet_;
    }

    for (const ldmx::HcalHit &hit : hcalRecHits) {
      if (hit.getEnergy() > 0.) {
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

    result.setVariables(
			nLayersHit_,
			xStd_,
			yStd_,
			zStd_,
			xMean_,
			yMean_,
			rMean_,
			isoHits_,
			isoEnergy_,
			nReadoutHits_,
			summedDet_,
			rMeanFromPhotonProj_);

    buildBDTFeatureVector(result);

    ldmx::Ort::FloatArrays inputs({bdtFeatures_});
    float pred = rt_->run({featureListName_}, inputs, {"probabilities"})[0].at(1);
    ldmx_log(info) << " Visibles BDT was ran, score is " << pred;

    event.add(collectionName_, result);
  }

  void VisiblesVetoProcessor::saveAsCSV(const std::string& filename) {
    // Open a new file to be appended
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) {
      std::cerr << "Error: Could not open file " << filename << std::endl;
      return;
    }
    
    // Write features to file
    for (int i = 0; i < bdtFeatures_.size(); ++i) {
      file << bdtFeatures_[i] << (i + 1 == bdtFeatures_.size() ? "\n" : ",");
    }
  }
  
}
