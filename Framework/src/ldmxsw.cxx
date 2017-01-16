#include "Framework/Process.h"
#include "Framework/EventProcessorFactory.h"
#include "Framework/ConfigurePython.h"
#include <iostream>

using namespace ldmxsw;
// this static instance must be here so that it comes into existance before any shared library is loaded
EventProcessorFactory EventProcessorFactory::theFactory_;

int main(int argc, char* argv[]) {

  Process* p{0};
  try {
    std::cout << "---- LDMXSW: Loading configuration --------" << std::endl;
    ConfigurePython cfg(argv[1]);
    p=cfg.makeProcess();
    std::cout << "---- LDMXSW: Configuration load complete  --------" << std::endl;
    
    p->setOutputFileTemplate("test.root");
    std::cout << "---- LDMXSW: Starting event processing --------" << std::endl;
    p->run(cfg.eventLimit());
    std::cout << "---- LDMXSW: Event processing complete  --------" << std::endl;

    
  } catch (Exception& e) {
    std::cerr << "Framework Error [" << e.name() << "] : " << e.message() << std::endl;
    std::cerr << "  at " << e.module() <<":"<<e.line()<<" in " <<e.function() << std::endl;
  }
  
  
  
  return 0;
}
