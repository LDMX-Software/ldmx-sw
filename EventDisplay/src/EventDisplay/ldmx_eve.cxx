/**
 * @file ldmx_eve.cxx
 * @author Josh Hiltbrand, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

// LDMX
#include "EventDisplay/Display.h"

#include <memory>
#include <string>

/**
 * Print Help Message for ldmx-eve
 */
void printHelp();

/**
 * @app ldmx-eve
 * @brief ldmx event display
 *
 * Transfers the input ROOT file to the EventDisplay class to manage and draw.
 */
int main(int argc, char** argv) {
  // Parse command line options
  bool verbose = false;
  std::string file;
  for (int iArg = 1; iArg < argc; iArg++) {
    if (strcmp(argv[iArg], "--help") == 0 or strcmp(argv[iArg], "-h") == 0) {
      printHelp();
      return 0;
    } else if (strcmp(argv[iArg], "--verbose") == 0 or
               strcmp(argv[iArg], "-v") == 0) {
      verbose = true;
    } else if (file.empty()) {
      file = argv[iArg];
    } else {
      printHelp();
      return 1;
    }
  }

  if (verbose) {
    std::cout << "[ ldmx-eve ] : Starting up ROOT app, manager, and browser."
              << std::endl;
  }

  int* dummyArgC;
  char** dummyArgV;
  TRint* app = new TRint("app", dummyArgC, dummyArgV, 0, 0, true);
  app->SetPrompt("");  // no root[#] at beginning of each line

  TEveManager* manager = TEveManager::Create(kTRUE, "FV");

  TEveBrowser* browser = manager->GetBrowser();
  browser->StartEmbedding(TRootBrowser::kLeft);

  if (verbose) {
    std::cout << "[ ldmx-eve ] : Constructing Event Display window and drawing "
                 "geometry."
              << std::endl;
  }

  eventdisplay::Display display(manager, verbose);

  if (verbose) {
    std::cout << "[ ldmx-eve ] : Opening file " << file << std::endl;
  }

  if (!display.SetFile(file)) {
    std::cerr << "[ ldmx-eve ] : Unable to open file! Exiting..." << std::endl;

    app->Terminate(1);
    manager->Terminate();
    browser->ReallyDelete();

    return 1;
  }

  browser->SetTabTitle("Event Control", 0);
  browser->StopEmbedding();

  if (verbose) {
    std::cout << "[ ldmx-eve ] : Event display window has been opened. Enter "
                 "'.q' to quit the root prompt."
              << std::endl;
  }

  // TApplication::Terminate is called when quitting root
  //  ROOT owns the hanging pointers and seems to be cleaning them up during
  //  quit
  app->Run(kFALSE);

  return 0;
}

void printHelp() {
  printf("Usage: ldmx-eve [-h,--help] [-v,--verbose] eventFilePath\n");
  printf("   -h,--help       : Print this help message and exit\n");
  printf("   -v,--verbose    : Print debug messages to standard output\n");
  printf("   eventFilePath   : Event file to use in display\n");
  printf("                     Currently, only one input file is supported.\n");
  printf(
      "  NOTE: You still need to enter '.q' to quit root after closing the "
      "display window.\n");

  return;
}
