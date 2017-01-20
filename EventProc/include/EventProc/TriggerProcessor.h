/**
 * @file TriggerProcessor.h
 * @brief Class that provides a trigger decision for recon using a TriggerResult object
 * @author Josh Hiltbrand, University of Minnesota
 */

#ifndef EVENTPROC_TRIGGERPROCESSOR_H_
#define EVENTPROC_TRIGGERPROCESSOR_H_

#include "Event/TriggerResult.h"
#include "Event/Event.h"
#include "Framework/ParameterSet.h"
#include "Framework/EventProcessor.h"

/**
 * @class TriggerProcessor
 * @brief Provides a trigger decision for recon using a TriggerResult object.
 */
class TriggerProcessor : public ldmxsw::Producer {

    public:

        TriggerProcessor(const std::string& name, const ldmxsw::Process& process) :
                ldmxsw::Producer(name, process) {
        }

        virtual ~TriggerProcessor() {;}

        virtual void configure(const ldmxsw::ParameterSet& pSet);

        virtual void produce(event::Event& event);

    private:

        float layerESumCut_{0};
        int mode_{0};
        int startLayer_{0};
        int endLayer_{0};

        TString algoName_;
        event::TriggerResult result_;

};

#endif
