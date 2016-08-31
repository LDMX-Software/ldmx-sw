#include "SimApplication/SimApplication.h"

#include <stdio.h>

int main(int, const char* argv[])  {
    printf("Hello LDMX Sim Application!\n");
    SimApplication* app = new SimApplication();
    app->run(argv);
    delete app;
    printf("Bye LDMX Sim Application!\n");
    return 0;
}
