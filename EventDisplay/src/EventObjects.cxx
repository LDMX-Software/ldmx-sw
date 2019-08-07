#include "EventDisplay/EventObjects.h"   

namespace ldmx {

    EventObjects::EventObjects() {

        hexReadout_ = new EcalHexReadout();
        drawer_ = new EveShapeDrawer();

        Initialize();
    }

    void EventObjects::Initialize() {
        
        ecalHits_ = new TEveElementList("ECAL RecHits");
        hcalHits_ = new TEveElementList("HCAL RecHits");
        recoilTrackerHits_ = new TEveElementList("Recoil Sim Hits");
        ecalClusters_ = new TEveElementList("ECAL Clusters");
        ecalSimParticles_ = new TEveElementList("ECAL SP Sim Particles");
        hits_ = new TEveElementList("Reco Hits");
        recoObjs_ = new TEveElementList("Reco Objects");

    }

    static bool compEcalHits(const EcalHit* a, const EcalHit* b) {
        return a->getEnergy() > b->getEnergy();
    }

    static bool compHcalHits(const HcalHit* a, const HcalHit* b) {
        return a->getPE() > b->getPE();
    }

    static bool compSimsP(const SimTrackerHit* a, const SimTrackerHit* b) {

        std::vector<double> paVec = a->getMomentum();
        std::vector<double> pbVec = b->getMomentum();

        double pa2 = pow(paVec[0],2)+pow(paVec[1],2)+pow(paVec[2],2);
        double pb2 = pow(pbVec[0],2)+pow(pbVec[1],2)+pow(pbVec[2],2);

        return pa2 > pb2;
    }

    static bool compSims(const SimTrackerHit* a, const SimTrackerHit* b) {

        if (a->getSimParticle() == b->getSimParticle()) {
            return compSimsP(a,b);
        } else {
            return a->getSimParticle() < b->getSimParticle();
        }
    }

    void EventObjects::SetSimThresh(double simThresh) {

        simThresh_ = simThresh;
        TEveElement* spHits = 0;
        spHits = hits_->FindChild("ECAL SP Sim Particles");
        TEveElement::List_i sim;

        for (sim = spHits->BeginChildren(); sim != spHits->EndChildren(); sim++) {
            
            TEveElement* el = *sim;
            SimParticle* sp = (ldmx::SimParticle*)el->GetSourceObject();
            std::vector<double> pVec = sp->getMomentum();
            double p = pow(pow(pVec[0],2) + pow(pVec[1],2) + pow(pVec[2],2),0.5);
            if (p < simThresh_) { 
                el->SetRnrSelf(kFALSE); 
            } else {
                el->SetRnrSelf(kTRUE);
            }
        }
    }

    void EventObjects::ColorClusters() {

        TEveElement* clusters = recoObjs_->FindChild("ECAL Clusters");
        if (clusters == 0) { 
            std::cout << "No clusters to color!" << std::endl;
            return; 
        }

        int theColor = 0;
        TEveElement::List_i cluster;
        for (cluster = clusters->BeginChildren(); cluster != clusters->EndChildren(); cluster++) {
            
            TEveElement* el = *cluster;
            TEveElement::List_i hit;
            Int_t color = 0;
            if (!el->IsPickable()) {
                color = 19;
            } else if (theColor < 9) {
                color = colors_[theColor];
                theColor++;
            } else {
                Int_t ci = 200*r_.Rndm();
                color = ci;
            }

            for (hit = el->BeginChildren(); hit != el->EndChildren(); hit++) { 
                TEveElement* elChild = *hit;
                elChild->SetMainColor(color);
            }
        }
    }

    void EventObjects::drawECALHits(TClonesArray* hits) {

        ldmx::EcalHit* hit;
        TEveRGBAPalette* palette = new TEveRGBAPalette(0,500.0);

        std::vector<EcalHit*> hitVec;
        for (TIter next(hits); hit = (ldmx::EcalHit*)next();) {
            hitVec.push_back(hit);
        }

        std::sort(hitVec.begin(), hitVec.end(), compEcalHits);

        for (int i = 0; i < hitVec.size(); i++) {

            double energy = hitVec[i]->getEnergy();

            if (energy == 0) { continue; }

            TString digiName;
            digiName.Form("%1.5g MeV", energy);

            const UChar_t* rgb = palette->ColorFromValue(energy);
            TColor* aColor = new TColor();
            Int_t color = aColor->GetColor((Int_t)rgb[0], (Int_t)rgb[1], (Int_t)rgb[2]);

            TEveGeoShape* ecalDigiHit = drawer_->drawHexPrism(
                    DETECTOR_GEOMETRY.getHexPrism( hitVec[i] ),
                    0, 0, 0, 
                    color, 0, digiName);

            ecalHits_->AddElement(ecalDigiHit);
        }

        ecalHits_->SetPickableRecursively(1);

        hits_->AddElement(ecalHits_);
    }

