// ROOT import and tessellated closure validation; not a Geant4 transport check.
// Run through check.py; it supplies each GDML entry point and a JSON output
// path.
#include <TGeoManager.h>
#include <TGeoTessellated.h>
#include <TGeoVolume.h>
#include <TROOT.h>
#include <TSystem.h>

#include <fstream>
#include <iostream>
#include <string>
void validate_framework(const char* gdml,
                        const char* output = "validation_root.json") {
  TGeoManager::SetDefaultUnits(TGeoManager::kG4Units);
  auto* geo = TGeoManager::Import(gdml);
  if (!geo || !geo->GetTopVolume()) {
    std::cerr << "GDML import failed\n";
    gSystem->Exit(2);
    return;
  }
  int tessellated = 0, facets = 0, vertices = 0, unclosed = 0;
  TIter next(geo->GetListOfShapes());
  while (auto* obj = next()) {
    auto* shape = dynamic_cast<TGeoTessellated*>(obj);
    if (!shape) continue;
    ++tessellated;
    facets += shape->GetNfacets();
    vertices += shape->GetNvertices();
    if (!shape->CheckClosure(false, false)) {
      ++unclosed;
      std::cerr << "Unclosed shape: " << shape->GetName() << '\n';
    }
  }
  std::ofstream out(output);
  out << "{\n  \"validator\": \"ROOT TGeo GDML import and tessellated "
         "CheckClosure(false,false)\",\n"
      << "  \"root_version\": \"" << gROOT->GetVersion() << "\",\n"
      << "  \"world\": \"" << geo->GetTopVolume()->GetName() << "\",\n"
      << "  \"volumes\": " << geo->GetListOfVolumes()->GetEntries() << ",\n"
      << "  \"world_daughters\": " << geo->GetTopVolume()->GetNdaughters()
      << ",\n"
      << "  \"tessellated_solids\": " << tessellated << ",\n"
      << "  \"facets\": " << facets << ",\n"
      << "  \"vertices\": " << vertices << ",\n"
      << "  \"unclosed_solids\": " << unclosed << ",\n"
      << "  \"geant4_transport_tested\": false,\n"
      << "  \"physical_materials_verified\": false,\n"
      << "  \"inter_part_overlaps_tested\": false\n}\n";
  out.close();
  std::cout << "ROOT_IMPORT_RESULT tessellated=" << tessellated
            << " facets=" << facets << " unclosed=" << unclosed << '\n';
  if (unclosed) gSystem->Exit(3);
  gSystem->Exit(0);
}
