#include "Framework/PluginFactory.h"

#include <dlfcn.h>

#include "Framework/EventProcessor.h"

framework::PluginFactory framework::PluginFactory::theFactory_
    __attribute((init_priority(500)));

namespace framework {

PluginFactory::PluginFactory() {}

void PluginFactory::registerEventProcessor(const std::string& classname,
                                           int classtype,
                                           EventProcessorMaker* maker) {
  auto ptr = moduleInfo_.find(classname);
  if (ptr != moduleInfo_.end()) {
    EXCEPTION_RAISE("ExistingEventProcessorDefinition",
                    "Already have a module registered with the classname '" +
                        classname + "'");
  }
  PluginInfo mi;
  mi.classname = classname;
  mi.classtype = classtype;
  mi.ep_maker = maker;
  mi.cop_maker = 0;
  moduleInfo_[classname] = mi;
}

void PluginFactory::registerConditionsObjectProvider(
    const std::string& classname, int classtype,
    ConditionsObjectProviderMaker* maker) {
  auto ptr = moduleInfo_.find(classname);
  if (ptr != moduleInfo_.end()) {
    EXCEPTION_RAISE("ExistingEventProcessorDefinition",
                    "Already have a module registered with the classname '" +
                        classname + "'");
  }
  PluginInfo mi;
  mi.classname = classname;
  mi.classtype = classtype;
  mi.cop_maker = maker;
  mi.ep_maker = 0;
  moduleInfo_[classname] = mi;
}

std::vector<std::string> PluginFactory::getEventProcessorClasses() const {
  std::vector<std::string> classes;
  for (auto ptr : moduleInfo_) {
    if (ptr.second.ep_maker != 0) classes.push_back(ptr.first);
  }
  return classes;
}

int PluginFactory::getEventProcessorClasstype(const std::string& ct) const {
  auto ptr = moduleInfo_.find(ct);
  if (ptr == moduleInfo_.end() || ptr->second.ep_maker == 0) {
    return 0;

  } else {
    return ptr->second.classtype;
  }
}

EventProcessor* PluginFactory::createEventProcessor(
    const std::string& classname, const std::string& moduleInstanceName,
    Process& process) {
  auto ptr = moduleInfo_.find(classname);
  if (ptr == moduleInfo_.end() || ptr->second.ep_maker == 0) {
    return 0;
  }
  return ptr->second.ep_maker(moduleInstanceName, process);
}

ConditionsObjectProvider* PluginFactory::createConditionsObjectProvider(
    const std::string& classname, const std::string& objName,
    const std::string& tagname, const framework::config::Parameters& params,
    Process& process) {
  auto ptr = moduleInfo_.find(classname);
  if (ptr == moduleInfo_.end() || ptr->second.cop_maker == 0) {
    return 0;
  }
  return ptr->second.cop_maker(objName, tagname, params, process);
}

void PluginFactory::loadLibrary(const std::string& libname) {
  if (librariesLoaded_.find(libname) != librariesLoaded_.end()) {
    return;  // already loaded
  }

  void* handle = dlopen(libname.c_str(), RTLD_NOW);
  if (handle == nullptr) {
    EXCEPTION_RAISE("LibraryLoadFailure",
                    "Error loading library '" + libname + "':" + dlerror());
  }

  librariesLoaded_.insert(libname);
}

}  // namespace framework
