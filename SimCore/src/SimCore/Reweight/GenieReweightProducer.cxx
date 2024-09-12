//
// Created by Wesley Ketchum on 4/29/24.
//

#include "SimCore/Reweight/GenieReweightProducer.h"
#include "SimCore/Event/HepMC3GenEvent.h"
#include "SimCore/Event/EventWeights.h"

#include "Framework/EventGen/HepMC3Converter.h"
#include "Framework/EventGen/EventRecord.h"
#include "Framework/Interaction/Interaction.h"
#include "Framework/Utils/RunOpt.h"

#include "RwFramework/GReWeightI.h"
#include "RwFramework/GSystSet.h"
#include "RwFramework/GSyst.h"
#include "RwFramework/GReWeight.h"
#include "RwCalculators/GReWeightNuXSecNCEL.h"
#include "RwCalculators/GReWeightNuXSecCCQE.h"
#include "RwCalculators/GReWeightNuXSecCCRES.h"
#include "RwCalculators/GReWeightNuXSecCOH.h"
#include "RwCalculators/GReWeightNonResonanceBkg.h"
#include "RwCalculators/GReWeightFGM.h"
#include "RwCalculators/GReWeightDISNuclMod.h"
#include "RwCalculators/GReWeightResonanceDecay.h"
#include "RwCalculators/GReWeightFZone.h"
#include "RwCalculators/GReWeightINuke.h"
#include "RwCalculators/GReWeightAGKY.h"
#include "RwCalculators/GReWeightNuXSecCCQEaxial.h"
#include "RwCalculators/GReWeightNuXSecCCQEvec.h"
#include "RwCalculators/GReWeightNuXSecNCRES.h"
#include "RwCalculators/GReWeightNuXSecDIS.h"

#include "RwCalculators/GReWeightINukeParams.h"
#include "RwCalculators/GReWeightNuXSecNC.h"
#include "RwCalculators/GReWeightXSecEmpiricalMEC.h"
#include "RwCalculators/GReWeightXSecMEC.h"
#include "RwCalculators/GReWeightDeltaradAngle.h"

#include "HepMC3/GenEvent.h"
#include "HepMC3/Print.h"

#include <iostream>
#include <random>



namespace simcore {

    GenieReweightProducer::GenieReweightProducer(const std::string& name,
                                                 framework::Process& process)
        : Producer(name, process)
    {
        hepMC3Converter_ = new genie::HepMC3Converter;
        genie_rw_ = new genie::rew::GReWeight;
    }

    GenieReweightProducer::~GenieReweightProducer()
    { delete hepMC3Converter_; delete genie_rw_; }

    void GenieReweightProducer::configure(framework::config::Parameters & ps) {
        hepmc3CollName_        = ps.getParameter<std::string>("hepmc3CollName");
        hepmc3PassName_        = ps.getParameter<std::string>("hepmc3PassName");
        eventWeightsCollName_  = ps.getParameter<std::string>("eventWeightsCollName");
        verbosity_             = ps.getParameter<int>("verbosity");
        seed_                  = ps.getParameter<int>("seed");
        n_weights_             = (size_t)(ps.getParameter<int>("n_weights"));
        auto var_types_strings = ps.getParameter< std::vector<std::string> >("var_types");

        std::default_random_engine generator(seed_);
        std::normal_distribution<double> normal_distribution;

        for (auto const& vt_str : var_types_strings) {
            auto vtype = ldmx::EventWeights::string_to_variation_type(vt_str);
            for(size_t i_w=0; i_w<n_weights_; ++i_w)
                variation_map_[vtype].push_back(normal_distribution(generator));
        }

    }

    void GenieReweightProducer::reinitializeGenieReweight()
    {
      delete genie_rw_;
      genie_rw_ = new genie::rew::GReWeight;

      genie::RunOpt::Instance()->SetTuneName(tune_);
      if ( ! genie::RunOpt::Instance()->Tune() ) {
        EXCEPTION_RAISE("ConfigurationException","No TuneId in RunOption.");
      }
      genie::RunOpt::Instance()->BuildTune();

      genie_rw_->AdoptWghtCalc( "hadro_fzone",     new genie::rew::GReWeightFZone           );
      genie_rw_->AdoptWghtCalc( "hadro_intranuke", new genie::rew::GReWeightINuke           );

      auto & syst = genie_rw_->Systematics();
      for (auto var : variation_map_)
        syst.Init(variation_type_to_genie_dial(var.first));
    }

