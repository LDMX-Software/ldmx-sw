/**
 * @file RecoilMissesEcalSkimmer.h
 * @brief Processor used to select events where the recoil electron misses the
 *        Ecal.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef RECON_SKIMS_RECOILMISSESECALSKIMMER_H_
#define RECON_SKIMS_RECOILMISSESECALSKIMMER_H_

//----------//
//   LDMX   //
//----------//
#include "Framework/EventProcessor.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "SimCore/Event/SimParticle.h"
#include "Tools/AnalysisUtils.h"

namespace recon {

class RecoilMissesEcalSkimmer : public framework::Producer {
 public:
  /** Constructor */
  RecoilMissesEcalSkimmer(const std::string &name, framework::Process &process);

  /** Destructor */
  ~RecoilMissesEcalSkimmer();

  /**
   * Run the processor and select events where the recoil misses the
   * Ecal.
   *
   * @param event The event to process.
   */
  void produce(framework::Event &event);

};  // RecoilMissesEcalSkimmer
}  // namespace recon

#endif  // RECON_SKIMS_RECOILMISSESECALSKIMMER_H_