    void EventObjects::drawHCALHits(TClonesArray* hits) {


        TEveRGBAPalette* palette = new TEveRGBAPalette(0,100.0);

        std::vector<HcalHit*> hitVec;
        ldmx::HcalHit* hit;
        for (TIter next(hits); hit = (ldmx::HcalHit*)next();) {
            hitVec.push_back(hit);
        }

        std::sort(hitVec.begin(), hitVec.end(), compHcalHits);

        ldmx::HcalID detID;
        for (int i = 0; i < hitVec.size(); i++) {
            int pe = hitVec[i]->getPE();
            if (pe == 0) { continue; }

            detID.setRawValue(hitVec[i]->getID());
            detID.unpack();

            const UChar_t* rgb = palette->ColorFromValue(pe);
            TColor* aColor = new TColor();
            Int_t color = aColor->GetColor((Int_t)rgb[0], (Int_t)rgb[1], (Int_t)rgb[2]);

            int bar = hitVec[i]->getStrip();
            int layer = hitVec[i]->getLayer();
            int section = hitVec[i]->getSection();
            TString digiName;
            digiName.Form("%d PEs, Section %d, Layer %d, Bar %d, Z %1.5g", pe, section, layer, bar, hitVec[i]->getZ());

            BoundingBox hcal_hit_bb = DETECTOR_GEOMETRY.getBoundingBox( hitVec[i] );
            TEveGeoShape *hcalDigiHit = drawer_->drawRectPrism(
                    hcal_hit_bb ,
                    0, 0, 0, color, 0, digiName );

            if ( hcalDigiHit ) {
                if ( hitVec[i]->getNoise() ) { hcalDigiHit->SetRnrSelf(0); }
                hcalHits_->AddElement(hcalDigiHit);
            } // successfully created hcal digi hit

        } //loop through sorted hit list

        hcalHits_->SetPickableRecursively(1);
        hits_->AddElement(hcalHits_);
    }

    void EventObjects::drawRecoilHits(TClonesArray* hits) {

        ldmx::SimTrackerHit* hit;
        int iter = 0;
        for (TIter next(hits); hit = (ldmx::SimTrackerHit*)next();) {

            std::vector<float> xyzPos = hit->getPosition();
            double energy = hit->getEdep();

            TString recoilName;
            recoilName.Form("Recoil Hit %d", iter);

            TEveGeoShape *recoilHit = drawer_->drawRectPrism(
                    DETECTOR_GEOMETRY.getBoundingBox( hit ) ,
                    0, 0, DETECTOR_GEOMETRY.getRotAngle( hit->getLayerID() , hit->getModuleID() )*180/M_PI,
                    kRed+1, 0, recoilName );
            recoilTrackerHits_->AddElement(recoilHit);

            iter++;
        }

        hits_->AddElement(recoilTrackerHits_);
    }

