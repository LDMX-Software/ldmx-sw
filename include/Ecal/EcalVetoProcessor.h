/**
 * @file EcalVetoProcessor.h
 * @brief Class that determines if event is vetoable using ECAL hit information
 * @author Owen Colegrove, UCSB
 */

#ifndef EVENTPROC_ECALVETOPROCESSOR_H_
#define EVENTPROC_ECALVETOPROCESSOR_H_

// LDMX
#include "DetDescr/EcalHexReadout.h"
#include "DetDescr/EcalID.h"
#include "Ecal/Event/EcalHit.h"
#include "Ecal/Event/EcalVetoResult.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

#include "Tools/ONNXRuntime.h"

// ROOT (MIP tracking)
#include "TVector3.h"

// C++
#include <map>
#include <memory>

namespace ecal {

/**
 * @class EcalVetoProcessor
 * @brief Determines if event is vetoable using ECAL hit information
 */
class EcalVetoProcessor : public framework::Producer {
 public:
  typedef std::pair<ldmx::EcalID, float> CellEnergyPair;

  typedef std::pair<float, float> XYCoords;

  EcalVetoProcessor(const std::string& name, framework::Process& process)
      : Producer(name, process) {}

  virtual ~EcalVetoProcessor() {}

  /**
   * Configure the processor using the given user specified parameters.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters& parameters) final override;

  void produce(framework::Event& event);

  // MIP tracking:  Class for storing hit information for tracking in a convenient way
  struct HitData {
    int layer;
    TVector3 pos;
  };

 private:
  /** Wrappers for ecalHexReadout functions. See hitToPair().
   *  Necessary to easily combine cellID with moduleID to get unique ID of
   *  hit in layer. In future: combine celID+moduleID+layerID.
   */
  bool isInShowerInnerRing(ldmx::EcalID centroidID, ldmx::EcalID probeID) {
    return hexReadout_->isNN(centroidID, probeID);
  }
  bool isInShowerOuterRing(ldmx::EcalID centroidID, ldmx::EcalID probeID) {
    return hexReadout_->isNNN(centroidID, probeID);
  }
  std::pair<double, double> getCellCentroidXYPair(ldmx::EcalID centroidID) {
    return hexReadout_->getCellCenterAbsolute(centroidID);
  }
  std::vector<ldmx::EcalID> getInnerRingCellIds(ldmx::EcalID id) {
    return hexReadout_->getNN(id);
  }
  std::vector<ldmx::EcalID> getOuterRingCellIds(ldmx::EcalID id) {
    return hexReadout_->getNNN(id);
  }

  void clearProcessor();

  ldmx::EcalID hitID(const ldmx::EcalHit& hit) const {
    return ldmx::EcalID(hit.getID());
  }

  /* Function to calculate the energy weighted shower centroid */
  ldmx::EcalID GetShowerCentroidIDAndRMS(
      const std::vector<ldmx::EcalHit>& ecalRecHits, double& showerRMS);

  /* Function to load up empty vector of hit maps */
  void fillHitMap(const std::vector<ldmx::EcalHit>& ecalRecHits,
                  std::map<ldmx::EcalID, float>& cellMap_);

  /* Function to take loaded hit maps and find isolated hits in them */
  void fillIsolatedHitMap(const std::vector<ldmx::EcalHit>& ecalRecHits,
                          ldmx::EcalID globalCentroid,
                          std::map<ldmx::EcalID, float>& cellMap_,
                          std::map<ldmx::EcalID, float>& cellMapIso_,
                          bool doTight = false);

  std::vector<XYCoords> getTrajectory(std::vector<double> momentum,
                                      std::vector<float> position);

  void buildBDTFeatureVector(const ldmx::EcalVetoResult& result);

  // MIP tracking
  /* Function to find distance between two lines (line 1 passing through v1 and v2, etc.) */
  float distTwoLines(TVector3 v1, TVector3 v2, TVector3 w1, TVector3 w2);
  /* Function to find the minimum point-line distance */
  float distPtToLine(TVector3 h1, TVector3 p1, TVector3 p2);

 private:
  std::map<ldmx::EcalID, float> cellMap_;
  std::map<ldmx::EcalID, float> cellMapTightIso_;

  std::vector<float> ecalLayerEdepRaw_;
  std::vector<float> ecalLayerEdepReadout_;
  std::vector<float> ecalLayerTime_;
  std::vector<float> mapsx;
  std::vector<float> mapsy;

  int nEcalLayers_{0};
  int backEcalStartingLayer_{0};
  int nReadoutHits_{0};
  int deepestLayerHit_{0};
  int doBdt_{0};

  double summedDet_{0};
  double summedTightIso_{0};
  double maxCellDep_{0};
  double showerRMS_{0};
  double xStd_{0};
  double yStd_{0};
  double avgLayerHit_{0};
  double stdLayerHit_{0};
  double ecalBackEnergy_{0};
  // MIP tracking
  int nStraightTracks_{0};
  int nLinregTracks_{0};
  int firstNearPhLayer_{0};
  float epAng_{0};
  float epSep_{0};

  double bdtCutVal_{0};

  bool verbose_{false};
  bool doesPassVeto_{false};

  const ldmx::EcalHexReadout* hexReadout_;

  std::string bdtFileName_;
  std::string cellFileNamexy_;
  std::vector<float> bdtFeatures_;

  std::string rec_pass_name_;
  std::string rec_coll_name_;

  /** Name of the collection which will containt the results. */
  std::string collectionName_{"EcalVeto"};

  std::unique_ptr<ldmx::Ort::ONNXRuntime> rt_;
};

}  // namespace ecal

#endif
