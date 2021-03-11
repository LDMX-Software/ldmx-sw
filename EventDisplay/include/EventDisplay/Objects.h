/**
 * @file Objects.h
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
#include "SimCore/Event/SimCalorimeterHit.h"

#include "EventDisplay/EveDetectorGeometry.h"
#include "EventDisplay/EveShapeDrawer.h"

#include "TColor.h"
#include "TEveArrow.h"
#include "TEveBox.h"
#include "TEveRGBAPalette.h"
#include "TRandom.h"

namespace eventdisplay {

/**
 * @class Objects
 * @brief Drawing methods for event objects.
 *
 * Both ECAL and HCAL hits are colored by their relative energy/pe deposits.
 */
class Objects {
 public:
  /**
   * Constructor
   * Defines new necessary objects.
   * @sa Initialize
   */
  Objects();

  /**
   * Destructor
   * Deletes objects that have been constructed.
   */
  ~Objects() {
    delete sim_objects_;
    delete rec_objects_;
  }

  /**
   * Defines new Eve Element Lists for the event objects.
   */
  void Initialize();

  /**
   * Not implemented
   */
  template <typename T>
  void draw(T o) { EXCEPTION_RAISE("NotImp","Drawing not implemented for the input type."); }

  /**
   * Drawing EcalHit
   */
  void draw(std::vector<ldmx::EcalHit> hits);

  /**
   * Drawing HcalHit
   */
  void draw(std::vector<ldmx::HcalHit> hits);

  /**
   * Drawing EcalCluster
   */
  void draw(std::vector<ldmx::EcalCluster> clusters);

  /**
   * Drawing SimTrackerHit
   */
  void draw(std::vector<ldmx::SimTrackerHit> hits);

  /**
   * Draws the hits in the input collection assuming that they are
   * SimTrackerHits that hit the recoil tracker.
   */
  void draw(std::vector<ldmx::SimCalorimeterHit> hits);

  /**
   * Draws the sim particles
   */
  void draw(std::map<int,ldmx::SimParticle> particles);

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
   * Get the objects from the sim level Eve Element
   * Used to attach these Eve Elements to the Eve Manager.
   */
  TEveElement* getSimObjects() { return sim_objects_; }

  /**
   * Get the objects from the reconstruction level Eve Element
   * Used to attach these Eve Elements to the Eve Manager.
   */
  TEveElement* getRecObjects() { return rec_objects_; }

 private:
  /// Eve Element containing all hits
  TEveElement* sim_objects_;
  /// Eve Element containing reco objects that aren't hits
  TEveElement* rec_objects_;

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
