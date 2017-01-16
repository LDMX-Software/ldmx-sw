#include "Framework/EventProcessor.h"
#include <iostream>

class DummyAnalyzer : public ldmxsw::Analyzer {
public:
  DummyAnalyzer(const std::string& name, const ldmxsw::Process& process) : ldmxsw::Analyzer(name,process) { }
  
  virtual void analyze(const event::Event& event) {
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

DECLARE_ANALYZER(DummyAnalyzer);
