#ifndef HCAL_HCALDIGIPRODUCER_H_
#define HCAL_HCALDIGIPRODUCER_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <memory>  //for smart pointers
#include <set>     //for tracking used detector IDs

//----------//
//   LDMX   //
//----------//
#include "DetDescr/HcalDigiID.h"
#include "DetDescr/HcalGeometry.h"
#include "DetDescr/HcalID.h"
#include "Framework/EventProcessor.h"
#include "Recon/Event/EventConstants.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "Tools/HgcrocEmulator.h"
#include "Tools/NoiseGenerator.h"

namespace hcal {

/**
 * @class HcalDigiProducer
 * @brief Performs basic HCal digitization
 */
class HcalDigiProducer : public framework::Producer {
 public:
  /**
   * Constructor
   * Makes unique noise generator and injector for this class
   */
  HcalDigiProducer(const std::string& name, framework::Process& process);

  /// Default destructor
  ~HcalDigiProducer() = default;

  /**
   * Configure this producer from the python configuration.
   * Sets event constants and configures the noise generator, noise injector,
   * and pulse function. Creates digi collection
   */
  void configure(framework::config::Parameters&) final override;

  /**
   * Simulates measurement of pulse and creates digi collection for input event.
   */
  void produce(framework::Event& event) final override;

 private:
  ///////////////////////////////////////////////////////////////////////////////////////
  // Python Configuration Parameters

  /// input hit collection name
  std::string inputCollName_;

  /// input pass name
  std::string inputPassName_;

  /// output hit collection name
  std::string digiCollName_;

  /// Time interval for chip clock in ns
  double clockCycle_;

  /// Depth of ADC buffer.
  int nADCs_;

  /// Index for the Sample Of Interest in the list of digi samples
  int iSOI_;

  /// Conversion from energy in MeV to voltage in mV
  double MeV_;

  /// Strip attenuation length [m]
  double attlength_;

  ///////////////////////////////////////////////////////////////////////////////////////
  // Other member variables

  /// Put noise into empty channels, not configurable, only helpful in
  /// development
  bool noise_{true};

  /// Hgcroc Emulator to digitize analog voltage signals
  std::unique_ptr<ldmx::HgcrocEmulator> hgcroc_;

  /// Conversion from time in ns to ticks of the internal clock
  double ns_;

  /// Generates noise hits based off of number of cells that are not hit
  std::unique_ptr<ldmx::NoiseGenerator> noiseGenerator_;

  /// Generates Gaussian noise on top of real hits
  std::unique_ptr<TRandom3> noiseInjector_;
};
}  // namespace hcal

#endif
