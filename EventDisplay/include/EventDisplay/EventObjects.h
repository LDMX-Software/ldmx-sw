#ifndef EVENTDISPLAY_EVENTOBJECTS_H_
#define EVENTDISPLAY_EVENTOBJECTS_H_

#include "DetDescr/EcalHexReadout.h"
#include "DetDescr/HcalID.h"
#include "Event/EcalHit.h"
#include "Event/HcalHit.h"
#include "Event/SimTrackerHit.h"
#include "Event/EcalCluster.h"
#include "Event/SimParticle.h"
#include "Tools/HitBox.h"
#include "Tools/HcalDetectorGeometry.h"

#include "EventDisplay/DetectorGeometry.h"
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

            TEveElement* ecalHits_{nullptr};
            TEveElement* hcalHits_{nullptr};
            TEveElement* recoilTrackerHits_{nullptr};
            TEveElement* ecalClusters_{nullptr};
            TEveElement* ecalSimParticles_{nullptr};

            TEveElement* hits_{nullptr};
            TEveElement* recoObjs_{nullptr};

            EcalHexReadout* hexReadout_{nullptr};
            EveShapeDrawer* drawer_{nullptr};

            double simThresh_ = 0;
            TRandom r_;
            std::vector<Color_t> colors_ = {kRed, kBlue, kGreen, kYellow, kMagenta, kBlack, kOrange, kPink};

    };
}

#endif
