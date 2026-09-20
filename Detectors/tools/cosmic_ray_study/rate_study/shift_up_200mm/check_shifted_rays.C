// Independent ROOT check of the +200 mm rate-only placement variation.
// Read unchanged GDML, then apply the saved world-space translation to each active solid.
// This checks ray intersections, not navigation, overlaps or mechanical fit.
#include <TGeoManager.h>
#include <TGeoMatrix.h>
#include <TGeoVolume.h>
#include <TGeoNode.h>
#include <TGeoShape.h>
#include <TSystem.h>
#include <fstream>
#include <sstream>
#include <map>
#include <array>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
struct ShiftPart { TGeoVolume* volume; TGeoHMatrix matrix; int layer; };
void walkShift(TGeoVolume* volume, TGeoHMatrix matrix, std::string path,
               const std::map<std::string,int>& layers,
               const std::map<std::string,std::array<double,3>>& moving, std::vector<ShiftPart>& parts,
               int& translated) {
  auto found = layers.find(path);
  if (found != layers.end()) {
    TGeoHMatrix placed(matrix);
    if (moving.count(path)) {
      const double* t = matrix.GetTranslation();
      const auto& shift=moving.at(path);
      double position[3] = {t[0]+shift[0], t[1]+shift[1], t[2]+shift[2]};
      placed.SetTranslation(position);
      ++translated;
    }
    parts.push_back({volume, placed, found->second});
  }
  for (int i=0; i<volume->GetNdaughters(); ++i) {
    auto* node = volume->GetNode(i);
    TGeoHMatrix child(matrix); child.Multiply(node->GetMatrix());
    walkShift(node->GetVolume(), child, path+"/"+node->GetName(),
              layers, moving, parts, translated);
  }
}
void check_shifted_rays(const char* gdml, const char* folder) {
  TGeoManager::SetDefaultUnits(TGeoManager::kG4Units);
  auto* manager=TGeoManager::Import(gdml);
  if (!manager) {gSystem->Exit(2); return;}
  std::string base=folder, line;
  std::map<std::string,int> layers;
  std::ifstream paths(base+"/active_paths.tsv");
  while (std::getline(paths,line)) {
    auto tab=line.find('\t'); layers[line.substr(tab+1)]=std::stoi(line.substr(0,tab));
  }
  // Some LYSO placements reuse a GDML node name; each receives the same shift.
  std::map<std::string,std::array<double,3>> moving;
  int requested=0;
  std::ifstream shifted(base+"/active_translations.tsv");
  while (std::getline(shifted,line)) {
    auto tab=line.find('\t'); std::istringstream values(line.substr(tab+1));
    std::array<double,3> delta; values>>delta[0]>>delta[1]>>delta[2];
    moving[line.substr(0,tab)]=delta; ++requested;
  }
  std::vector<ShiftPart> parts; TGeoHMatrix identity; int translated=0;
  walkShift(manager->GetTopVolume(),identity,manager->GetTopVolume()->GetName(),
            layers,moving,parts,translated);
  std::ifstream rays_in(base+"/root_test_rays.csv");
  int rays=0, mismatches=0;
  while (std::getline(rays_in,line)) {
    std::replace(line.begin(),line.end(),',',' '); std::istringstream input(line);
    double p[3],d[3]; unsigned expected,actual=0;
    input>>p[0]>>p[1]>>p[2]>>d[0]>>d[1]>>d[2]>>expected;
    for (auto& part:parts) {
      double q[3],v[3]; part.matrix.MasterToLocal(p,q); part.matrix.MasterToLocalVect(d,v);
      auto* shape=part.volume->GetShape();
      double t=shape->Contains(q)?0:shape->DistFromOutside(q,v,3);
      if (!std::isfinite(t)||t<0||t>1e5) continue;
      double inside[3]; for(int k=0;k<3;++k) inside[k]=q[k]+(t+1e-7)*v[k];
      if (!shape->Contains(inside)) continue;
      double length=shape->DistFromInside(inside,v,3)+1e-7;
      if(std::isfinite(length)&&length>0&&length<1e5) actual|=(1u<<part.layer);
    }
    if(actual!=expected) ++mismatches;
    ++rays;
  }
  std::ofstream result(base+"/root_ray_check.json");
  result<<"{\"rays\":"<<rays<<",\"layer_decisions\":"<<rays*26
        <<",\"mismatches\":"<<mismatches<<",\"active_volumes\":"<<parts.size()
        <<",\"translated_active_volumes\":"<<translated<<"}\n";
  result.close();
  std::cout<<"Rays "<<rays<<", mismatches "<<mismatches<<", active "<<parts.size()<<", translated "<<translated<<", requested "<<requested<<std::endl;
  if (mismatches||parts.size()!=217||translated!=requested||rays!=192) gSystem->Exit(3);
}
