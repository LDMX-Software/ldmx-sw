/**
 * @file DNNDNNEcalVetoProcessor.h
 * @brief Class that determines if event is vetoable using ECAL hit information w/ a deep neural network
 * @author Huilin Qu, UCSB
 */

#ifndef EVENTPROC_DNNECALVETOPROCESSOR_H_
#define EVENTPROC_DNNECALVETOPROCESSOR_H_

// LDMX
#include "DetDescr/EcalHexReadout.h"
#include "Event/EventDef.h"
#include "Framework/EventProcessor.h"
#include "Framework/Parameters.h"

#ifdef LDMX_USE_ONNXRUNTIME
#include "Tools/ONNXRuntime.h"
#endif

namespace ldmx {

  /**
   * @class DNNEcalVetoProcessor
   * @brief Determines if event is vetoable using ECAL hit information w/ a deep neural network
   */
  class DNNEcalVetoProcessor: public Producer {

  public:
    DNNEcalVetoProcessor(const std::string& name, Process& process);
    virtual ~DNNEcalVetoProcessor() {}
    void configure(Parameters& parameters) final override;
    void produce(Event& event);

  private:
    /**
     * Make inputs to the DNN from ECAL RecHits.
     * @param ecalRecHits The EcalHit collection.
     */
    void make_inputs(const std::vector<EcalHit>& ecalRecHits);

  private:
    /** Maximum number of hits allowed in ECAL. Events with more hits will be marked as BKG directly without running the DNN. */
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
#ifdef LDMX_USE_ONNXRUNTIME
    std::unique_ptr<Ort::ONNXRuntime> rt_;
#endif

    std::unique_ptr<EcalHexReadout> hexReadout_;

    /** Name of the collection which will containt the results. */
    std::string collectionName_{"DNNEcalVeto"};

    bool debug_ = false;

  };

}

#endif
