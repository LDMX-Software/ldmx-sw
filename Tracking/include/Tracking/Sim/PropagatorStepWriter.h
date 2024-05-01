#ifndef TRACKING_SIM_PROPAGATORSTEPWRITER_H
#define TRACKING_SIM_PROPAGATORSTEPWRITER_H

/* This class is a strip down version of the ActsExample::PropagatorStepWriter
 * It's used to dump in a root file all the steps information of the Acts::Propagator 
 * for a complete validation of a tracking geometry. 
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

class PropagatorStepWriter{
 public:
  struct Config {
    //std::string collection =
    //    "propagation_steps";            ///< particle collection to write
        
    std::string filePath = "";          ///< path of the output file
    std::string fileMode = "RECREATE";  ///< file access mode
    std::string treeName = "propagation_steps";  ///< name of the output tree
    TFile* rootFile = nullptr;                   ///< common root file
  };
      
  /// Constructor with
  /// @param cfg configuration struct
  /// @param output logging level
  PropagatorStepWriter(const Config& cfg);
      
  ~PropagatorStepWriter();
  
  bool WriteSteps(framework::Event &event,
                  const std::vector<PropagationSteps>& stepCollection,
                  const std::vector<ldmx::Measurement>& measurements,
                  const Acts::Vector3 & start_pos, const Acts::Vector3& start_mom);
                                 
 protected:
      
  Config m_cfg;                    ///< the configuration object
  std::mutex m_writeMutex;         ///< protect multi-threaded writes
  TFile* m_outputFile;             ///< the output file name
  TTree* m_outputTree;             ///< the output tree
  int m_eventNr;                   ///< the event number of
      
  std::vector<int> m_volumeID;     ///< volume identifier
  std::vector<int> m_boundaryID;   ///< boundary identifier
  std::vector<int> m_layerID;      ///< layer identifier if
  std::vector<int> m_approachID;   ///< surface identifier
  std::vector<int> m_sensitiveID;  ///< surface identifier
  std::vector<float> m_x;          ///< global x
  std::vector<float> m_y;          ///< global y
  std::vector<float> m_z;          ///< global z
  std::vector<float> m_dx;         ///< global direction x
  std::vector<float> m_dy;         ///< global direction y
  std::vector<float> m_dz;         ///< global direction z
  std::vector<int> m_step_type;    ///< step type
  std::vector<float> m_step_acc;   ///< accuracy
  std::vector<float> m_step_act;   ///< actor check
  std::vector<float> m_step_abt;   ///< aborter
  std::vector<float> m_step_usr;   ///< user
  std::vector<float> m_hit_x;      ///< hit location X
  std::vector<float> m_hit_y;      ///< hit location Y
  std::vector<float> m_hit_z;      ///< hit location Z
  std::vector<float> m_start_pos;  ///< start position of the particle propagated
  std::vector<float> m_start_mom;  ///< start momentum of the particle propagated
        
};
}
}


#endif
