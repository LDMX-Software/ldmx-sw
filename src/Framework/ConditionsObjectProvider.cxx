#include "Framework/ConditionsObjectProvider.h"

// LDMX
#include "Framework/Process.h"
#include "Framework/PluginFactory.h"

namespace ldmx {
    
ConditionsObjectProvider::ConditionsObjectProvider(const std::string& objname, const std::string& tagname, const Parameters& params, Process& process) :
    process_{process}, objectName_ {objname}, tagname_{tagname}, theLog_{logging::makeLogger(objname)} { }


std::pair<const ConditionsObject*,ConditionsIOV> ConditionsObjectProvider::requestParentCondition(const std::string& name, const EventHeader& context) {
  const ConditionsObject* obj=process_.getConditions().getConditionPtr(name);
  ConditionsIOV iov=process_.getConditions().getConditionIOV(name);
  return std::make_pair(obj,iov);
}


void ConditionsObjectProvider::declare(const std::string& classname, ConditionsObjectProviderMaker* maker) {
  PluginFactory::getInstance().registerConditionsObjectProvider(classname, CLASSTYPE, maker);
}

}
