#include "SimApplication/SimApplication.h"
#include "Framework/EventProcessorFactory.h"

#include <stdio.h>

// this static instance must be here so that it comes into existance before any shared library is loaded

using sim::SimApplication;

int main(int argc, char** argv)  {
    SimApplication* app = new SimApplication();
    app->run(argc, argv);
    delete app;
    return 0;
}
