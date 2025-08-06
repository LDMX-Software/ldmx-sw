#include "Framework/Conditions.h"

#include <sstream>

#include "Framework/ConditionsObjectProvider.h"
#include "Framework/Process.h"

namespace framework {

Conditions::Conditions(Process& p) : process_{p} {}

void Conditions::createConditionsObjectProvider(
    const std::string& classname, const std::string& objname,
    const std::string& tagname, const framework::config::Parameters& params) {
  auto cop = ConditionsObjectProvider::Factory::get().make(
      classname, objname, tagname, params, process_);
  if (not cop) {
    EXCEPTION_RAISE("UnableToCreate",
                    "No ConditionsObjectProvider for " + classname);
  }

  std::string provides = cop.value()->getConditionObjectName();
  if (provider_map_.find(provides) != provider_map_.end()) {
    EXCEPTION_RAISE(
        "ConditionAmbiguityException",
        "Multiple ConditonsObjectProviders configured to provide " + provides);
  }
  provider_map_[provides] = cop.value();
}

void Conditions::onProcessStart() {
  for (auto ptr : provider_map_) ptr.second->onProcessStart();
}

void Conditions::onProcessEnd() {
  for (auto ptr : provider_map_) ptr.second->onProcessEnd();
}

void Conditions::onNewRun(ldmx::RunHeader& rh) {
  for (auto ptr : provider_map_) ptr.second->onNewRun(rh);
}

ConditionsIOV Conditions::getConditionIOV(
    const std::string& condition_name) const {
  auto cacheptr = cache_.find(condition_name);
  if (cacheptr == cache_.end()) {
    return ConditionsIOV();
  }
  else {
    return cacheptr->second.iov_;
  }
}

const ConditionsObject* Conditions::getConditionPtr(
    const std::string& condition_name) {
  const ldmx::EventHeader& context = *(process_.getEventHeader());
  auto cacheptr = cache_.find(condition_name);

  if (cacheptr == cache_.end()) {
    auto copptr = provider_map_.find(condition_name);

    if (copptr == provider_map_.end()) {
      EXCEPTION_RAISE("ConditionUnavailable",
                      "No provider is available for : " + condition_name);
    }

    std::pair<const ConditionsObject*, ConditionsIOV> cond =
        copptr->second->getCondition(context);

    if (!cond.first) {
      EXCEPTION_RAISE(
          "ConditionUnavailable",
          "Null condition returned for requested item : " + condition_name);
    }

    // first request, create a cache entry
    CacheEntry ce;
    ce.iov_ = cond.second;
    ce.obj_ = cond.first;
    ce.provider_ = copptr->second;
    cache_[condition_name] = ce;
    return ce.obj_;
  } else {
    /// if still valid, we return what we have
    if (cacheptr->second.iov_.validForEvent(context)) {
      return cacheptr->second.obj_;
    }
    else {
      // if not, we release the old object
      cacheptr->second.provider_->releaseConditionsObject(cacheptr->second.obj_);
      // now ask for a new one
      std::pair<const ConditionsObject*, ConditionsIOV> cond =
          cacheptr->second.provider_->getCondition(context);

      if (!cond.first) {
        std::stringstream s;
        s << "Unable to update condition '" << condition_name << "' for event "
          << context.getEventNumber() << " run " << context.getRun();
        if (context.isRealData()){
          s << " DATA";
	}
        else {
          s << " MC";
	}
        EXCEPTION_RAISE("ConditionUnavailable", s.str());
      }
      cacheptr->second.iov_ = cond.second;
      cacheptr->second.obj_ = cond.first;
      return cond.first;
    }
  }
}

}  // namespace framework
