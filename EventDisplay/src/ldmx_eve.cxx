// LDMX
#include "EventDisplay/EventDisplay.h"

// ROOT
#include "TRint.h"

// TEVE
#include "TEveBrowser.h"

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

    TEveManager::Create();

    TEveBrowser* browser = gEve->GetBrowser();
    browser->StartEmbedding(TRootBrowser::kLeft);

    gEve->AddEvent(new TEveEventManager("LDMX Detector", ""));
    EventDisplay *display = new EventDisplay();
    gEve->AddEvent(new TEveEventManager("LDMX Event", ""));

    browser->StopEmbedding();
    browser->SetTabTitle("Event Control", 0);

    if (!display->SetFile(file)) { 
        app->Terminate(0);
        delete browser;
        delete display;

        return -1;
    }

    app->Run(kFALSE);
    app->Terminate(0);

    delete browser;
    delete display;

    return 0;
} 
