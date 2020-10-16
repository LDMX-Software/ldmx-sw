#include "DetDescr/EcalHexReadout.h"
#include "Framework/ConditionsObjectProvider.h"
#include "Event/RunHeader.h"
#include "Event/EventHeader.h"
#include <sys/types.h>
#include <regex.h>

/**
 * @file EcalGeometryProvider.cxx
 * @brief Class that creates EcalHexReadout object based on python specification
 * @author Jeremiah Mans, UMN
 */

namespace ldmx {

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
   * Provides access to the EcalHexReadout or EcalTriggerGeometry
   * @note Currently, these are assumed to be valid for all time, but this behavior could be changed.  Users should not cache the pointer
   * between events
   */
  virtual std::pair<const ConditionsObject*,ConditionsIOV> getCondition(const EventHeader& context);

  /**
   * Take no action on release, as the object is permanently owned by the Provider
   */
  virtual void releaseConditionsObject(const ConditionsObject* co) {
  }

  virtual void onNewRun(RunHeader& rh) {
    if (detectorGeometry_.empty()) detectorGeometry_=rh.getDetectorName();
    else if (ecalGeometry_!=nullptr && detectorGeometry_!=rh.getDetectorName()) {
      EXCEPTION_RAISE("GeometryException","Attempting to run a single job with multiple geometries "+detectorGeometry_+" and '"+rh.getDetectorName()+"'");
    }
    //make sure detector name has been set
    if (detectorGeometry_.empty())
        EXCEPTION_RAISE("GeometryException","EcalGeometryProvider unable to get the name of the detector from the RunHeader.");
  }

    
 private:
  /** Handle to the parameters, needed for future use during get condition */
  Parameters params_;
  /** Geometry as last used */
  std::string detectorGeometry_;
  EcalHexReadout* ecalGeometry_;
};
    

EcalGeometryProvider::EcalGeometryProvider(const std::string& name, const std::string& tagname, const Parameters& parameters, Process& process) : ConditionsObjectProvider{EcalHexReadout::CONDITIONS_OBJECT_NAME,tagname,parameters,process}, params_{parameters} {
      
  ecalGeometry_=0;
}

EcalGeometryProvider::~EcalGeometryProvider() {
  if (ecalGeometry_) delete ecalGeometry_;
  ecalGeometry_=0;
}

std::pair<const ConditionsObject*,ConditionsIOV> EcalGeometryProvider::getCondition(const EventHeader& context) {
  static const std::string KEYNAME("detectors_valid");
	
  if (!ecalGeometry_) {

    Parameters phex=(params_.exists("EcalHexReadout"))?(params_.getParameter<Parameters>("EcalHexReadout")):(params_);
    
    // search through the subtrees
    for (auto key: phex.keys()) {
      Parameters pver=phex.getParameter<Parameters>(key);
      
      if (!pver.exists(KEYNAME)) {
        ldmx_log(warn) << "No parameter " << KEYNAME << " found in " << key;
        // log strange situation and continue
        continue;
      }
      
     std::vector<std::string> dets_valid=pver.getParameter<std::vector<std::string> >(KEYNAME);
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
    
  }
    
  return std::make_pair(ecalGeometry_,ConditionsIOV(context.getRun(),context.getRun(),true,true));
}

} // namespace ldmx

DECLARE_CONDITIONS_PROVIDER_NS(ldmx, EcalGeometryProvider);
