/**
 * @file PluginFactory.h
 * @brief Class which provides a singleton module factory that creates EventProcessor objects
 * @author Jeremy Mans, University of Minnesota
 */

#ifndef FRAMEWORK_EVENTPROCESSORFACTORY_H_
#define FRAMEWORK_EVENTPROCESSORFACTORY_H_

// LDMX
#include "Framework/EventProcessor.h"

// STL
#include <map>
#include <set>
#include <vector>

namespace ldmx {

    /**
     * @class PluginFactory
     * @brief Singleton module factory that creates EventProcessor objects.
     */
    class PluginFactory {

        public:

            /**
             * Get the factory instance.
             * @return The factory.
             */
            static PluginFactory& getInstance() {
                return theFactory_;
            }

            /**
             * Register the event processor.
             * @param classname The name of the class associated with processor.
             * @param classtype The type of class associated with processor.
             * @param maker TODO.
             */
            void registerEventProcessor(const std::string& classname, int classtype, EventProcessorMaker* maker);

            /**
             * Get the classes associated with the processor.
             * @return a vector of strings corresponding to processor classes.
             */
            std::vector<std::string> getEventProcessorClasses() const;

            /**
             * Get the class type for the event processor.
             * @param TODO.
             */
            int getEventProcessorClasstype(const std::string&) const;

            /**
             * Make an event processor.
             * @param classname Class name of event processor.
             * @param moduleInstanceName TODO.
             * @param process The process type to create.
             */
            EventProcessor* createEventProcessor(const std::string& classname, const std::string& moduleInstanceName, Process& process);

            /**
             * Load a library.
             * @param libname The library to load.
             */
            void loadLibrary(const std::string& libname);

        private:

            /**
             * Constructor
             */
            PluginFactory();

            /**
             * @struct EventProcessorInfo
             * @brief Processor info container to hold classname, class type and maker.
             */
            struct EventProcessorInfo {
                    std::string classname;
                    int classtype;
                    EventProcessorMaker* maker;
            };

            /** A map of names to processor containers. */
            std::map<std::string, EventProcessorInfo> moduleInfo_;

            /** A set of names of loaded libraries. */
            std::set<std::string> librariesLoaded_;

            /** Factor for creating the EventProcessor object. */
            static PluginFactory theFactory_;
    };

}

#endif
