/*
 * @file EaTVisFeatures.h
 * @brief Class used to explore cuts and features for the EaT Visibles search,
 *        and can save features to a .txt file.
 * @author Kieran Wall, University of Virginia
 */

#ifndef HCAL_EATVISFEATURES_H_
#define HCAL_EATVISFEATURES_H_

// LDMX
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

namespace hcal {

  class EaTVisFeatures : public framework::Analyzer {
  public:
    EaTVisFeatures(const std::string& name, framework::Process& process)
      : Analyzer(name, process) {}

    ~EaTVisFeatures() override = default;

    void configure(framework::config::Parameters& parameters) override;

    void analyze(const framework::Event& event) override;

  private:

    bool training_{false};
    std::string trainingFile_;

    double beamEnergyMeV_{0.};

    std::string hcal_rec_collection_;
    std::string ecal_rec_collection_;
    bool recoil_from_tracking_{false};
    std::string track_collection_;
    std::string sp_collection_;
    std::string default_pass_name_;

    bool in_list(std::vector<int> parents, int a);

  };
  
} // namespace hcal

#endif
