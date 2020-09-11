#include "Ecal/EcalGeometryProvider.h"

DECLARE_CONDITIONS_PROVIDER_NS(ldmx, EcalGeometryProvider);

namespace ldmx {
    EcalGeometryProvider::EcalGeometryProvider(const std::string& name, const std::string& tagname, const Parameters& parameters, Process& process) :
	ConditionsObjectProvider{name,tagname,parameters,process} {

	objectNames_.push_back(EcalHexReadout::CONDITIONS_OBJECT_NAME);
	objectNames_.push_back("EcalGeometry");
//	objectNames_.push_back(EcalTriggerGeometry::CONDITIONS_OBJECT_NAME);
	
	// create the ecalGeometry
	ecalGeometry_=new EcalHexReadout(parameters);
	/*
	// create the ecalTriggerGeometry
	int symmetry=0x100;
        ecalTriggerGeometry_=new EcalTriggerGeometry(symmetry,ecalGeometry_);
	*/
    }
    EcalGeometryProvider::~EcalGeometryProvider() {
	if (ecalGeometry_) delete ecalGeometry_;
	ecalGeometry_=0;
    }
    std::pair<const ConditionsObject*,ConditionsIOV> EcalGeometryProvider::getCondition(const std::string& condition_name, const EventHeader& context, const RunHeader& run_context) {
	if (condition_name=="EcalGeometry" || condition_name==EcalHexReadout::CONDITIONS_OBJECT_NAME) {
	    return std::make_pair(ecalGeometry_,ConditionsIOV(true,true));
	}
    }
}
