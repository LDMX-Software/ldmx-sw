#include "Framework/Process.h"
#include "Framework/Conditions.h"
#include "Framework/PluginFactory.h"
#include <sstream>

namespace ldmx {

    Conditions::Conditions(Process& p) : process_{p} {
    }
    
    void Conditions::createConditionsObjectProvider(const std::string& classname, const std::string& instancename, const std::string& tagname, const Parameters& params) {
        ConditionsObjectProvider* cop = 
            PluginFactory::getInstance().createConditionsObjectProvider(classname,instancename,tagname,params,process_);
        if (cop) {
            providers_.push_back(cop);
    
            std::vector<std::string> provides=cop->getConditionObjectsAvailable();
            for (auto sptr : provides) {
                if (providerMap_.find(sptr)!=providerMap_.end()) {
                    EXCEPTION_RAISE("ConditionAmbiguityException",
                            std::string("Multiple ConditonsObjectProviders configured to provide ")+sptr);
                }
                providerMap_[sptr]=cop;
            }
        } else {
          EXCEPTION_RAISE("ConditionsException","No ConditionsObjectProvider for "+classname);    
        }
    }

    void Conditions::onProcessStart() {
        for (auto ptr: providers_)
            ptr->onProcessStart();
    }
    
    void Conditions::onProcessEnd() {
        for (auto ptr: providers_)
            ptr->onProcessEnd();
    }


    const EventHeader& Conditions::getEventHeader() const {
	return *(process_.getEventHeader());
    }

    const RunHeader& Conditions::getRunHeader() const {
	return *(process_.getRunHeader());
    }
  

    const ConditionsObject* Conditions::getConditionInternal(const std::string& condition_name, const EventHeader& context, const RunHeader& run_context) {
        auto cacheptr = cache_.find(condition_name);
        
        if (cacheptr == cache_.end()) {
            auto copptr = providerMap_.find(condition_name);
            
            if (copptr==providerMap_.end()) {
                EXCEPTION_RAISE("ConditionUnavailable",std::string("No provider is available for : "+condition_name));
            }
            
            std::pair<const ConditionsObject*,ConditionsIOV> cond=copptr->second->getCondition(condition_name,context,run_context);
            
            if (!cond.first) {
                EXCEPTION_RAISE("ConditionUnavailable",std::string("Null condition returned for requested item : "+condition_name));
            }
            // first request, create a cache entry
            CacheEntry ce;
            ce.iov=cond.second;
            ce.obj=cond.first;
            ce.provider=copptr->second;
            cache_[condition_name]=ce;
        } else {
            /// if still valid, we return what we have
            if (cacheptr->second.iov.validForEvent(context)) return cacheptr->second.obj;
            else {
                // if not, we release the old object
                cacheptr->second.provider->releaseConditionsObject(cacheptr->second.obj);
                // now ask for a new one
                std::pair<const ConditionsObject*,ConditionsIOV> cond=cacheptr->second.provider->getCondition(condition_name,context, run_context);
                
                if (!cond.first) {
                    std::stringstream s;
                    s << "Unable to update condition '" 
                      << condition_name << "' for event " 
                      << context.getEventNumber() << " run " << context.getRun();
                    if (context.isRealData()) s << " DATA";
                    else s << " MC";
                    EXCEPTION_RAISE("ConditionUnavailable",s.str());
                }
                cacheptr->second.iov=cond.second;
                cacheptr->second.obj=cond.first;
                return cond.first;
            }
        }
    }   
}
