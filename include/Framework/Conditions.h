/**
 * @file Conditions.h
 * @brief Container and caching class for conditions information
 * @author Jeremy Mans, University of Minnesota
 */

#ifndef FRAMEWORK_CONDITIONS_H_
#define FRAMEWORK_CONDITIONS_H_

/*~~~~~~~~~~~*/
/*   Event   */
/*~~~~~~~~~~~*/
#include "Framework/Exception.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/ConditionsIOV.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <map>
#include <vector>

namespace ldmx {

    class Process;
    class ConditionsObjectProvider;
    class ConditionsObject;
    class EventHeader;
    class RunHeader;
    class Parameters;

    /**
     * @class Conditions
     * @brief Container and cache for conditions and conditions providers
     */
    class Conditions {

        public:

            /**
             * Constructor
             */
            Conditions(Process&);

            /**
             * Class destructor.
             */
            ~Conditions() {;}

            /**
             * Primary request action for a conditions object If the
             * object is in the cache and still valid (IOV), the
             * cached object will be returned.  If it is not in the cache, 
             * or is out of date, the () method will be called to provide the 
             * object.
             */
            template <class T>
            const T& getCondition(const std::string& condition_name) {
	      const ConditionsObject* obj=getConditionInternal(condition_name, getEventHeader(), getRunHeader());
                if (!obj) {
                    EXCEPTION_RAISE("ConditionUnavailableException",
                            std::string("Requested condition not available for unspecific reason: ")+condition_name);
                }
                return dynamic_cast<const T&>(*obj);
            }

            /**
             * Calls onProcessStart for all ConditionsObjectProviders
             */
            void onProcessStart();

            /**
             * Calls onProcessEnd for all ConditionsObjectProviders
             */
            void onProcessEnd();


            /** 
             * Create a ConditionsObjectProvider given the information
             */
            void createConditionsObjectProvider(const std::string& classname, const std::string& instancename, const std::string& tagname, const Parameters& params);
        
       private:

            /** Cache-managing method */
            const ConditionsObject* getConditionInternal(const std::string& condition_name, const EventHeader& context, const RunHeader& run_context);

	    /** Get the event header from the process */
            const EventHeader& getEventHeader() const;

	    /** Get the run header from the process */
            const RunHeader& getRunHeader() const;
		
            /** Handle to the Process. */
            Process& process_;

            /** Set of conditions object providers */
            std::vector<ConditionsObjectProvider*> providers_;
    
            /** Map of who provides which condition */
            std::map<std::string, ConditionsObjectProvider*> providerMap_;
    
            /**
             * An entry to store an already loaded conditions object
             */
            struct CacheEntry {
                ConditionsIOV iov;
                ConditionsObjectProvider* provider;
                const ConditionsObject* obj;
            };
    
            /** Conditions cache */
            std::map<std::string,CacheEntry> cache_;
    };

}

#endif
