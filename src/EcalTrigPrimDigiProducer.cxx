#include "Ecal/EcalTrigPrimDigiProducer.h"
#include "Ecal/EcalTriggerGeometry.h"

namespace ldmx {
    EcalTrigPrimDigiProducer::EcalTrigPrimDigiProducer(const std::string& name, Process& process) : Producer(name, process) {
    }

    void EcalTrigPrimDigiProducer::configure(Parameters&) {
    }

    void EcalTrigPrimDigiProducer::produce(Event& event) {
	const EcalTriggerGeometry& geom=getCondition<EcalTriggerGeometry>(EcalTriggerGeometry::CONDITIONS_OBJECT_NAME);
	
	
    }
}
