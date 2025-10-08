#ifndef TRACKING_SIM_PROPAGATORSTEPWRITER_H
#define TRACKING_SIM_PROPAGATORSTEPWRITER_H

/* This class is a strip down version of the ActsExample::PropagatorStepWriter
 * It's used to dump in a root file all the steps information of the
 * Acts::Propagator for a complete validation of a tracking geometry.
 */

#include <mutex>

//--- Framework ---//
#include "Framework/Event.h"

//--- ACTS ---//
#include "Acts/Propagator/detail/SteppingLogger.hpp"

//--- Tracking ---//
#include "Tracking/Event/Measurement.h"

//--- ROOT ---//
#include "TFile.h"
#include "TTree.h"

using PropagationSteps = std::vector<Acts::detail::Step>;

namespace tracking {
namespace sim {

class PropagatorStepWriter {
 public:
  struct Config {
    // std::string collection =
    //     "propagation_steps";            ///< particle collection to write

    std::string file_path_ = "";                   ///< path of the output file
    std::string file_mode_ = "RECREATE";           ///< file access mode
    std::string tree_name_ = "propagation_steps";  ///< name of the output tree
    TFile* root_file_ = nullptr;                   ///< common root file
  };

  /// Constructor with
  /// @param cfg configuration struct
  /// @param output logging level
  PropagatorStepWriter(const Config& cfg);

  ~PropagatorStepWriter();

  bool writeSteps(framework::Event& event,
                  const std::vector<PropagationSteps>& stepCollection,
                  const std::vector<ldmx::Measurement>& measurements,
                  const Acts::Vector3& start_pos,
                  const Acts::Vector3& start_mom);

 protected:
  Config m_cfg_;              ///< the configuration object
  std::mutex m_write_mutex_;  ///< protect multi-threaded writes
  TFile* m_output_file_;      ///< the output file name
  TTree* m_output_tree_;      ///< the output tree
  int m_event_nr_;            ///< the event number of

  //  std::vector<int> m_volumeID;     ///< volume identifier
  std::vector<int> m_boundary_id_;   ///< boundary identifier
  std::vector<int> m_layer_id_;      ///< layer identifier if
  std::vector<int> m_approach_id_;   ///< surface identifier
  std::vector<int> m_sensitive_id_;  ///< surface identifier
  std::vector<float> m_x_;           ///< global x_
  std::vector<float> m_y_;           ///< global y_
  std::vector<float> m_z_;           ///< global z_
  std::vector<float> m_dx_;          ///< global direction x_
  std::vector<float> m_dy_;          ///< global direction y_
  std::vector<float> m_dz_;          ///< global direction z_
  std::vector<int> m_step_type_;     ///< step type
  std::vector<float> m_step_acc_;    ///< accuracy
  std::vector<float> m_step_act_;    ///< actor check
  std::vector<float> m_step_abt_;    ///< aborter
  std::vector<float> m_step_usr_;    ///< user
  std::vector<float> m_hit_x_;       ///< hit location X
  std::vector<float> m_hit_y_;       ///< hit location Y
  std::vector<float> m_hit_z_;       ///< hit location Z
  std::vector<float>
      m_start_pos_;  ///< start position of the particle propagated
  std::vector<float>
      m_start_mom_;  ///< start momentum of the particle propagated
};
}  // namespace sim
}  // namespace tracking

#endif
