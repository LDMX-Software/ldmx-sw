/**
 * @file EventObjects.h
 * @author Josh Hiltbrand, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef EVENTDISPLAY_EVENTOBJECTS_H_
#define EVENTDISPLAY_EVENTOBJECTS_H_

#include "DetDescr/HcalID.h"
#include "Ecal/Event/EcalCluster.h"
#include "Ecal/Event/EcalHit.h"
#include "EventDisplay/DetectorGeometry.h"
#include "Hcal/Event/HcalHit.h"
#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"

#include "EventDisplay/EveDetectorGeometry.h"
#include "EventDisplay/EveShapeDrawer.h"

#include "TColor.h"
#include "TEveArrow.h"
#include "TEveBox.h"
#include "TEveRGBAPalette.h"
#include "TRandom.h"

namespace eventdisplay {

/**
 * @class EventObjects
 * @brief Drawing methods for event objects.
 *
 * Both ECAL and HCAL hits are colored by their relative energy/pe deposits.
 */
class EventObjects {
 public:
  /**
   * Constructor
   * Defines new necessary objects.
   * @sa Initialize
   */
  EventObjects();

  /**
   * Destructor
   * Deletes objects that have been constructed.
   */
  ~EventObjects() {
    delete ecalHits_;
    delete hcalHits_;
    delete recoilTrackerHits_;
    delete ecalClusters_;
    delete ecalSimParticles_;

    delete hits_;
    delete recoObjs_;
  }

  /**
   * Defines new Eve Element Lists for the event objects.
   */
  void Initialize();

  /**
   * Draws the hits in the input collection assuming that they are EcalHits
   */
  void drawECALHits(std::vector<ldmx::EcalHit> hits);

  /**
   * Draws the hits in the input collection assuming that they are HcalHits
   */
  void drawHCALHits(std::vector<ldmx::HcalHit> hits);

  /**
   * Draws the hits in the input collection assuming that they are
   * SimTrackerHits that hit the recoil tracker.
   */
  void drawRecoilHits(std::vector<ldmx::SimTrackerHit> hits);

  /**
   * Draws the hits in the input collection assuming that they are EcalClusters
   */
  void drawECALClusters(std::vector<ldmx::EcalCluster> clusters);

  /**
   * Draws the hits in the input collection assuming that they are
   * SimTrackerHits that hit the Ecal Scoring Planes
   */
  void drawECALSimParticles(std::vector<ldmx::SimTrackerHit> ecalSimParticles);

  /**
   * Sets the energy threshold for a sim particle to be drawn.
   *
   * @param simThresh minimum energy to be included
   */
  void SetSimThresh(double simThresh);

  /**
   * Colors ecal clusters according to colors_
   */
  void ColorClusters();

  /**
   * Get the hits Eve Element
   * Used to attach these Eve Elements to the Eve Manager.
   */
  TEveElement* getHitCollections() { return hits_; }

  /**
   * Get the recoObjs Eve Element
   * Used to attach these Eve Elements to the Eve Manager.
   */
  TEveElement* getRecoObjects() { return recoObjs_; }

 private:
  /// Eve Element containing ecal hits 
  TEveElement* ecalHits_;  
  /// Eve Element containing hcal hits
  TEveElement* hcalHits_;
  /// Eve Element containing recoil tracker hits
  TEveElement* recoilTrackerHits_;
  /// Eve Element containing ecal clusters
  TEveElement* ecalClusters_;
  /// Eve Element containing ecal sim particles
  TEveElement* ecalSimParticles_;

  /// Eve Element containing all hits
  TEveElement* hits_;
  /// Eve Element containing reco objects that aren't hits
  TEveElement* recoObjs_;

  /// threshold for sim particles to be drawn
  double simThresh_ = 0;

  /** list of colors to use with ecal clusters */
  std::vector<Color_t> colors_ = {kRed,     kBlue,  kGreen,  kYellow,
                                  kMagenta, kBlack, kOrange, kPink};

  /// random number generator for colors if we go over the ones in
  TRandom r_;
};
}  // namespace eventdisplay

#endif
