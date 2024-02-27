
#ifndef RECON_BEAMELECTRONLOCATOR_H
#define RECON_BEAMELECTRONLOCATOR_H

//---< Framework >---//
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

//---< Recon >---//
#include "Recon/Event/BeamElectronTruth.h"
#include "SimCore/Event/SimCalorimeterHit.h"

namespace recon {

/**
 * Electron counting processor.
 *
 * This processor uses calo simhits in e.g. the target to get truth info
 * about beam electrons. The hits associated with beam electrons can already
 * be isolated by the truth hit collection producer. However, the simhits
 * have approximately infinite resolution. This processor's raison d'être
 * is to run some sort of  grouping, to ensure we have at most one
 * reconstructed truth information object per electron.
 *
 */
class BeamElectronLocator : public framework::Producer {
 public:
  /**
   * Constructor.
   *
   * @param name Name for this instance of the class.
   * @param process The Process class associated with EventProcessor,
   * provided by the framework.
   */
  BeamElectronLocator(const std::string &name, framework::Process &process);

  /// Destructor
  virtual ~BeamElectronLocator();

  /**
   * Configure the processor using the given user specified parameters.
   *
   * The user specified parameters that are available are defined
   * in the python configuration class. Look at the beamElecronLocator.py
   * in Recon/python for the python structure.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters &parameters) final override;

  /**
   * Prints the configuration to log in debug mode
   */
  void onProcessStart() final override;

  /**
   * Process the event and put new data products into it.
   *
   * @param event The event to process.
   */
  void produce(framework::Event &event) final override;

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
   * The min value measured by the system (edge) in X, in mm
   **/
  double minXmm_;
  /**
   * The max value measured by the system (edge) in X, in mm
   **/
  double maxXmm_;

  /**
   * The min value measured by the system (edge) in Y, in mm
   **/
  double minYmm_;
  /**
   * The max value measured by the system (edge) in Y, in mm
   **/
  double maxYmm_;

  /**
   * The granularity of the detector (e.g. TS) in X, in mm
   **/
  double granularityXmm_;
  /**
   * The granularity of the detector (e.g. TS) in Y, in mm
   **/
  double granularityYmm_;

  /**
   * The tolerance within which simhits are considered to belong to
   * the same electron.
   **/
  double tolerance_;

  /**
   * Indicate verbose printout to log according to log level.
   */
  bool verbose_{false};

  /**
   * Bins coordinates according to some given granularity (passed as argument).
   * Returns the lower bin edge. -1 for underflow (coordinate < min_in_mm);
   * with a system of N bins, returns n = N if coordinate > max_in_mm.
   *
   * TODO also implement a function that returns the grid of non-empty hit
   * coordinates, which accounts for that we don't know the multiplicity at a
   * location
   */
  int bin(float coordinate, double binWidth, double min, double max);

};  // BeamElectronLocator
}  // namespace recon

#endif  // RECON_BEAMELECTRONLOCATOR_H
