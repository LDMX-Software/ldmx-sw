#include "TSystem.h"
#include <TGeoManager.h>
void gdml_to_root_export()
{
    // Loading the library and geometry                                                                                                                                                                                                                                         
    TSystem *gSystem = new TSystem();
    gSystem->Load("libGeom");

    gGeoManager->Import("fulldetector.gdml");
    gGeoManager->Export("fulldetector.root");
}
