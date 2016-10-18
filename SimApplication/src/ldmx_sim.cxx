#include "SimApplication/SimApplication.h"

#include <stdio.h>

using sim::SimApplication;

int main(int argc, char** argv)  {
    SimApplication* app = new SimApplication();
    app->run(argc, argv);
    delete app;
    return 0;
}
