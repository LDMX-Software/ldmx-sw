#include "EventDisplay/EventObjects.h"   

namespace ldmx {

    EventObjects::EventObjects() {
        Initialize();
    }

    void EventObjects::Initialize() {
        
        //collections of event objects
        ecalHits_           = new TEveElementList("ECAL RecHits");
        hcalHits_           = new TEveElementList("HCAL RecHits");
        recoilTrackerHits_  = new TEveElementList("Recoil Sim Hits");
        ecalClusters_       = new TEveElementList("ECAL Clusters");
        ecalSimParticles_   = new TEveElementList("ECAL SP Sim Particles");

        //packages of event objects to be passed to event display manager
        hits_               = new TEveElementList("Reco Hits");
        recoObjs_           = new TEveElementList("Reco Objects");

    }

    static bool compEcalHits(const EcalHit &a, const EcalHit &b) {
        return a.getEnergy() > b.getEnergy();
    }

    static bool compHcalHits(const HcalHit &a, const HcalHit &b) {
        return a.getPE() > b.getPE();
    }

    static bool compScorePlaneHits(const SimTrackerHit &a, const SimTrackerHit &b) {
        return (a.getTrackID() < b.getTrackID());
    }

    static bool areScorePlaneHitsEqual(const SimTrackerHit &a, const SimTrackerHit &b) {
        return (a.getTrackID() == b.getTrackID());
    }

