//
// Created by Wesley Ketchum on 4/29/24.
//

#include "SimCore/Reweight/GenieReweightProducer.h"

#include <iostream>
#include <random>

#include "Framework/EventGen/EventRecord.h"
#include "Framework/EventGen/HepMC3Converter.h"
#include "Framework/Interaction/Interaction.h"
#include "Framework/Messenger/Messenger.h"
#include "Framework/Utils/AppInit.h"
#include "Framework/Utils/RunOpt.h"
#include "HepMC3/GenEvent.h"
#include "HepMC3/PrintStreams.h"
#include "RwCalculators/GReWeightAGKY.h"
#include "RwCalculators/GReWeightDISNuclMod.h"
#include "RwCalculators/GReWeightDeltaradAngle.h"
#include "RwCalculators/GReWeightFGM.h"
#include "RwCalculators/GReWeightFZone.h"
#include "RwCalculators/GReWeightINuke.h"
#include "RwCalculators/GReWeightINukeParams.h"
#include "RwCalculators/GReWeightNonResonanceBkg.h"
#include "RwCalculators/GReWeightNuXSecCCQE.h"
#include "RwCalculators/GReWeightNuXSecCCQEaxial.h"
#include "RwCalculators/GReWeightNuXSecCCQEvec.h"
#include "RwCalculators/GReWeightNuXSecCCRES.h"
#include "RwCalculators/GReWeightNuXSecCOH.h"
#include "RwCalculators/GReWeightNuXSecDIS.h"
#include "RwCalculators/GReWeightNuXSecNC.h"
#include "RwCalculators/GReWeightNuXSecNCEL.h"
#include "RwCalculators/GReWeightNuXSecNCRES.h"
#include "RwCalculators/GReWeightResonanceDecay.h"
#include "RwCalculators/GReWeightXSecEmpiricalMEC.h"
#include "RwCalculators/GReWeightXSecMEC.h"
#include "RwFramework/GReWeight.h"
#include "RwFramework/GReWeightI.h"
#include "RwFramework/GSyst.h"
#include "RwFramework/GSystSet.h"
#include "SimCore/Event/EventWeights.h"
#include "SimCore/Event/HepMC3GenEvent.h"

