#include "Framework/ConditionsObjectProvider.h"

// LDMX
#include "Framework/PluginFactory.h"
#include "Framework/Process.h"

namespace framework {

ConditionsObjectProvider::ConditionsObjectProvider(
    const std::string& objname, const std::string& tagname,
    const framework::config::Parameters& params, Process& process)
    : process_{process},
      objectName_{objname},
      tagname_{tagname},
      theLog_{logging::makeLogger(objname)} {}

std::pair<const ConditionsObject*, ConditionsIOV>
ConditionsObjectProvider::requestParentCondition(
    const std::string& name, const ldmx::EventHeader& context) {
  const ConditionsObject* obj = process_.getConditions().getConditionPtr(name);
  ConditionsIOV iov = process_.getConditions().getConditionIOV(name);
  return std::make_pair(obj, iov);
}

void ConditionsObjectProvider::declare(const std::string& classname,
                                       ConditionsObjectProviderMaker* maker) {
  PluginFactory::getInstance().registerConditionsObjectProvider(
      classname, CLASSTYPE, maker);
}

}  // namespace framework
