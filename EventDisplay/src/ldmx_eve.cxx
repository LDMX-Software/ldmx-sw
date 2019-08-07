/**
 * @file ldmx_eve.cxx
 * @author Josh Hiltbrand, University of Minnesota
 */

// LDMX
#include "EventDisplay/EventDisplay.h"

#include <string>

using ldmx::EventDisplay;

/**
 * @app ldmx-eve
 * @brief ldmx event display
 *
 * Transfers the input ROOT file to the EventDisplay class to manage and draw.
 */
int main(int argc, char** argv) {

    if (argc != 2) {
        std::cout << "The event display requires 1 argument!" << std::endl;
        std::cout << "The argument is the ROOT file containing the event to be visualized." << std::endl;
        return -1;
    }

    const char* file = argv[1];

    TRint *app = new TRint("App", &argc, argv);

    TEveManager* manager = new TEveManager(1600, 1200, kTRUE, "FV");
    TEveUtil::SetupEnvironment();
    TEveUtil::SetupGUI();

    TEveBrowser* browser = manager->GetBrowser();
    browser->StartEmbedding(TRootBrowser::kLeft);

    EventDisplay *display = new EventDisplay(manager);
    if (!display->SetFile(file)) { 
        app->Terminate(0);
        delete browser;
        delete display;
        delete manager;

        return -1;
    }

    browser->SetTabTitle("Event Control", 0);
    browser->StopEmbedding();

    app->Run(kFALSE);
    app->Terminate(0);

    delete browser;
    delete display;
    delete manager;

    return 0;
} 