    void EventObjects::SetSimThresh(double simThresh) {

        simThresh_ = simThresh;
        TEveElement* spHits = 0;
        spHits = hits_->FindChild("ECAL SP Sim Particles");
        TEveElement::List_i sim;

        for (sim = spHits->BeginChildren(); sim != spHits->EndChildren(); sim++) {
            
            TEveElement* el = *sim;
            SimTrackerHit* sp = (ldmx::SimTrackerHit*)el->GetSourceObject();
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
            std::cout << "[ EventObjects ] : No clusters to color!" << std::endl;
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
                std::cout << "[ EventObjects ] : Using random colors to fill in extra clusters." << std::endl;
            }

            for (hit = el->BeginChildren(); hit != el->EndChildren(); hit++) { 
                TEveElement* elChild = *hit;
                elChild->SetMainColor(color);
            }
        }
    }

    void EventObjects::drawECALHits(std::vector<EcalHit> hits) {

        TEveRGBAPalette* palette = new TEveRGBAPalette(0,500.0);

        std::sort(hits.begin(), hits.end(), compEcalHits);

        for ( const EcalHit &hit : hits ) {

            double energy = hit.getEnergy();

            if (energy == 0) { continue; }

            TString digiName;
            digiName.Form("%1.5g MeV", energy);

            const UChar_t* rgb = palette->ColorFromValue(energy);
            TColor* aColor = new TColor();
            Int_t color = aColor->GetColor((Int_t)rgb[0], (Int_t)rgb[1], (Int_t)rgb[2]);

            TEveGeoShape* ecalDigiHit = EveShapeDrawer::getInstance().drawHexPrism(
                    DetectorGeometry::getInstance().getHexPrism( EcalID(hit.getID()) ),
                    0, 0, 0, 
                    color, 0, digiName);

            ecalHits_->AddElement(ecalDigiHit);
        }

        ecalHits_->SetPickableRecursively(1);

        hits_->AddElement(ecalHits_);
    }

    void EventObjects::drawHCALHits(std::vector<HcalHit> hits) {


        TEveRGBAPalette* palette = new TEveRGBAPalette(0,100.0);

        std::sort(hits.begin(), hits.end(), compHcalHits);

        for ( const HcalHit &hit : hits ) {
            int pe = hit.getPE();
            if (pe == 0) { continue; }

            HcalID id(hit.getID());

            const UChar_t* rgb = palette->ColorFromValue(pe);
            TColor* aColor = new TColor();
            Int_t color = aColor->GetColor((Int_t)rgb[0], (Int_t)rgb[1], (Int_t)rgb[2]);

            TString digiName;
            digiName.Form("%d PEs, Section %d, Layer %d, Bar %d, Z %1.5g", pe, 
                    id.section(), id.layer(), id.strip(), hit.getZPos());

            BoundingBox hcal_hit_bb = DetectorGeometry::getInstance().getBoundingBox( hit );
            TEveGeoShape *hcalDigiHit = EveShapeDrawer::getInstance().drawRectPrism(
                    hcal_hit_bb ,
                    0, 0, 0, color, 0, digiName );

            if ( hcalDigiHit ) {
                if ( hit.isNoise() ) { hcalDigiHit->SetRnrSelf(0); }
                hcalHits_->AddElement(hcalDigiHit);
            } // successfully created hcal digi hit

        } //loop through sorted hit list

        hcalHits_->SetPickableRecursively(1);
        hits_->AddElement(hcalHits_);
    }

    void EventObjects::drawRecoilHits(std::vector<SimTrackerHit> hits) {

        int iter = 0;
        for ( const SimTrackerHit &hit : hits ) {

            std::vector<float> xyzPos = hit.getPosition();
            double energy = hit.getEdep();

            TString recoilName;
            recoilName.Form("Recoil Hit %d", iter);

            TEveGeoShape *recoilHit = EveShapeDrawer::getInstance().drawRectPrism(
                    DetectorGeometry::getInstance().getBoundingBox( hit ) ,
                    0, 0, DetectorGeometry::getInstance().getRotAngle( hit.getLayerID() , hit.getModuleID() )*180/M_PI,
                    kRed+1, 0, recoilName );
            recoilTrackerHits_->AddElement(recoilHit);

            iter++;
        }

        hits_->AddElement(recoilTrackerHits_);
    }

    void EventObjects::drawECALClusters(std::vector<EcalCluster> clusters) {

        TEveRGBAPalette* palette = new TEveRGBAPalette(0,4000.0);

        int iC = 0;
        for (const EcalCluster &cluster : clusters ) {

            TString clusterName;
            clusterName.Form("ECAL Cluster %d", iC);

            TEveElement* ecalCluster = new TEveElementList(clusterName);

            double energy = cluster.getEnergy();
            std::vector<unsigned int> clusterHitIDs = cluster.getHitIDs();

            int numHits = clusterHitIDs.size();

            for (int iHit = 0; iHit < numHits; iHit++) {

                EcalID id(clusterHitIDs.at(iHit));

                const UChar_t* rgb = palette->ColorFromValue(energy);
                TColor* aColor = new TColor();
                Int_t color = aColor->GetColor((Int_t)rgb[0], (Int_t)rgb[1], (Int_t)rgb[2]);
    
                TEveGeoShape* ecalDigiHit = EveShapeDrawer::getInstance().drawHexPrism(
                        DetectorGeometry::getInstance().getHexPrism( id ),
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
    
    void EventObjects::drawECALSimParticles(std::vector<SimTrackerHit> ecalSimParticles) {

        std::sort(ecalSimParticles.begin(), ecalSimParticles.end(), compScorePlaneHits );
           
        auto lastUniqueEntry = std::unique( ecalSimParticles.begin() , ecalSimParticles.end() , areScorePlaneHitsEqual );

        ecalSimParticles.erase( lastUniqueEntry , ecalSimParticles.end() );

        for ( const SimTrackerHit &spHit : ecalSimParticles ) {

            std::vector<double> pVec = spHit.getMomentum();
            double p = pow(pow(pVec[0],2)+pow(pVec[1],2)+pow(pVec[2],2),0.5);

            double E = spHit.getEnergy();

            std::vector<float> simStart = spHit.getPosition();
            std::vector<double> simDir   = pVec;
            double rCheck = pow(pow(simDir[0],2)+pow(simDir[1],2)+pow(simDir[2],2),0.5);

            double scale = 1;
            double largest = 0;
            if (abs(simDir[0]) > 3500.0) {
                 scale = 500.0/abs(simDir[0]);
                 largest = simDir[0];
            }
            if (abs(simDir[1]) > 3500.0 && abs(simDir[1]) > largest) {
                 scale = 500.0/abs(simDir[1]);
                 largest = simDir[1];
            }
            if (abs(simDir[2]) > 3500.0 && abs(simDir[2]) > 3500) {
                 scale = 500.0/abs(simDir[2]);
            }

            double r = pow(pow(scale*(simDir[0]),2) + pow(scale*(simDir[1]),2) + pow(scale*(simDir[2]),2),0.5);
            signed int pdgID = spHit.getPdgID();

            TEveArrow* simArr = new TEveArrow(scale*simDir[0],scale*simDir[1],scale*simDir[2],simStart[0],simStart[1],simStart[2]);

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
