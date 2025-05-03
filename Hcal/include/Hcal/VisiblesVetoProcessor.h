/*
 * @file VisiblesVetoProcessor.h
 * @brief Class that determines if activity in the Hcal looks like a LLP
 * @author Tyler Horoho, University of Virginia
 */

#ifndef EVENTPROC_VISIBLESVETOPROCESSOR_H_
#define EVENTPROC_VISIBLESVETOPROCESSOR_H_

// LDMX
#include "Hcal/Event/HcalHit.h"
#include "Hcal/Event/VisiblesVetoResult.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Tools/ONNXRuntime.h"

// recoil tracking
#include "Tracking/Event/Track.h"

namespace hcal {

  class VisiblesVetoProcessor : public framework::Producer {
  public:
    VisiblesVetoProcessor(const std::string& name, framework::Process& process)
      : Producer(name, process) {}

    virtual ~VisiblesVetoProcessor() {}

    void configure(framework::config::Parameters& parameters) override;

    void produce(framework::Event& event) override;

  private:
    void clearProcessor();

    void buildBDTFeatureVector(const ldmx::VisiblesVetoResult& result);

    /* a function for finding track IDs for truth-level tracking */
    bool in_list(std::vector<int> parents, int a);

    int nLayersHit_{0};
    double xStd_{0};
    double yStd_{0};
    double zStd_{0};
    double xMean_{0};
    double yMean_{0};
    double rMean_{0};
    int isoHits_{0};
    double isoEnergy_{0};
    int nReadoutHits_{0};
    double summedDet_{0};
    double rMeanFromPhotonProj_{0};

    double bdtCutVal_{0};

    double beamEnergyMeV_{0};

    bool verbose_{false};

    std::vector<float> bdtFeatures_;
    std::string featureListName_;

    // Pass and collection names
    std::string rec_pass_name_;
    std::string rec_coll_name_;
    bool recoil_from_tracking_;
    std::string track_pass_name_;
    std::string track_collection_;
    std::string sp_coll_name_;
    std::string sp_pass_name_;

    std::string collectionName_{"VisiblesVeto"};

    std::unique_ptr<ldmx::Ort::ONNXRuntime> rt_;

  };
  
} // namespace hcal

#endif
