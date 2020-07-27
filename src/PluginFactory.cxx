#include "Framework/EventProcessor.h"
#include "Framework/PluginFactory.h"
#include <dlfcn.h>

ldmx::PluginFactory ldmx::PluginFactory::theFactory_ __attribute((init_priority(500)));

namespace ldmx {

    PluginFactory::PluginFactory() {
    }

    void PluginFactory::registerEventProcessor(const std::string& classname, int classtype, EventProcessorMaker* maker) {
        auto ptr = moduleInfo_.find(classname);
        if (ptr != moduleInfo_.end()) {
            EXCEPTION_RAISE("ExistingEventProcessorDefinition", "Already have a module registered with the classname '" + classname + "'");
        }
        EventProcessorInfo mi;
        mi.classname = classname;
        mi.classtype = classtype;
        mi.maker = maker;
        moduleInfo_[classname] = mi;
    }

    std::vector<std::string> PluginFactory::getEventProcessorClasses() const {
        std::vector<std::string> classes;
        for (auto ptr : moduleInfo_) {
            classes.push_back(ptr.first);
        }
        return classes;
    }

    int PluginFactory::getEventProcessorClasstype(const std::string& ct) const {
        auto ptr = moduleInfo_.find(ct);
        if (ptr == moduleInfo_.end()) {
            return 0;

        } else {
            return ptr->second.classtype;
        }
    }

    EventProcessor* PluginFactory::createEventProcessor(const std::string& classname, const std::string& moduleInstanceName, Process& process) {
        auto ptr = moduleInfo_.find(classname);
        if (ptr == moduleInfo_.end()) {
            return 0;
        }
        return ptr->second.maker(moduleInstanceName, process);
    }

    void PluginFactory::loadLibrary(const std::string& libname) {
        if (librariesLoaded_.find(libname) != librariesLoaded_.end()) {
            return; // already loaded
        }

        void* handle = dlopen(libname.c_str(), RTLD_NOW);
        if (handle == nullptr) {
            EXCEPTION_RAISE("LibraryLoadFailure", "Error loading library '" + libname + "':" + dlerror());
        }

        librariesLoaded_.insert(libname);
    }

}
