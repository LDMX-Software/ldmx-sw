
#ifndef RECON_ELECTRONCOUNTER_H
#define RECON_ELECTRONCOUNTER_H

//---< Framework >---//
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

//---< TrigScint >---//
#include "TrigScint/TrigScintCluster.h"
#include "TrigScint/TrigScintTrack.h"

namespace recon {

/**
 * Electron counting processor.
 *
 * This processor will use objects reconstructed in the trigger scintillators,
 * or truth info on the number of electrons, and set the electron count in the
 * event.
 */
class ElectronCounter : public Producer {
 public:
  /**
   * Constructor.
   *
   * @param name Name for this instance of the class.
   * @param process The Process class associated with EventProcessor,
   * provided by the framework.
   */
  ElectronCounter(const std::string &name, Process &process);

  /// Destructor
  ~ElectronCounter();

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

 private:
  /**
   * The name of the input collection used for counting electrons
   */
  std::string inputColl_;

  /**
   * The pass name of the input collection used for counting electrons
   */
  std::string inputPassName_;

  /**
   * The name of the output collection used to save some electron counting
   * variables
   */
  std::string outputColl_;

  /**
   * The number of electrons actually simulated in the event
   */
  int nElectronsSim_{-1};

  /**
   * Use the number of electrons actually simulated in the event as the electron
   * count
   */
  int useSimElectronCount_{false};

};  // ElectronCounter
}  // namespace recon

#endif  // RECON_ELECTRONCOUNTER_H
