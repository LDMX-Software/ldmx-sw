#include "SimApplication/SimApplication.h"

#include <stdio.h>

int main(int argc, char** argv)  {
    SimApplication* app = new SimApplication();
    app->run(argc, argv);
    delete app;
    return 0;
}
