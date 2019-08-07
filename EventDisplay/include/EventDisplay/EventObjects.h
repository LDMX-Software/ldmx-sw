#ifndef EVENTDISPLAY_EVENTOBJECTS_H_
#define EVENTDISPLAY_EVENTOBJECTS_H_

#include "DetDescr/EcalHexReadout.h"
#include "DetDescr/HcalID.h"
#include "DetDescr/DetectorGeometry.h"
#include "Event/EcalHit.h"
#include "Event/HcalHit.h"
#include "Event/SimTrackerHit.h"
#include "Event/EcalCluster.h"
#include "Event/SimParticle.h"

#include "EventDisplay/EveDetectorGeometry.h"
#include "EventDisplay/EveShapeDrawer.h"

#include "TEveRGBAPalette.h"
#include "TEveArrow.h"
#include "TEveBox.h"
#include "TClonesArray.h"
#include "TColor.h"
#include "TRandom.h"

namespace ldmx {

    class EventObjects {

        public:

            EventObjects();

            ~EventObjects() {

                delete ecalHits_;
                delete hcalHits_;
                delete recoilTrackerHits_;
                delete ecalClusters_;
                delete ecalSimParticles_;

                delete hits_;
                delete recoObjs_;

                delete hexReadout_;
                delete drawer_;

            }

            void Initialize();

            void drawECALHits(TClonesArray* hits);
            
            void drawHCALHits(TClonesArray* hits);

            void drawRecoilHits(TClonesArray* hits);

            void drawECALClusters(TClonesArray* clusters);

            void drawECALSimParticles(TClonesArray* ecalSimParticles);

            void SetSimThresh(double simThresh);

            void ColorClusters();

            TEveElement* getECALHits() { return ecalHits_; }

            TEveElement* getHCALHits() { return hcalHits_; }

            TEveElement* getRecoilHits() { return recoilTrackerHits_; }

            TEveElement* getECALClusters() { return ecalClusters_; }

            TEveElement* getECALSimParticles() { return ecalSimParticles_; }

            TEveElement* getHitCollections() { return hits_; }

            TEveElement* getRecoObjects() { return recoObjs_; }

        private:

            TEveElement* ecalHits_;
            TEveElement* hcalHits_;
            TEveElement* recoilTrackerHits_;
            TEveElement* ecalClusters_;
            TEveElement* ecalSimParticles_;

            TEveElement* hits_;
            TEveElement* recoObjs_;

            EcalHexReadout* hexReadout_;
            EveShapeDrawer* drawer_;

            double simThresh_ = 0;
            TRandom r_;
            std::vector<Color_t> colors_ = {kRed, kBlue, kGreen, kYellow, kMagenta, kBlack, kOrange, kPink};

    };
}

#endif
