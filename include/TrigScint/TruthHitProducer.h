
#ifndef TRIGSCINT_TRUTHHITPRODUCER_H_
#define TRIGSCINT_TRUTHHITPRODUCER_H_

/***************/
/*   ldmx-sw   */
/***************/

#include "Event/EventConstants.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "Event/TrigScintHit.h"
#include "Framework/EventProcessor.h"
#include "Framework/Configure/Parameters.h"

namespace ldmx {

/**
 * Producer making a collection based on some truth info cuts.
 * Like, skimming out only the beam electrons.
 * This producer creates a new hit collection,
 * that can then be fed to digi etc like anything else.
 */
class TruthHitProducer : public Producer {

public:
  /**
   * Constructor.
   *
   * @param name Name for this instance of the class.
   * @param process The Process class associated with EventProcessor,
   * provided by the framework.
   */
  TruthHitProducer(const std::string &name, Process &process);

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
  void configure(Parameters &parameters) final override;

  /**
   * Process the event and put new data products into it.
   *
   * @param event The event to process.
   */
  void produce(Event &event);

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

}; // TruthHitProducer

} // namespace ldmx

#endif // TRIGSCINT_TRUTHHITPRODUCER_H_
