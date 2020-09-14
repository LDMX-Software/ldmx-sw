/**
 * @file EcalGeometryProvider.h
 * @brief Class that creates EcalHexReadout and EcalTriggerGeometry objects based on python specifications
 * @author Jeremiah Mans, UMN
 */

#ifndef ECAL_ECALGEOMETRYPROVIDER_H_
#define ECAL_ECALGEOMETRYPROVIDER_H_

// LDMX
#include "DetDescr/EcalHexReadout.h"
//#include "Ecal/EcalTriggerGeometry.h"
#include "Framework/ConditionsObjectProvider.h"

namespace ldmx {

    /**
     * @class EcalGeometryProvider
     * @brief Common, singleton source for EcalHexReadout and EcalTriggerGeometry
     */
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
	virtual std::pair<const ConditionsObject*,ConditionsIOV> getCondition(const std::string& condition_name, const EventHeader& context, const RunHeader& runcontext);

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
	//	EcalTriggerGeometry* ecalTriggerGeometry_;
	
    };
    
}

    
#endif // ECAL_ECALGEOMETRYPROVIDER_H_
