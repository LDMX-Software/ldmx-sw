#include "EventDisplay/Objects.h"

namespace eventdisplay {

Objects::Objects() { Initialize(); }

void Objects::Initialize() {
  // packages of event objects to be passed to event display manager
  sim_objects_ = new TEveElementList("Simulation Objects");
  rec_objects_ = new TEveElementList("Reconstruction Objects");
}

void Objects::SetSimThresh(double simThresh) {
  simThresh_ = simThresh;
  TEveElement* spHits = 0;
  spHits = sim_objects_->FindChild("SimParticles leaving ECAL");
  TEveElement::List_i sim;

  for (sim = spHits->BeginChildren(); sim != spHits->EndChildren(); sim++) {
    TEveElement* el = *sim;
    ldmx::SimTrackerHit* sp = (ldmx::SimTrackerHit*)el->GetSourceObject();
    std::vector<double> pVec = sp->getMomentum();
    double p = pow(pow(pVec[0], 2) + pow(pVec[1], 2) + pow(pVec[2], 2), 0.5);
    if (p < simThresh_) {
      el->SetRnrSelf(kFALSE);
    } else {
      el->SetRnrSelf(kTRUE);
    }
  }
}

void Objects::ColorClusters() {
  TEveElement* clusters = rec_objects_->FindChild("ECAL Clusters");
  if (clusters == 0) {
    std::cout << "[ Objects ] : No clusters to color!" << std::endl;
    return;
  }

  int theColor = 0;
  TEveElement::List_i cluster;
  for (cluster = clusters->BeginChildren(); cluster != clusters->EndChildren();
       cluster++) {
    TEveElement* el = *cluster;
    TEveElement::List_i hit;
    Int_t color = 0;
    if (!el->IsPickable()) {
      color = 19;
    } else if (theColor < 9) {
      color = colors_[theColor];
      theColor++;
    } else {
      Int_t ci = 200 * r_.Rndm();
      color = ci;
      std::cout
          << "[ Objects ] : Using random colors to fill in extra clusters."
          << std::endl;
    }

    for (hit = el->BeginChildren(); hit != el->EndChildren(); hit++) {
      TEveElement* elChild = *hit;
      elChild->SetMainColor(color);
    }
  }
}

void Objects::draw(std::vector<ldmx::EcalHit> hits) {
  TEveRGBAPalette* palette = new TEveRGBAPalette(0, 500.0);

  std::sort(hits.begin(), hits.end(),
            [](const ldmx::EcalHit& a, const ldmx::EcalHit& b) {
              return a.getEnergy() < b.getEnergy();
            });

  auto ecal_hits = new TEveElementList("ECAL RecHits");
  for (const ldmx::EcalHit& hit : hits) {
    double energy = hit.getEnergy();

    if (energy == 0) {
      continue;
    }

    TString digiName;
    digiName.Form("%1.5g MeV", energy);

    const UChar_t* rgb = palette->ColorFromValue(energy);
    TColor* aColor = new TColor();
    Int_t color = aColor->GetColor((Int_t)rgb[0], (Int_t)rgb[1], (Int_t)rgb[2]);

    TEveGeoShape* ecalDigiHit = EveShapeDrawer::getInstance().drawHexPrism(
        DetectorGeometry::getInstance().getHexPrism(ldmx::EcalID(hit.getID())),
        0, 0, 0, color, 0, digiName);

    ecal_hits->AddElement(ecalDigiHit);
  }

  ecal_hits->SetPickableRecursively(1);

  rec_objects_->AddElement(ecal_hits);
}

void Objects::draw(std::vector<ldmx::HcalHit> hits) {
  TEveRGBAPalette* palette = new TEveRGBAPalette(0, 100.0);

  std::sort(hits.begin(), hits.end(),
      [](const ldmx::HcalHit& a, const ldmx::HcalHit& b) {
        return a.getEnergy() < b.getEnergy();
      });

  auto hcal_hits = new TEveElementList("HCAL Rec Hits");
  for (const ldmx::HcalHit& hit : hits) {
    int pe = hit.getPE();
    if (pe == 0) {
      continue;
    }

    ldmx::HcalID id(hit.getID());

    const UChar_t* rgb = palette->ColorFromValue(pe);
    TColor* aColor = new TColor();
    Int_t color = aColor->GetColor((Int_t)rgb[0], (Int_t)rgb[1], (Int_t)rgb[2]);

    TString digiName;
    digiName.Form("%d PEs, Section %d, Layer %d, Bar %d, Z %1.5g", pe,
                  id.section(), id.layer(), id.strip(), hit.getZPos());

    BoundingBox hcal_hit_bb =
        DetectorGeometry::getInstance().getBoundingBox(hit);
    TEveGeoShape* hcalDigiHit = EveShapeDrawer::getInstance().drawRectPrism(
        hcal_hit_bb, 0, 0, 0, color, 0, digiName);

    if (hcalDigiHit) {
      if (hit.isNoise()) {
        hcalDigiHit->SetRnrSelf(0);
      }
      hcal_hits->AddElement(hcalDigiHit);
    }  // successfully created hcal digi hit

  }  // loop through sorted hit list

  hcal_hits->SetPickableRecursively(1);
  rec_objects_->AddElement(hcal_hits);
}

void Objects::draw(std::vector<ldmx::EcalCluster> clusters) {
  TEveRGBAPalette* palette = new TEveRGBAPalette(0, 4000.0);

  auto eve_clusters = new TEveElementList("Ecal Clusters");
  int iC = 0;
  for (const ldmx::EcalCluster& cluster : clusters) {
    TString clusterName;
    clusterName.Form("ECAL Cluster %d", iC);

    TEveElement* ecalCluster = new TEveElementList(clusterName);

    double energy = cluster.getEnergy();
    std::vector<unsigned int> clusterHitIDs = cluster.getHitIDs();

    int numHits = clusterHitIDs.size();

    for (int iHit = 0; iHit < numHits; iHit++) {
      ldmx::EcalID id(clusterHitIDs.at(iHit));

      const UChar_t* rgb = palette->ColorFromValue(energy);
      TColor* aColor = new TColor();
      Int_t color =
          aColor->GetColor((Int_t)rgb[0], (Int_t)rgb[1], (Int_t)rgb[2]);

      TEveGeoShape* ecalDigiHit = EveShapeDrawer::getInstance().drawHexPrism(
          DetectorGeometry::getInstance().getHexPrism(id), 0, 0, 0, color, 0,
          "RecHit");
      ecalCluster->AddElement(ecalDigiHit);

      if (numHits < 2) {
        ecalCluster->SetPickableRecursively(0);
      } else {
        ecalCluster->SetPickableRecursively(1);
      }
    }
    eve_clusters->AddElement(ecalCluster);
    iC++;
  }

  eve_clusters->SetPickable(1);
  rec_objects_->AddElement(eve_clusters);
}

void Objects::draw(std::vector<ldmx::SimTrackerHit> hits) {
  // need at least one hit for subdet ID
  if (hits.empty()) return;

  // get id for determining subdet
  ldmx::DetectorID id(hits.at(0).getID());

  if (id.subdet() == ldmx::SubdetectorIDType::SD_TRACKER_RECOIL) {
    // recoil tracker ID
    int iter = 0;
    auto recoil_tracker_hits = new TEveElementList("Recoil Tracker Hits");
    for (const ldmx::SimTrackerHit& hit : hits) {
      std::vector<float> xyzPos = hit.getPosition();
      double energy = hit.getEdep();

      TString recoilName;
      recoilName.Form("Recoil Hit %d", iter);

      TEveGeoShape* recoilHit = EveShapeDrawer::getInstance().drawRectPrism(
          DetectorGeometry::getInstance().getBoundingBox(hit), 0, 0,
          DetectorGeometry::getInstance().getRotAngle(hit.getLayerID(),
                                                      hit.getModuleID()) *
              180 / M_PI,
          kRed + 1, 0, recoilName);
      recoil_tracker_hits->AddElement(recoilHit);

      iter++;
    }
    sim_objects_->AddElement(recoil_tracker_hits);
  } else if (id.subdet() == ldmx::SubdetectorIDType::SD_SIM_SPECIAL) {
    // scoring plane ID

    std::sort(hits.begin(), hits.end(),
        [](const ldmx::SimTrackerHit& a, const ldmx::SimTrackerHit& b) {
          return a.getTrackID() < b.getTrackID();
        });

    auto lastUniqueEntry =
        std::unique(hits.begin(), hits.end(), 
            [](ldmx::SimTrackerHit& a, ldmx::SimTrackerHit& b) {
              return a.getTrackID() == b.getTrackID();
            });

    hits.erase(lastUniqueEntry, hits.end());

    auto leaving_ecal_sim_particles =
        new TEveElementList("SimParticles leaving ECAL");
    for (const ldmx::SimTrackerHit& spHit : hits) {
      std::vector<double> pVec = spHit.getMomentum();
      double p = pow(pow(pVec[0], 2) + pow(pVec[1], 2) + pow(pVec[2], 2), 0.5);

      double E = spHit.getEnergy();

      std::vector<float> simStart = spHit.getPosition();
      std::vector<double> simDir = pVec;
      double rCheck =
          pow(pow(simDir[0], 2) + pow(simDir[1], 2) + pow(simDir[2], 2), 0.5);

      double scale = 1;
      double largest = 0;
      if (abs(simDir[0]) > 3500.0) {
        scale = 500.0 / abs(simDir[0]);
        largest = simDir[0];
      }
      if (abs(simDir[1]) > 3500.0 && abs(simDir[1]) > largest) {
        scale = 500.0 / abs(simDir[1]);
        largest = simDir[1];
      }
      if (abs(simDir[2]) > 3500.0 && abs(simDir[2]) > 3500) {
        scale = 500.0 / abs(simDir[2]);
      }

      double r = pow(pow(scale * (simDir[0]), 2) + pow(scale * (simDir[1]), 2) +
                         pow(scale * (simDir[2]), 2),
                     0.5);
      signed int pdgID = spHit.getPdgID();

      TEveArrow* simArr =
          new TEveArrow(scale * simDir[0], scale * simDir[1], scale * simDir[2],
                        simStart[0], simStart[1], simStart[2]);

      simArr->SetMainColor(kBlack);
      simArr->SetTubeR(60 * 0.02 / r);
      simArr->SetConeL(100 * 0.02 / r);
      simArr->SetConeR(150 * 0.02 / r);
      simArr->SetPickable(kTRUE);
      if (p < simThresh_) {
        simArr->SetRnrSelf(kFALSE);
      }

      TString name;
      name.Form("PDG = %d, p = %1.5g MeV/c", pdgID, p);
      simArr->SetElementName(name);
      leaving_ecal_sim_particles->AddElement(simArr);
    }

    sim_objects_->AddElement(leaving_ecal_sim_particles);
  } else {
    EXCEPTION_RAISE("NotImp",
                    "SimTrackerHit drawing not implemented for subdet " +
                        std::to_string(int(id.subdet())));
  }
}

void Objects::draw(std::vector<ldmx::SimCalorimeterHit> hits) {
  // need at least one hit for subdet ID
  if (hits.empty()) return;

  // get id for determining subdet
  ldmx::DetectorID id(hits.at(0).getID());

  TEveRGBAPalette* palette = new TEveRGBAPalette(0, 100.0);

  std::sort(hits.begin(), hits.end(),
      [](const ldmx::SimCalorimeterHit& a, const ldmx::SimCalorimeterHit& b) {
        return a.getEdep() < b.getEdep();
      });

  if (id.subdet() == ldmx::SubdetectorIDType::SD_HCAL) {
  
    auto hcal_hits = new TEveElementList("HCAL Sim Hits");
    for (const ldmx::SimCalorimeterHit& hit : hits) {
  
      ldmx::HcalID id(hit.getID());
  
      float edep = hit.getEdep();

      const UChar_t* rgb = palette->ColorFromValue(edep);
      TColor* aColor = new TColor();
      Int_t color = aColor->GetColor((Int_t)rgb[0], (Int_t)rgb[1], (Int_t)rgb[2]);

      auto position{hit.getPosition()};
  
      TString digiName;
      digiName.Form("%.2f MeV, Section %d, Layer %d, Bar %d, Z %1.5g", hit.getEdep(),
                    id.section(), id.layer(), id.strip(), position.at(2));
  
      TEveGeoShape* eve_hit = EveShapeDrawer::getInstance().drawRectPrism(
          position.at(0), position.at(1), position.at(2),
          50., 50., 10.,
          0, 0, 0, 
          color, 0, digiName);
  
      if (eve_hit) {
        hcal_hits->AddElement(eve_hit);
      }  // successfully created hcal digi hit
  
    }  // loop through sorted hit list
  
    hcal_hits->SetPickableRecursively(1);
    sim_objects_->AddElement(hcal_hits);
  
  } else if (id.subdet() == ldmx::SubdetectorIDType::SD_ECAL) {
  
    auto ecal_hits = new TEveElementList("ECAL Sim Hits");
    for (const ldmx::SimCalorimeterHit& hit : hits) {
  
      ldmx::EcalID id(hit.getID());
  
      float edep = hit.getEdep();

      const UChar_t* rgb = palette->ColorFromValue(edep);
      TColor* aColor = new TColor();
      Int_t color = aColor->GetColor((Int_t)rgb[0], (Int_t)rgb[1], (Int_t)rgb[2]);
  
      TString digiName;
      digiName.Form("%.2f MeV, Module %d, Layer %d, Cell %d", hit.getEdep(),
                    id.module(), id.layer(), id.cell());

      auto position{hit.getPosition()};
  
      TEveGeoShape* eve_hit = EveShapeDrawer::getInstance().drawHexPrism(
          position.at(0), position.at(1), position.at(2),
          0, 0, 0, 
          1., 2.,
          color, 0, digiName);
  
      if (eve_hit) {
        ecal_hits->AddElement(eve_hit);
      }  // successfully created hcal digi hit
  
    }  // loop through sorted hit list
  
    ecal_hits->SetPickableRecursively(1);
    sim_objects_->AddElement(ecal_hits);

  } else {
    EXCEPTION_RAISE("NotImp",
                    "SimCalorimeterHit drawing not implemented for subdet " +
                        std::to_string(int(id.subdet())));
  }

}

void Objects::draw(std::map<int,ldmx::SimParticle> particles) {

    static const std::map<int,char*> translation = {
      { int(ldmx::SimParticle::ProcessType::unknown), "unknown" },
      { int(ldmx::SimParticle::ProcessType::annihil), "annihil" },
      { int(ldmx::SimParticle::ProcessType::compt), "compt" },
      { int(ldmx::SimParticle::ProcessType::conv), "conv" },
      { int(ldmx::SimParticle::ProcessType::electronNuclear), "electronNuclear" },
      { int(ldmx::SimParticle::ProcessType::eBrem), "eBrem" },
      { int(ldmx::SimParticle::ProcessType::eIoni), "eIoni" },
      { int(ldmx::SimParticle::ProcessType::msc), "msc" },
      { int(ldmx::SimParticle::ProcessType::phot), "phot" },
      { int(ldmx::SimParticle::ProcessType::photonNuclear), "photonNuclear" },
      { int(ldmx::SimParticle::ProcessType::GammaToMuPair), "GammaToMuPair" },
      { int(ldmx::SimParticle::ProcessType::eDarkBrem), "eDarkBrem" }
    };

    
    TString eve_list_name;
    eve_list_name.Form("SimParticles above %.1f MeV", simThresh_);
    auto sim_particles = new TEveElementList(eve_list_name);

    for (auto const& [ track_id, particle] : particles) {
      // skip if KE is less than threshold
      if (particle.getEnergy()-particle.getMass() < simThresh_) continue;

      std::vector<double> pVec = particle.getMomentum();
      double p = pow(pow(pVec.at(0), 2) + pow(pVec.at(1), 2) + pow(pVec.at(2), 2), 0.5);

      std::vector<double> simStart = particle.getVertex();
      std::vector<double> simEnd   = particle.getEndPoint();
      std::vector<double> simDir   = {0.,0.,0.};
      for (int i=0; i<3; i++) simDir[i] = simEnd.at(i)-simStart.at(i);

      TEveArrow* simArr =
          new TEveArrow(simDir.at(0),simDir.at(1),simDir.at(2),
                        simStart.at(0),simStart.at(1),simStart.at(2));

      double r = pow(
            pow((simDir.at(0)), 2) 
          + pow((simDir.at(1)), 2) 
          + pow((simDir.at(2)), 2),
          0.5);

      simArr->SetMainColor(kBlack);
      simArr->SetTubeR(60 * 0.02 / r);
      simArr->SetConeL(100 * 0.02 / r);
      simArr->SetConeR(150 * 0.02 / r);
      simArr->SetPickable(kTRUE);

      int parent_id = 0; //for primaries
      if (not particle.getParents().empty()) parent_id = particle.getParents().at(0);

      TString name;
      name.Form("PDG = %d, p = %1.5g MeV/c from track ID %d via %s process", 
          particle.getPdgID(), p, parent_id,
          translation.at(particle.getProcessType()));
      simArr->SetElementName(name);
      sim_particles->AddElement(simArr);
    }

    sim_objects_->AddElement(sim_particles);
  
}

}  // namespace eventdisplay
