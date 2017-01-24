/**
 * @file DummyAnalyzer.cxx
 * @brief Class that defines a dummy Analyzer implementation that just prints some messages
 * @author Jeremy Mans, University of Minnesota
 */

#include "Framework/EventProcessor.h"
#include <iostream>

namespace ldmx {

/**
 * @class DummyAnalyzer
 * @brief A dummy Analyzer implementation that just prints some messages
 */
class DummyAnalyzer : public Analyzer {

    public:

        DummyAnalyzer(const std::string& name, const Process& process) :
                Analyzer(name, process) {
        }

        virtual void analyze(const Event& event) {
            std::cout << "DummyAnalyzer: Analyzing an event!" << std::endl;
        }

        virtual void onFileOpen() {
            std::cout << "DummyAnalyzer: Opening a file!" << std::endl;
        }

        virtual void onFileClose() {
            std::cout << "DummyAnalyzer: Closing a file!" << std::endl;
        }

        virtual void onProcessStart() {
            std::cout << "DummyAnalyzer: Starting processing!" << std::endl;
        }

        virtual void onProcessEnd() {
            std::cout << "DummyAnalyzer: Finishing processing!" << std::endl;
        }
};
}

DECLARE_ANALYZER_NS(ldmx, DummyAnalyzer);
