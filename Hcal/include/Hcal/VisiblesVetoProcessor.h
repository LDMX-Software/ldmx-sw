#ifndef EVENTPROC_VISIBLESVETOPROCESSOR_H_
#define EVENTPROC_VISIBLESVETOPROCESSOR_H_

// LDMX
#include "Hcal/Event/HcalHit.h"
#include "Hcal/Event/VisiblesVetoResult.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Tools/ONNXRuntime.h"

namespace hcal {

  class VisiblesVetoProcessor : public framework::Producer {
  public:
    EcalVetoProcessor(const std::string& name, framework::Process& process)
      : Producer(name, process) {}

    virtual ~VisiblesVetoProcessor() {}

    void configure(framework::config::Parameters& parameters) override;

    void produce(framework::Event& event) override;

  private:
    void clearProcessor();

    void buildBDTFeatureVector(const ldmx::VisiblesVetoResult& result);

    void saveAsCSV(const std::string& filename);

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

    std::string bdtFileName_;
    std::string rocFileName_;
    std::vector<float> bdtFeatures_;
    std::string featureListName_;

    std::string rec_pass_name_;
    std::string rec_coll_name_;

    std::string collectionName_{"VisiblesVeto"};

    std::unique_ptr<ldmx::Ort::ONNXRuntime> rt_;

  };
  
} // namespace hcal

#endif
