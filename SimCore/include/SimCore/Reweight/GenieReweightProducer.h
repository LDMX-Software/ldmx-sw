//
// Created by Wesley Ketchum on 4/29/24.
//

#ifndef SIMCORE_GENIEREWEIGHTPRODUCER_H
#define SIMCORE_GENIEREWEIGHTPRODUCER_H

#include <map>
#include <string>

#include "Framework/EventProcessor.h"
#include "RwFramework/GSyst.h"
#include "SimCore/Event/EventWeights.h"

namespace genie {
class Interaction;
class HepMC3Converter;
}  // namespace genie

namespace genie::rew {
class GReWeight;
}

namespace simcore {

class GenieReweightProducer : public framework::Producer {
 public:
  // constructor
  GenieReweightProducer(const std::string& name, framework::Process& process);

  // default destructor
  virtual ~GenieReweightProducer();

  // configuration
  virtual void configure(framework::config::Parameters&);

  // on new run
  virtual void onNewRun(const ldmx::RunHeader& runHeader);

  // produce on the event
  virtual void produce(framework::Event& event);

 private:
  // seed to use
  int verbosity_;

  // input hepmc3 collection name
  std::string hepmc3CollName_;

  // input hepmc3 pass name
  std::string hepmc3PassName_;

  // output EventWeights collection name
  std::string eventWeightsCollName_;

  // seed to use
  int seed_;

  // number of weights to be calculated per event
  size_t n_weights_;

  // GENIE tune
  std::string tune_;

  // variations to run
  std::map<ldmx::EventWeights::VariationType, std::vector<double> >
      variation_map_;

  // hepmc3 convertor
  genie::HepMC3Converter* hepMC3Converter_;

  genie::rew::GReWeight* genie_rw_;

  void reinitializeGenieReweight();
  void reconfigureGenieReweight(size_t);

  inline static genie::rew::EGSyst variation_type_to_genie_dial(
      const ldmx::EventWeights::VariationType& vtype) {
    switch (vtype) {
      case ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_MFP_pi:
        return genie::rew::EGSyst::kINukeTwkDial_MFP_pi;
      case ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_MFP_N:
        return genie::rew::EGSyst::kINukeTwkDial_MFP_N;
      case ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_FrCEx_pi:
        return genie::rew::EGSyst::kINukeTwkDial_FrCEx_pi;
      case ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_FrInel_pi:
        return genie::rew::EGSyst::kINukeTwkDial_FrInel_pi;
      case ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_FrAbs_pi:
        return genie::rew::EGSyst::kINukeTwkDial_FrAbs_pi;
      case ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_FrPiProd_pi:
        return genie::rew::EGSyst::kINukeTwkDial_FrPiProd_pi;
      case ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_FrCEx_N:
        return genie::rew::EGSyst::kINukeTwkDial_FrCEx_N;
      case ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_FrInel_N:
        return genie::rew::EGSyst::kINukeTwkDial_FrInel_N;
      case ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_FrAbs_N:
        return genie::rew::EGSyst::kINukeTwkDial_FrAbs_N;
      case ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_FrPiProd_N:
        return genie::rew::EGSyst::kINukeTwkDial_FrPiProd_N;
      case ldmx::EventWeights::VariationType ::kGENIE_HadrNuclTwkDial_FormZone:
        return genie::rew::EGSyst::kHadrNuclTwkDial_FormZone;
      default:
        return genie::rew::EGSyst::kNullSystematic;
    }
    return genie::rew::EGSyst::kNullSystematic;
  }
};
}  // namespace simcore

#endif  // SIMCORE_GENIEREWEIGHTPRODUCER_H