    void EventObjects::drawECALClusters(TClonesArray* clusters) {

        TEveRGBAPalette* palette = new TEveRGBAPalette(0,4000.0);

        int iC = 0;
        EcalCluster* cluster;
        for (TIter next(clusters); cluster = (ldmx::EcalCluster*)next();) {

            TString clusterName;
            clusterName.Form("ECAL Cluster %d", iC);

            TEveElement* ecalCluster = new TEveElementList(clusterName);

            double energy = cluster->getEnergy();
            std::vector<unsigned int> clusterHitIDs = cluster->getHitIDs();

            int numHits = clusterHitIDs.size();

            for (int iHit = 0; iHit < numHits; iHit++) {
                unsigned int cellID = clusterHitIDs[iHit]>>15;
                unsigned int moduleID = (clusterHitIDs[iHit]<<17)>>29;
                int layer = (clusterHitIDs[iHit]<<20)>>24;

                const UChar_t* rgb = palette->ColorFromValue(energy);
                TColor* aColor = new TColor();
                Int_t color = aColor->GetColor((Int_t)rgb[0], (Int_t)rgb[1], (Int_t)rgb[2]);
    
                TEveGeoShape* ecalDigiHit = drawer_->drawHexPrism(
                        DETECTOR_GEOMETRY.getHexPrism( cellID , moduleID , layer ),
                        0, 0, 0, 
                        color, 0, "RecHit");
                ecalCluster->AddElement(ecalDigiHit);

                if (numHits < 2) { 
                    ecalCluster->SetPickableRecursively(0);
                } else {
                    ecalCluster->SetPickableRecursively(1);
                }
            }
            ecalClusters_->AddElement(ecalCluster);
            iC++;
        }

        ecalClusters_->SetPickable(1);
        recoObjs_->AddElement(ecalClusters_);
    }

    
    void EventObjects::drawECALSimParticles(TClonesArray* ecalSimParticles) {

        ldmx::SimTrackerHit* ecalSPP;
        std::vector<SimTrackerHit*> simVec;
        std::vector<SimTrackerHit*> filteredSimVec;
        for (TIter next(ecalSimParticles); ecalSPP = (ldmx::SimTrackerHit*)next();) {
            simVec.push_back(ecalSPP);
        }

        std::sort(simVec.begin(), simVec.end(), compSims);
           
        SimParticle* lastP = 0; // sometimes multiple SP hits from same particle
        for (int j = 0; j < simVec.size(); j++) {
            SimParticle* sP = simVec[j]->getSimParticle();
            if (sP == lastP) continue;
            lastP = sP;
            filteredSimVec.push_back(simVec[j]);
        }

        std::sort(filteredSimVec.begin(), filteredSimVec.end(), compSimsP);

        for (int j = 0; j < filteredSimVec.size(); j++) {

            SimParticle* sP = filteredSimVec[j]->getSimParticle();

            std::vector<double> pVec = filteredSimVec[j]->getMomentum();
            std::vector<float> rVec = filteredSimVec[j]->getPosition();
            double p = pow(pow(pVec[0],2)+pow(pVec[1],2)+pow(pVec[2],2),0.5);

            double E = sP->getEnergy();

            std::vector<double> simStart = sP->getVertex();
            std::vector<double> simEnd = sP->getEndPoint();
            double rCheck = pow(pow(simEnd[0],2)+pow(simEnd[1],2)+pow(simEnd[2],2),0.5);

            double scale = 1;
            double largest = 0;
            if (abs(simEnd[0]) > 3500.0) {
                 scale = 500.0/abs(simEnd[0]-simStart[0]);
                 largest = simEnd[0];
            }
            if (abs(simEnd[1]) > 3500.0 && abs(simEnd[1]) > largest) {
                 scale = 500.0/abs(simEnd[1]-simStart[1]);
                 largest = simEnd[1];
            }
            if (abs(simEnd[2]) > 3500.0 && abs(simEnd[2]) > 3500) {
                 scale = 500.0/abs(simEnd[2]-simStart[2]);
            }

            double r = pow(pow(scale*(simEnd[0]-simStart[0]),2) + pow(scale*(simEnd[1]-simStart[1]),2) + pow(scale*(simEnd[2]-simStart[2]),2),0.5);
            signed int pdgID = sP->getPdgID();

            TEveArrow* simArr = new TEveArrow(scale*(simEnd[0]-simStart[0]),scale*(simEnd[1]-simStart[1]),scale*(simEnd[2]-simStart[2]),simStart[0],simStart[1],simStart[2]);

            simArr->SetSourceObject(sP);
            simArr->SetMainColor(kBlack);
            simArr->SetTubeR(60*0.02/r);
            simArr->SetConeL(100*0.02/r);
            simArr->SetConeR(150*0.02/r);
            simArr->SetPickable(kTRUE);
            if (p < simThresh_) { simArr->SetRnrSelf(kFALSE); }

            TString name;
            name.Form("PDG = %d, p = %1.5g MeV/c", pdgID, p);
            simArr->SetElementName(name);
            ecalSimParticles_->AddElement(simArr);
        }

        hits_->AddElement(ecalSimParticles_);
    }
}
