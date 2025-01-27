/**
 * @file EcalWABRecProcessor.h
 * @brief Class that reconstructs important kinematic variables for WAB studies
 * @author Sanjit Masanam, UCSB
 */

#ifndef EVENTPROC_ECALWABROCESSOR_H_
#define EVENTPROC_ECALWABROCESSOR_H_

// LDMX
#include "DetDescr/EcalGeometry.h"
#include "DetDescr/EcalID.h"
#include "Ecal/Event/EcalHit.h"
#include "Ecal/Event/EcalWABResult.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

#include "Tools/ONNXRuntime.h"

// ROOT (MIP tracking)
#include "TVector3.h"

// C++
#include <map>
#include <memory>

namespace ecal {

class EcalWABRecProcessor : public framework::Producer {
public:
  
  EcalWABRecProcessor(const std::string& name, framework::Process& process)
      : Producer(name, process) {}

  virtual ~EcalWABRecProcessor() {}

  /**
   * Configure the processor using the given user specified parameters.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters& parameters) override;

  void produce(framework::Event& event);
    
private:
  std::string rec_pass_name_;
  std::string rec_coll_name_;
  bool data;
  bool WAB;

  /** Name of the collection which will containt the results. */
  std::string collectionName_{"EcalWABRec"};

  /// handle to current geometry (to share with member functions)
  const ldmx::EcalGeometry* geometry_;

};  // EcalWABRecProcessor

}  // namespace ecal

#endif