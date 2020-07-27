/**
 * @file ConditionsIOV.h
 * @brief Interval-of-validity object for conditions information
 * @author Jeremy Mans, University of Minnesota
 */

#ifndef FRAMEWORK_CONDITIONSIOV_H_
#define FRAMEWORK_CONDITIONSIOV_H_

/*~~~~~~~~~~~*/
/*   Event   */
/*~~~~~~~~~~~*/
#include "Framework/Exception.h"
#include "Event/EventHeader.h"

namespace ldmx {

    /** 
     * @class ConditionsIOV
     *
     * @brief Class which defines the run/event/type range for which a given condition is valid, including for all time
     */
    class ConditionsIOV {
	public:

	/** 
	 * Constructor for null validity
	 */
	ConditionsIOV() : firstRun_{0}, lastRun_{0}, validForData_{false}, validForMC_{false} {
	}

	/**
	 * Constructor for a unlimited validity
	 */
	ConditionsIOV(bool validForData, bool validForMC) : firstRun_(-1), lastRun_(-1), validForData_{validForData}, validForMC_{validForMC} {
	}

	
	/**
	 * Constructor for a run-limited validity
	 * @arg firstRun should be -1 if valid from beginning of time
	 * @arg lastRun should be -1 if valid to end of time
	 */
	ConditionsIOV(int firstRun, int lastRun, bool validForData=true, bool validForMC=true) : firstRun_(firstRun), lastRun_(lastRun), validForData_{validForData}, validForMC_{validForMC} {
	}

	/** Checks to see if this condition is valid for the given event using information from the header */
	bool validForEvent(const EventHeader& eh) const {
	    return (eh.getRun()>firstRun_) && (eh.getRun()<=lastRun_ || lastRun_==-1) && ((eh.isRealData())?(validForData_):(validForMC_));
	}

	/** 
	 * Print the object
	 */
	void Print() const;

	/** 
	 * Print the object
	 */
	std::string ToString() const;
	
	private:
	/** First run for which this condition is valid */
	int firstRun_;

	/** Last run for which this condition is valid or -1 for infinite validity */
	int lastRun_;

	/** Is this Condition valid for real data? */
	bool validForData_;
	
	/** Is this Condition valid for simulation? */
	bool validForMC_;
    };
}

#endif // FRAMEWORK_CONDITIONSIOV_H_
