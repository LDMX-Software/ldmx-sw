#include "SimApplication/SimApplication.h"
#include "Exception/Exception.h"

#include <stdio.h>
#include <iostream>

using ldmx::SimApplication;

int main(int argc, char** argv) {
    try {
        SimApplication* app = new SimApplication();
        app->run(argc, argv);
        delete app;
    } catch ( ldmx::Exception& e ) {
        std::cerr << "Simulation Error [" << e.name() << "] : " << e.message() << std::endl;
        std::cerr << "  at " << e.module() << ":" << e.line() << " in " << e.function() << std::endl;
    }
    return 0;
}
