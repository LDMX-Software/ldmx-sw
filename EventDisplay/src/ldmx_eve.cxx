// LDMX
#include "EventDisplay/EventDisplay.h"

#include <string>

using ldmx::EventDisplay;

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
    manager->AddEvent(new TEveEventManager("LDMX Detector", ""));

    EventDisplay *display = new EventDisplay(manager);
    if (!display->SetFile(file)) { 
        app->Terminate(0);
        delete browser;
        delete display;
        delete manager;

        return -1;
    }

    manager->AddEvent(new TEveEventManager("LDMX Event", ""));

    browser->SetTabTitle("Event Control", 0);
    browser->StopEmbedding();

    app->Run(kFALSE);
    app->Terminate(0);

    delete browser;
    delete display;
    delete manager;

    return 0;
} 
