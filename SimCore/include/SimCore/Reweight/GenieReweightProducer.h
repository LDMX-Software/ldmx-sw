//
// Created by Wesley Ketchum on 4/29/24.
//

#ifndef SIMCORE_GENIEREWEIGHTPRODUCER_H
#define SIMCORE_GENIEREWEIGHTPRODUCER_H

#include "SimCore/Event/EventWeights.h"

#include "Framework/EventProcessor.h"

#include <string>
#include <map>

namespace genie {
    class Interaction;
    class HepMC3Converter;
}

namespace genie::rew {
    class GReWeight;
}

namespace simcore {

    class GenieReweightProducer : public framework::Producer {
    public:

        //constructor
        GenieReweightProducer(const std::string& name, framework::Process& process);

        //default destructor
        virtual ~GenieReweightProducer();

        //configuration
        virtual void configure(framework::config::Parameters&);

        //on new run
        virtual void onNewRun(const ldmx::RunHeader &runHeader);

        //produce on the event
        virtual void produce(framework::Event& event);

    private:

        //input hepmc3 collection name
        std::string hepmc3CollName_;

        //input hepmc3 pass name
        std::string hepmc3PassName_;

        //output EventWeights collection name
        std::string eventWeightsCollName_;

        //seed to use
        int seed_;

        // number of weights to be calculated per event
        size_t n_weights_;

        // GENIE tune
        std::string tune_;

        //variations to run
        std::map< ldmx::EventWeights::VariationType, std::vector<double> > variation_map_;

        //hepmc3 convertor
        genie::HepMC3Converter* hepMC3Converter_;

        genie::rew::GReWeight* genie_rw_;

        void reconfigureGenieReweight();

    };

}

#endif //SIMCORE_GENIEREWEIGHTPRODUCER_H
