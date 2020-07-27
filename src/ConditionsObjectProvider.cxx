#include "Framework/ConditionsObjectProvider.h"

// LDMX
#include "Framework/Process.h"
#include "Framework/PluginFactory.h"

namespace ldmx {
    
	ConditionsObjectProvider::ConditionsObjectProvider(const std::string& name, const Parameters& params, Process& process) :
	process_{process}, name_ {name} , theLog_{logging::makeLogger(name)} {
	}
    
    void ConditionsObjectProvider::declare(const std::string& classname, ConditionsObjectProviderMaker* maker) {

      PluginFactory::getInstance().registerConditionsObjectProvider(classname, CLASSTYPE, maker);
    }

}
