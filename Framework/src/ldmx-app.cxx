#include "Framework/Process.h"
#include "Framework/EventProcessorFactory.h"
#include "Framework/ConfigurePython.h"
#include <iostream>
#include <string.h>

using namespace ldmxsw;

int main(int argc, char* argv[]) {

  if (argc<2) {
    printf("Usage: ldmx-app [application arguments] {configuration_script.py} [arguments to configuration script]\n");
    return 0;
  }

  int ptrpy=1;
  for (ptrpy=1; ptrpy<argc; ptrpy++) {
    if (strstr(argv[ptrpy],".py")) break;
  }

  if (ptrpy==argc) {
    printf("Usage: ldmx-app [application arguments] {configuration_script.py} [arguments to configuration script]\n");
    printf("  ** No python script provided. **\n");
    return 0;
  }

  
  
  Process* p{0};
  try {
    std::cout << "---- LDMXSW: Loading configuration --------" << std::endl;
    ConfigurePython cfg(argv[ptrpy],argv+ptrpy+1,argc-ptrpy);
    p=cfg.makeProcess();
    std::cout << "---- LDMXSW: Configuration load complete  --------" << std::endl;
    
    std::cout << "---- LDMXSW: Starting event processing --------" << std::endl;
    p->run(cfg.eventLimit());
    std::cout << "---- LDMXSW: Event processing complete  --------" << std::endl;

    
  } catch (Exception& e) {
    std::cerr << "Framework Error [" << e.name() << "] : " << e.message() << std::endl;
    std::cerr << "  at " << e.module() <<":"<<e.line()<<" in " <<e.function() << std::endl;
  }
  
  
  
  return 0;
}
