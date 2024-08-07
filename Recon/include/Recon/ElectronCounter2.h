
#ifndef RECON_ELECTRONCOUNTER2_H
#define RECON_ELECTRONCOUNTER2_H

//---< Framework >---//
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

//---< TrigScint >---//
#include "TrigScint/Event/TrigScintTrack.h"

//---< Ecal >---//
#include "Ecal/Event/EcalCluster.h"

namespace recon {

/**
 * Electron counting processor.
 *
 * This processor will use produced Trigger-Scintilator tracks and combine
 * the track position information with the position information of
 * clusters in the Ecal to determine the amount of non-interacting
 * electrons in the event.
 */
class ElectronCounter2 : public framework::Producer {
 public:
  /**
   * Constructor.
   *
   * @param name Name for this instance of the class.
   * @param process The Process class associated with EventProcessor,
   * provided by the framework.
   */
  ElectronCounter2(const std::string &name, framework::Process &process)
    : Producer(name, process) {}

  /**
   * Configure the processor using the given user specified parameters.
   *
   * The user specified parameters that are available are defined
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
  void produce(framework::Event &event) final override;

 private:
  /**
   * The name of the input collections
   * Element 0 : Trigger-Scintillator tracks
   * Element 1 : Ecal clusters
   */
  std::vector<std::string> inputCollections_;

  /**
   * The pass name of the input collection
   */
  std::string inputPassName_;

  /**
   * The name of the output collection
   * Element 0 : Number of electrons in event
   */
  std::string outputCollection_;

  // Tolerance for TS track and Ecal cluster overlap
  double xTolerance{0};
  double yTolerance{0};

  // Shift of ecal surface x,y
  std::vector<double> ecalPosShift_;

  // Energy split for electrons in ecal
  std::vector<double> ecalEnergySplit_;

  // If verbose or not
  bool verbose_{false};

};  // ElectronCounter2
}  // namespace recon

#endif  // RECON_ELECTRONCOUNTER2_H
