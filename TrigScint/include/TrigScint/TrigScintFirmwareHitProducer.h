/**
 * @file TrigScintFirmwareHitProducer.h
 * @brief Staging of Real Hits
 * @author Lene Kristian Bryngemark, Stanford University
 */

#ifndef TRIGSCINT_TRIGSCINTFIRMWAREHITPRODUCER_H
#define TRIGSCINT_TRIGSCINTFIRMWAREHITPRODUCER_H

/*~~~~~~~~~~*/
/*   ROOT   */
/*~~~~~~~~~~*/
#include "TRandom3.h"

// LDMX
#include "DetDescr/TrigScintID.h"
#include "Recon/Event/EventConstants.h"
#include "Tools/NoiseGenerator.h"
#include "TrigScint/Event/TrigScintHit.h"
#include "TrigScint/Event/TrigScintQIEDigis.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

/*~~~~~~~~~~~*/
/* TrigScint */
/*~~~~~~~~~~~*/
#include "TrigScint/Firmware/objdef.h"
#include "TrigScint/SimQIE.h"

namespace trigscint {

/**
 * @class TrigScintFirmwareHitProducer
 * @brief
 */
class TrigScintFirmwareHitProducer : public framework::Producer {
 public:
  TrigScintFirmwareHitProducer(const std::string& name,
                               framework::Process& process)
      : Producer(name, process) {}

  void configure(framework::config::Parameters& ps) override;

  void produce(framework::Event& event) override;

  /**
   * add a hit at index idx to a cluster
   */

 private:
  /// Class to set the verbosity level.
  // TODO: Make use of the global verbose parameter.
  bool verbose_{false};

  /// Name of the input collection containing the sim hits
  std::string inputCollection_;

  /// Name of the pass that the input collection is on (empty string means take
  /// any pass)
  std::string inputPassName_;

  /// Name of the output collection that will be used to stored the
  /// digitized trigger scintillator hits
  std::string outputCollection_;

  /// SiPM gain
  double gain_{1e6};

  /// QIE pedestal
  double pedestal_{6.0};

  /// Total MeV per MIP
  double mevPerMip_{1.40};

  /// Total number of photoelectrons per MIP
  double pePerMip_{13.5};

  /// Total number of photoelectrons per MIP
  int sample_of_interest_{2};

  std::string testCollection_;

  bool doTest_{true};
};

}  // namespace trigscint

#endif /* TRIGSCINT_TRIGSCINTFIRMWAREHITPRODUCER_H */
