#include "Ecal/EcalTrigPrimDigiProducer.h"
#include "Ecal/EcalTriggerGeometry.h"
#include "Event/EcalDigiCollection.h"
#include "Ecal/HgcrocTriggerCalculations.h"

namespace ldmx {
    EcalTrigPrimDigiProducer::EcalTrigPrimDigiProducer(const std::string& name, Process& process) : Producer(name, process) {

    }

    void EcalTrigPrimDigiProducer::configure(Parameters& ps) {
        digiCollName_ = ps.getParameter<std::string>( "digiCollName" );
        digiPassName_ = ps.getParameter<std::string>( "digiPassName" );
	condObjName_ = ps.getParameter<std::string>("condObjName","EcalTrigPrimDigiConditions");
    }

    void EcalTrigPrimDigiProducer::produce(Event& event) {
	const EcalTriggerGeometry& geom=getCondition<EcalTriggerGeometry>(EcalTriggerGeometry::CONDITIONS_OBJECT_NAME);
	
	// get the Ecal digis
        const EcalDigiCollection& ecalDigis = event.getObject<EcalDigiCollection>( digiCollName_ , digiPassName_ );

	// get the calibration object
	const IntegerTableCondition& conditions = conditions().getCondition<IntegerTableCondition>(condObjName_);

	// construct the calculator...
	HgcrocTriggerCalculations calc(conditions); 
	
	// Loop over the digis
	for (unsigned int ix=0; ix<ecalDigis.getNumDigis(); ix++) {
	    calc.addDigi();

	}
	// Now, we 
	
    }
}