    void GenieReweightProducer::onNewRun(const ldmx::RunHeader &runHeader) {

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
        if(verbosity_>=0)
          std::cout << "Found new tune " << new_tune << " (used to be " << tune_ << ")" << std::endl;
        tune_ = new_tune;
        reinitializeGenieReweight();
      }
    }

    void GenieReweightProducer::reconfigureGenieReweight(size_t i_w)
    {
      auto & syst = genie_rw_->Systematics();
      for(auto var : variation_map_){
        if(var.first==ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_MFP_pi)
            syst.Set(genie::rew::GSyst::FromString("MFP_pi"), var.second[i_w]);
        else if(var.first==ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_MFP_N)
          syst.Set(genie::rew::GSyst::FromString("MFP_N"), var.second[i_w]);
        else if(var.first==ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_FrCEx_pi)
          syst.Set(genie::rew::GSyst::FromString("FrCEx_pi"), var.second[i_w]);
        else if(var.first==ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_FrInel_pi)
          syst.Set(genie::rew::GSyst::FromString("FrInel_pi"), var.second[i_w]);
        else if(var.first==ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_FrAbs_pi)
          syst.Set(genie::rew::GSyst::FromString("FrAbs_pi"), var.second[i_w]);
        else if(var.first==ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_FrPiProd_pi)
          syst.Set(genie::rew::GSyst::FromString("FrPiProd_pi"), var.second[i_w]);
        else if(var.first==ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_FrCEx_N)
          syst.Set(genie::rew::GSyst::FromString("FrCEx_N"), var.second[i_w]);
        else if(var.first==ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_FrInel_N)
          syst.Set(genie::rew::GSyst::FromString("FrInel_N"), var.second[i_w]);
        else if(var.first==ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_FrAbs_N)
          syst.Set(genie::rew::GSyst::FromString("FrAbs_N"), var.second[i_w]);
        else if(var.first==ldmx::EventWeights::VariationType::kGENIE_INukeTwkDial_FrPiProd_N)
          syst.Set(genie::rew::GSyst::FromString("FrPiProd_N"), var.second[i_w]);
        else if (var.first==ldmx::EventWeights::VariationType::kGENIE_HadrNuclTwkDial_FormZone)
          syst.Set(genie::rew::GSyst::FromString("FormZone"), var.second[i_w]);

      }
      genie_rw_->Reconfigure();

    }

    void GenieReweightProducer::produce(framework::Event &event) {

        //grab the input hepmc3 event collection
        auto hepmc3_col = event.getObject< std::vector<ldmx::HepMC3GenEvent> >(hepmc3CollName_, hepmc3PassName_);

        //create an output weights
        ldmx::EventWeights ev_weights(variation_map_);

        for(size_t i_w=0; i_w < n_weights_; ++i_w) {

            double running_weight=1;

            reconfigureGenieReweight(i_w);

            //setup a loop here ... but we're going to force only looping over one interaction if it exists for now.
            for (size_t i_ev=0; i_ev<1; ++i_ev) {

                auto const& hepmc3_ev = hepmc3_col.at(i_ev);
                //fill our event data into a HepMC3GenEvent
                HepMC3::GenEvent hepmc3_genev;
                hepmc3_genev.read_data(hepmc3_ev);

                //print it out to check it ...
                if(i_w==0 && verbosity_>=1) HepMC3::Print::line(hepmc3_genev, true); //print attributes

                //now convert to genie event record
                auto genie_ev_record_ptr = hepMC3Converter_->RetrieveGHEP(hepmc3_genev);

                //print that out too ...
                if(i_w==0 && verbosity_>=1) genie_ev_record_ptr->Print(std::cout);

                //auto this_weight = 1.0 + var_value*0.05;
                auto this_weight = genie_rw_->CalcWeight(*genie_ev_record_ptr);

                running_weight = running_weight * this_weight;

            }//end loop over interactions in event

            ev_weights.addWeight(running_weight);

        } //end loop over weights

        if(verbosity_>=1)
          ev_weights.Print();

        event.add(eventWeightsCollName_, ev_weights);

    }
}

DECLARE_PRODUCER_NS(simcore, GenieReweightProducer);
