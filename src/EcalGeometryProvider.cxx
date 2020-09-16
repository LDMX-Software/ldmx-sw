#include "Ecal/EcalGeometryProvider.h"
#include "Event/RunHeader.h"
#include <sys/types.h>
#include <regex.h>

DECLARE_CONDITIONS_PROVIDER_NS(ldmx, EcalGeometryProvider);

namespace ldmx {
    EcalGeometryProvider::EcalGeometryProvider(const std::string& name, const std::string& tagname, const Parameters& parameters, Process& process) :
      ConditionsObjectProvider{name,tagname,parameters,process}, params_{parameters} {

	objectNames_.push_back(EcalHexReadout::CONDITIONS_OBJECT_NAME);
	objectNames_.push_back("EcalGeometry");
//	objectNames_.push_back(EcalTriggerGeometry::CONDITIONS_OBJECT_NAME);
	
	// create the ecalGeometry
	ecalGeometry_=0;
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
	static const std::string KEYNAME("detectors_valid");
	
	if (condition_name=="EcalGeometry" || condition_name==EcalHexReadout::CONDITIONS_OBJECT_NAME) {
	    if (!ecalGeometry_) {
		detectorGeometry_=run_context.getDetectorName();

		// find the right branch of the tree
		if (!params_.exists("EcalHexReadout")) {
		    EXCEPTION_RAISE("GeometryException","No configuration information found for EcalHexReadout");
		}
		
		const Parameters& phex=params_.getParameter<const Parameters&>("EcalHexReadout");
		// search through the subtrees
		for (auto key: phex.keys()) {
		    const Parameters& pver=phex.getParameter<const Parameters&>(key);
		        
		    if (!pver.exists(KEYNAME)) {
			ldmx_log(warn) << "No parameter " << KEYNAME << " found in " << key;
			// log strange situation and continue
			continue;
		    }
		    
		    const std::vector<std::string>& dets_valid=pver.getParameter<const std::vector<std::string>&>(KEYNAME);
		    for (auto detregex : dets_valid) {
			std::string regex(detregex);
			if (regex.empty()) continue; // no empty regex allowed
			if (regex[0]!='^') regex.insert(0,1,'^');
			if (regex.back()!='$') regex+='$';
			regex_t reg;

			
			int rv=regcomp(&reg,regex.c_str(),REG_EXTENDED|REG_ICASE|REG_NOSUB);
			if (rv) {
			    char err[1024];
			    regerror(rv,&reg,err,1024);
			    EXCEPTION_RAISE("GeometryException","Invalid detector regular expression : '"+regex+"' "
				+err);
			}
			int nmatch=regexec(&reg,detectorGeometry_.c_str(),0,0,0);
			regfree(&reg);
			if (!nmatch) {
			    ecalGeometry_=new EcalHexReadout(pver);
			    break;
			}
		    }
		    if (ecalGeometry_) break;
		    
		}
		if (!ecalGeometry_) {
		    EXCEPTION_RAISE("GeometryException","Unable to create EcalHexReadout");
		}

	    } else if (run_context.getDetectorName()!=detectorGeometry_) {
		EXCEPTION_RAISE("GeometryException","Attempting to run a single job with multiple geometries "+detectorGeometry_+" and '"+run_context.getDetectorName()+"'");
	    }
	    
	    return std::make_pair(ecalGeometry_,ConditionsIOV(run_context.getRunNumber(),run_context.getRunNumber(),true,true));
	}
    }
}
