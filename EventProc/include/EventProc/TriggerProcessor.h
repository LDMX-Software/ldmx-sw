#ifndef TRIGGERPROCESSOR_H_
#define TRIGGERPROCESSOR_H_

#include "Event/TriggerResult.h"
#include "Event/Event.h"
#include "Framework/EventProcessor.h"
#include "Framework/ParameterSet.h"

class TriggerProcessor : public ldmxsw::Producer {

public:

    TriggerProcessor(const std::string& name, const ldmxsw::Process& process) : ldmxsw::Producer(name,process) { }

    virtual void configure(const ldmxsw::ParameterSet& pSet);

    virtual void produce(event::Event& event);

private:

    float layerESumCut_;
    int mode_;
    int startLayer_;
    int endLayer_;

    TString algoName_;
    event::TriggerResult result_;

};

#endif
