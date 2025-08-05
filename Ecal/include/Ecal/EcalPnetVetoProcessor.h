/**
 * @file EcalPnetVetoProcessor.h
 * @brief Class that determines if event is vetoable using ECAL hit information
 * w/ a deep neural network
 * @author Huilin Qu, Tamas Almos Vami (UCSB)
 */

#ifndef EVENTPROC_ECALPNETVETOPROCESSOR_H_
#define EVENTPROC_ECALPNETVETOPROCESSOR_H_

// LDMX
#include <algorithm>
#include <numeric>

#include "DetDescr/EcalGeometry.h"
#include "Ecal/EcalHelper.h"
#include "Ecal/Event/EcalHit.h"
#include "Ecal/Event/EcalVetoResult.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "Tools/ONNXRuntime.h"

namespace ecal {

/**
 * @class EcalPnetVetoProcessor
 * @brief Determines if event is vetoable using ECAL hit information w/ a deep
 * neural network
 */
class EcalPnetVetoProcessor : public framework::Producer {
 public:
  EcalPnetVetoProcessor(const std::string& name, framework::Process& process);
  virtual ~EcalPnetVetoProcessor() = default;
  void configure(framework::config::Parameters& parameters) override;
  void produce(framework::Event& event) override;

 private:
  /**
   * Make inputs to the DNN from ECAL RecHits.
   * @param ecalRecHits The EcalHit collection.
   */
  void makeInputs(const ldmx::EcalGeometry& geom,
                  const std::vector<ldmx::EcalHit>& ecalRecHits,
                  std::array<double, 3> etraj, std::array<double, 3> enorm);

  /**
   * Transform logits to a probability
   * @param logits: Vector of logits
   * @returns Vector of log softmax probabilities
   */
  std::vector<float> logSoftmax(const std::vector<float>& logits);

 private:
  /** Maximum number of hits allowed in ECAL. Events with more hits will be
   * marked as BKG directly without running ParticleNet. */
  constexpr static unsigned int MAX_NUM_HITS = 300;

  constexpr static unsigned int N_COORDINATE_DIM = 3;
  constexpr static unsigned int COORDINATE_X_OFFSET = 0;
  constexpr static unsigned int COORDINATE_Y_OFFSET = MAX_NUM_HITS;
  constexpr static unsigned int COORDINATE_Z_OFFSET = 2 * MAX_NUM_HITS;

  constexpr static unsigned int N_FEATURE_DIM = 5;
  constexpr static unsigned int FEATURE_X_OFFSET = 0;
  constexpr static unsigned int FEATURE_Y_OFFSET = MAX_NUM_HITS;
  constexpr static unsigned int FEATURE_Z_OFFSET = 2 * MAX_NUM_HITS;
  constexpr static unsigned int FEATURE_ENERGY_OFFSET = 3 * MAX_NUM_HITS;
  constexpr static unsigned int FEATURE_LAYERID_OFFSET = 4 * MAX_NUM_HITS;

  const static std::vector<std::string> INPUT_NAMES;
  const static std::vector<unsigned int> INPUT_SIZES;

  float disc_cut_ = -99;
  std::vector<std::vector<float>> data_;
  std::unique_ptr<ldmx::Ort::ONNXRuntime> rt_;

  /** Name of the collection which will containt the results. */
  std::string collection_name_{"EcalPnetVeto"};

  std::string rec_coll_name_;
  std::string ecal_rec_hits_passname_;
  std::string ecal_sp_hits_passname_;
  std::string track_pass_name_;
  std::string track_collection_;
  bool recoil_from_tracking_;
};

}  // namespace ecal

#endif
