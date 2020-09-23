#include "DetDescr/EcalHexReadout.h"
#include "Framework/ConditionsObjectProvider.h"
#include "Event/RunHeader.h"
#include <sys/types.h>
#include <regex.h>


class EcalGeometryProvider : public ConditionsObjectProvider {
 public:
  /**
   * Class constructor
   * @param parameters -- uses the "EcalHexReadout" section to configure the EcalHexReadout
   */
  EcalGeometryProvider(const std::string& name, const std::string& tagname, const Parameters& parameters, Process& process);


  /** Destructor */
  virtual ~EcalGeometryProvider();
  
  /**
   * Provides access to the EcalHexReadout 
   * @note Currently, these are assumed to be valid for all time, but this behavior could be changed.  Users should not cache the pointer
   * between events
   */
  virtual std::pair<const ConditionsObject*,ConditionsIOV> getCondition(const EventHeader& context, const RunHeader& runcontext);

  /**
   * Take no action on release, as the object is permanently owned by the Provider
   */
  virtual void releaseConditionsObject(const ConditionsObject* co) {
  }

 private:
  /** Handle to the parameters, needed for future use during get condition */
  Parameters params_;
  /** Geometry as last used */
  std::string detectorGeometry_;
  EcalHexReadout* ecalGeometry_;
};

DECLARE_CONDITIONS_PROVIDER_NS(ldmx, EcalGeometryProvider);

namespace ldmx {
EcalGeometryProvider::EcalGeometryProvider(const std::string& name, const std::string& tagname, const Parameters& parameters, Process& process) :
    ConditionsObjectProvider{EcalHexReadout::CONDITIONS_OBJECT_NAME,tagname,parameters,process}, params_{parameters} {

      ecalGeometry_=0;
    }
EcalGeometryProvider::~EcalGeometryProvider() {
  if (ecalGeometry_) delete ecalGeometry_;
  ecalGeometry_=0;
  if (ecalTriggerGeometry_) delete ecalTriggerGeometry_;
  ecalTriggerGeometry_=0;
}

std::pair<const ConditionsObject*,ConditionsIOV> EcalGeometryProvider::getCondition(const EventHeader& context, const RunHeader& run_context) {
  static const std::string KEYNAME("detectors_valid");
  
  if (!ecalGeometry_) {
    detectorGeometry_=run_context.getDetectorName();
  
    // search through the subtrees
    for (auto key: params_.keys()) {
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
