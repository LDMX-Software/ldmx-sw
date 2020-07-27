#include "Framework/ConditionsObjectProvider.h"

// LDMX
#include "Framework/Process.h"
#include "Framework/PluginFactory.h"

namespace ldmx {
    
    ConditionsObjectProvider::ConditionsObjectProvider(const std::string& name, const std::string& tagname, const Parameters& params, Process& process) :
	process_{process}, name_ {name}, tagname_{tagname}, theLog_{logging::makeLogger(name)} {
	}
    
    void ConditionsObjectProvider::declare(const std::string& classname, ConditionsObjectProviderMaker* maker) {

      PluginFactory::getInstance().registerConditionsObjectProvider(classname, CLASSTYPE, maker);
    }

}
