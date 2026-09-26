// Optional ECal centering/removal applied to the original ROOT solids.
// Compare finite-volume coincidence tests with ROOT's original GDML solids.
// Material chords exclude the inactive silicon parent surrounding its active
// daughter, since selected rays cross the daughter and must not double-count it.
#include <TGeoManager.h>
#include <TGeoMatrix.h>
#include <TGeoVolume.h>
#include <TGeoNode.h>
#include <TGeoShape.h>
#include <TGeoMaterial.h>
#include <TSystem.h>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <algorithm>
struct Part { TGeoVolume* v; TGeoHMatrix matrix; std::string path; int layer; };
void walkLayout(TGeoVolume* v,TGeoHMatrix m,std::string path,std::map<std::string,int>& layer,std::vector<Part>& parts,bool removeEcal,double dx,double dy){
 if(removeEcal && path.find("native_ecal")!=std::string::npos)return;
 TGeoHMatrix placed(m);
 if(path.find("native_ecal")!=std::string::npos){const double* t=m.GetTranslation();double q[3]={t[0]+dx,t[1]+dy,t[2]};placed.SetTranslation(q);}
 auto it=layer.find(path); auto mat=std::string(v->GetMaterial()?v->GetMaterial()->GetName():"");
 if(v->GetShape() && mat!="Air" && mat!="Vacuum" && !v->IsAssembly()) parts.push_back({v,placed,path,it==layer.end()?-1:it->second});
 for(int i=0;i<v->GetNdaughters();i++){auto* n=v->GetNode(i);TGeoHMatrix c(m);c.Multiply(n->GetMatrix());walkLayout(n->GetVolume(),c,path+"/"+n->GetName(),layer,parts,removeEcal,dx,dy);}
}
void check_layout(const char* gdml,const char* folder,bool removeEcal=false,double dx=0,double dy=0){
 TGeoManager::SetDefaultUnits(TGeoManager::kG4Units);auto* g=TGeoManager::Import(gdml);if(!g){gSystem->Exit(2);return;}
 std::string base=folder,line;std::map<std::string,int> layer;
 std::ifstream pf(base+"/active_paths.tsv");while(std::getline(pf,line)){auto t=line.find('\t');layer[line.substr(t+1)]=std::stoi(line.substr(0,t));}
 std::vector<Part> parts;TGeoHMatrix id;walkLayout(g->GetTopVolume(),id,g->GetTopVolume()->GetName(),layer,parts,removeEcal,dx,dy);
 int nLayers=0,activeCount=0;for(auto& kv:layer)nLayers=std::max(nLayers,kv.second+1);for(auto& part:parts)if(part.layer>=0)activeCount++;
 std::ifstream f(base+"/root_test_rays.csv");std::ofstream chords(base+"/material_chords.csv");chords<<"ray,material,entry_mm,length_mm,path\n"<<std::setprecision(12);int rays=0,mismatch=0;
 while(std::getline(f,line)){std::replace(line.begin(),line.end(),',',' ');std::istringstream ss(line);double p[3],d[3];unsigned expected;ss>>p[0]>>p[1]>>p[2]>>d[0]>>d[1]>>d[2]>>expected;unsigned actual=0;
 for(auto& part:parts){double q[3],v[3];part.matrix.MasterToLocal(p,q);part.matrix.MasterToLocalVect(d,v);auto* s=part.v->GetShape();double t=s->Contains(q)?0:s->DistFromOutside(q,v,3);if(!std::isfinite(t)||t<0||t>1e5)continue;double inside[3];for(int k=0;k<3;k++)inside[k]=q[k]+(t+1e-7)*v[k];if(!s->Contains(inside))continue;double len=s->DistFromInside(inside,v,3)+1e-7;if(!std::isfinite(len)||len<=0||len>1e5)continue;if(part.layer>=0)actual|=(1u<<part.layer);
 bool trackerParent=part.path.find("recoil_l14_sensor_vol_")!=std::string::npos && part.path.find("active_sensor")==std::string::npos;
 if(!trackerParent && expected==((1u<<nLayers)-1))chords<<rays<<','<<part.v->GetMaterial()->GetName()<<','<<t<<','<<len<<','<<part.path<<'\n';
 }
 if(actual!=expected){mismatch++;std::cout<<"Mismatch "<<rays<<" "<<actual<<" "<<expected<<std::endl;}rays++;
 }
 std::ofstream out(base+"/root_ray_check.json");out<<"{\"rays\":"<<rays<<",\"layer_decisions\":"<<rays*nLayers<<",\"mismatches\":"<<mismatch<<",\"active_volumes\":"<<activeCount<<",\"layers\":"<<nLayers<<"}\n";std::cout<<"Rays "<<rays<<" mismatches "<<mismatch<<std::endl;out.close();if(mismatch || activeCount!=(removeEcal?213:217) || rays!=192)gSystem->Exit(3);
}