namespace simcore {

GenieReweightProducer::GenieReweightProducer(const std::string& name,
                                             framework::Process& process)
    : Producer(name, process) {
  hep_mc3_converter_ = std::make_unique<genie::HepMC3Converter>();
  genie_rw_ = std::make_unique<genie::rew::GReWeight>();
}

void GenieReweightProducer::configure(framework::config::Parameters& ps) {
  hepmc3_coll_name_ = ps.get<std::string>("hepmc3CollName");
  hepmc3_pass_name_ = ps.get<std::string>("hepmc3PassName");
  event_weights_coll_name_ = ps.get<std::string>("eventWeightsCollName");
  seed_ = ps.get<int>("seed");
  n_weights_ = static_cast<size_t>(ps.get<int>("n_weights"));
  auto var_types_strings = ps.get<std::vector<std::string> >("var_types");

  message_threshold_file_ = ps.get<std::string>("message_threshold_file");

  std::default_random_engine generator(seed_);
  std::normal_distribution<double> normal_distribution;

  for (auto const& vt_str : var_types_strings) {
    auto vtype = ldmx::EventWeights::stringToVariationType(vt_str);
    for (size_t i_w = 0; i_w < n_weights_; ++i_w)
      variation_map_[vtype].push_back(normal_distribution(generator));
  }
}

void GenieReweightProducer::reinitializeGenieReweight() {
  genie_rw_ = std::make_unique<genie::rew::GReWeight>();

  // set message thresholds
  genie::utils::app_init::MesgThresholds(message_threshold_file_);

  genie::RunOpt::Instance()->SetTuneName(tune_);
  if (!genie::RunOpt::Instance()->Tune()) {
    EXCEPTION_RAISE("ConfigurationException", "No TuneId in RunOption.");
  }
  genie::RunOpt::Instance()->BuildTune();

  genie_rw_->AdoptWghtCalc("hadro_fzone", new genie::rew::GReWeightFZone);
  genie_rw_->AdoptWghtCalc("hadro_intranuke", new genie::rew::GReWeightINuke);

  auto& syst = genie_rw_->Systematics();
  for (auto var : variation_map_)
    syst.Init(variationTypeToGenieDial(var.first));
}

void GenieReweightProducer::onNewRun(const ldmx::RunHeader& runHeader) {
  const std::string genie_tune_par = "GenieTune";
  std::string new_tune;
  for (auto par : runHeader.getStringParameters())
    if (par.first.size() >= genie_tune_par.size() &&
        par.first.compare(par.first.size() - genie_tune_par.size(),
                          genie_tune_par.size(), genie_tune_par) == 0) {
      new_tune = par.second;
      break;
    }

  if (tune_ != new_tune) {
    ldmx_log(debug) << "Found new tune " << new_tune << " (used to be " << tune_
                    << ")" << std::endl;
    tune_ = new_tune;
    reinitializeGenieReweight();
  }
}

void GenieReweightProducer::reconfigureGenieReweight(size_t i_w) {
  auto& syst = genie_rw_->Systematics();
  for (auto var : variation_map_) {
    if (var.first ==
        ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_MFP_pi)
      syst.Set(genie::rew::GSyst::FromString("MFP_pi"), var.second[i_w]);
    else if (var.first ==
             ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_MFP_N)
      syst.Set(genie::rew::GSyst::FromString("MFP_N"), var.second[i_w]);
    else if (var.first ==
             ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_FrCEx_pi)
      syst.Set(genie::rew::GSyst::FromString("FrCEx_pi"), var.second[i_w]);
    else if (var.first ==
             ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_FrInel_pi)
      syst.Set(genie::rew::GSyst::FromString("FrInel_pi"), var.second[i_w]);
    else if (var.first ==
             ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_FrAbs_pi)
      syst.Set(genie::rew::GSyst::FromString("FrAbs_pi"), var.second[i_w]);
    else if (var.first ==
             ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_FrPiProd_pi)
      syst.Set(genie::rew::GSyst::FromString("FrPiProd_pi"), var.second[i_w]);
    else if (var.first ==
             ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_FrCEx_N)
      syst.Set(genie::rew::GSyst::FromString("FrCEx_N"), var.second[i_w]);
    else if (var.first ==
             ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_FrInel_N)
      syst.Set(genie::rew::GSyst::FromString("FrInel_N"), var.second[i_w]);
    else if (var.first ==
             ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_FrAbs_N)
      syst.Set(genie::rew::GSyst::FromString("FrAbs_N"), var.second[i_w]);
    else if (var.first ==
             ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_FrPiProd_N)
      syst.Set(genie::rew::GSyst::FromString("FrPiProd_N"), var.second[i_w]);
    else if (var.first ==
             ldmx::EventWeights::VariationType::kGENIE_HadrNuclTwkDial_FormZone)
      syst.Set(genie::rew::GSyst::FromString("FormZone"), var.second[i_w]);
  }
  genie_rw_->Reconfigure();
}

void GenieReweightProducer::produce(framework::Event& event) {
  // grab the input hepmc3 event collection
  auto hepmc3_col = event.getObject<std::vector<ldmx::HepMC3GenEvent> >(
      hepmc3_coll_name_, hepmc3_pass_name_);

  // create an output weights
  ldmx::EventWeights ev_weights(variation_map_);

  for (size_t i_w = 0; i_w < n_weights_; ++i_w) {
    double running_weight = 1;

    reconfigureGenieReweight(i_w);

    // setup a loop here ... but we're going to force only looping over one
    // interaction if it exists for now.
    for (size_t i_ev = 0; i_ev < 1; ++i_ev) {
      auto const& hepmc3_ev = hepmc3_col.at(i_ev);
      // fill our event data into a HepMC3GenEvent
      HepMC3::GenEvent hepmc3_genev;
      hepmc3_genev.read_data(hepmc3_ev);

      // print it out to check it ...
      if (i_w == 0) ldmx_log(debug) << hepmc3_genev;

      // now convert to genie event record
      auto genie_ev_record_ptr = hep_mc3_converter_->RetrieveGHEP(hepmc3_genev);

      // print that out too ...
      if (i_w == 0) ldmx_log(debug) << *genie_ev_record_ptr;

      // auto this_weight = 1.0 + var_value*0.05;
      auto this_weight = genie_rw_->CalcWeight(*genie_ev_record_ptr);

      running_weight = running_weight * this_weight;

    }  // end loop over interactions in event

    ev_weights.addWeight(running_weight);

  }  // end loop over weights

  ldmx_log(trace) << ev_weights;

  event.add(event_weights_coll_name_, ev_weights);
}
}  // namespace simcore

DECLARE_PRODUCER(simcore::GenieReweightProducer)
