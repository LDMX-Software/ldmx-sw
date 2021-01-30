
#ifndef TRIGSCINT_TRUTHHITPRODUCER_H_
#define TRIGSCINT_TRUTHHITPRODUCER_H_

/***************/
/*   ldmx-sw   */
/***************/

#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Recon/Event/EventConstants.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "TrigScint/Event/TrigScintHit.h"

namespace trigscint {

/**
 * Producer making a collection based on some truth info cuts.
 * Like, skimming out only the beam electrons.
 * This producer creates a new hit collection,
 * that can then be fed to digi etc like anything else.
 */
class TruthHitProducer : public framework::Producer {
 public:
  /**
   * Constructor.
   *
   * @param name Name for this instance of the class.
   * @param process The Process class associated with EventProcessor,
   * provided by the framework.
   */
  TruthHitProducer(const std::string &name, framework::Process &process);

  /// Destructor
  ~TruthHitProducer();

  /**
   * Configure the processor using the given user specified parameters.
   *
   * The user specified parameters that are availabed are defined
   * in the python configuration class. Look at the my_processor.py
   * module of the EventProc python for the python structure.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters &parameters) final override;

  /**
   * Process the event and put new data products into it.
   *
   * @param event The event to process.
   */
  void produce(framework::Event &event);

  /// Class to set the verbosity level.
  // TODO: Make use of the global verbose parameter.
  bool verbose_{false};

  /// Name of the input collection containing the sim hits
  std::string inputCollection_;

  /// Name of the pass that the input collection is on (empty string means take
  /// any pass)
  std::string inputPassName_;

  /// Name of the output collection that will be used to store the
  /// selected sim hits
  std::string outputCollection_;

};  // TruthHitProducer

}  // namespace trigscint

#endif  // TRIGSCINT_TRUTHHITPRODUCER_H_
