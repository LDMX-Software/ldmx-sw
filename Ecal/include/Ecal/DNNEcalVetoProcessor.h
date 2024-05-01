/**
 * @file DNNDNNEcalVetoProcessor.h
 * @brief Class that determines if event is vetoable using ECAL hit information
 * w/ a deep neural network
 * @author Huilin Qu, UCSB
 */

#ifndef EVENTPROC_DNNECALVETOPROCESSOR_H_
#define EVENTPROC_DNNECALVETOPROCESSOR_H_

// LDMX
#include "Ecal/Event/EcalHit.h"
#include "Ecal/Event/EcalVetoResult.h"
#include "DetDescr/EcalGeometry.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

#include "Tools/ONNXRuntime.h"

namespace ecal {

/**
 * @class DNNEcalVetoProcessor
 * @brief Determines if event is vetoable using ECAL hit information w/ a deep
 * neural network
 */
class DNNEcalVetoProcessor : public framework::Producer {
 public:
  DNNEcalVetoProcessor(const std::string& name, framework::Process& process);
  virtual ~DNNEcalVetoProcessor() {}
  void configure(framework::config::Parameters& parameters) final override;
  void produce(framework::Event& event);

 private:
  /**
   * Make inputs to the DNN from ECAL RecHits.
   * @param ecalRecHits The EcalHit collection.
   */
  void make_inputs(const ldmx::EcalGeometry& geom,
                   const std::vector<ldmx::EcalHit>& ecalRecHits);

 private:
  /** Maximum number of hits allowed in ECAL. Events with more hits will be
   * marked as BKG directly without running the DNN. */
  constexpr static unsigned int max_num_hits_ = 50;

  constexpr static unsigned int n_coordinate_dim_ = 3;
  constexpr static unsigned int coordinate_x_offset_ = 0;
  constexpr static unsigned int coordinate_y_offset_ = max_num_hits_;
  constexpr static unsigned int coordinate_z_offset_ = 2 * max_num_hits_;

  constexpr static unsigned int n_feature_dim_ = 5;
  constexpr static unsigned int feature_x_offset_ = 0;
  constexpr static unsigned int feature_y_offset_ = max_num_hits_;
  constexpr static unsigned int feature_z_offset_ = 2 * max_num_hits_;
  constexpr static unsigned int feature_layerid_offset_ = 3 * max_num_hits_;
  constexpr static unsigned int feature_energy_offset_ = 4 * max_num_hits_;

  const static std::vector<std::string> input_names_;
  const static std::vector<unsigned int> input_sizes_;

  float disc_cut_ = -99;
  std::vector<std::vector<float>> data_;
  std::unique_ptr<ldmx::Ort::ONNXRuntime> rt_;

  /** Name of the collection which will containt the results. */
  std::string collectionName_{"DNNEcalVeto"};

  bool debug_ = false;
};

}  // namespace ecal

#endif
