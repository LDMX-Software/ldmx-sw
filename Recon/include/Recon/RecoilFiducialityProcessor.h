/**
 * @file RecoilFiducialityProcessor.h
 * @brief Class that flags events with a fiducial recoil electron.
 * @author Elizabeth Berzin, Stanford University
 */

#ifndef RECON_RECOILFIDUCIALITYPROCESSOR_H_
#define RECON_RECOILFIDUCIALITYPROCESSOR_H_

// LDMX
#include "Ecal/Event/EcalHit.h"
#include "Event/FiducialFlag.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "Tools/AnalysisUtils.h"

namespace recon {

/**
 * @class RecoilFiducialityProcessor
 * @brief Flags events with a fiducial recoil electron, based on truth
 * information.
 *
 * @note
 */
class RecoilFiducialityProcessor : public framework::Producer {
 public:
  /**
   * Class constructor.
   */
  RecoilFiducialityProcessor(const std::string& name,
                             framework::Process& process)
      : framework::Producer(name, process) {}

  /**
   * Class destructor.
   */
  virtual ~RecoilFiducialityProcessor() = default;

  /**
   * Configure the processor using the given user specified parameters.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters& parameters) override;

  /**
   * Create a FiducialFlag object to contain info about
   * whether the recoil electron satisfies certain
   * fiduciality conditions.
   * @param event The event to run the fiduciality check on.
   */
  void produce(framework::Event& event) override;

 private:
  /** Minimum recoil electron momentum at production. */
  double min_p_mag_;

  /** Minimum number of recoil electron hits in the recoil tracker. */
  int min_tracker_hits_;

  /** The name of the ecal collection. */
  std::string ecal_collection_;

  /** The name of the hcal collection. */
  std::string hcal_collection_;

  /** The name of the recoil tracker collection. */
  std::string recoil_collection_;

  /** The name of the output collection. */
  std::string output_collection_;

  /** Inverse option for skimming. */
  bool inverse_skim_{false};
};

}  // namespace recon

#endif
