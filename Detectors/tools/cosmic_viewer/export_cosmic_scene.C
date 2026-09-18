// Export real ROOT-loaded GDML surface meshes and straight-line shape
// crossings. This is geometry inspection only; it does not simulate particle
// transport.
#include <TBuffer3D.h>
#include <TGeoManager.h>
#include <TGeoMaterial.h>
#include <TGeoMatrix.h>
#include <TGeoNode.h>
#include <TGeoShape.h>
#include <TGeoVolume.h>
#include <TSystem.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>

namespace CosmicScene {
std::string esc(const std::string& s) {
  std::string o;
  for (char c : s) {
    if (c == '"' || c == '\\') o += '\\';
    if (c == '\n') {
      o += "\\n";
      continue;
    }
    o += c;
  }
  return o;
}
std::string low(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return s;
}
std::string group(const std::string& path) {
  auto p = low(path);
  if (p.find("cad") != std::string::npos ||
      p.find("mechanic") != std::string::npos)
    return "cad";
  if (p.find("trig") != std::string::npos) return "trigger";
  if (p.find("recoil") != std::string::npos ||
      p.find("tracker") != std::string::npos)
    return "tracker";
  if (p.find("hcal") != std::string::npos ||
      p.find("hadron_cal") != std::string::npos)
    return "hcal";
  if (p.find("ecal") != std::string::npos ||
      p.find("em_cal") != std::string::npos)
    return "ecal";
  if (p.find("target") != std::string::npos) return "target";
  return "other";
}
void vec(std::ofstream& out, const double* p) {
  out << '[' << p[0] << ',' << p[1] << ',' << p[2] << ']';
}
void visit(TGeoVolume* vol, const TGeoHMatrix& matrix, const std::string& path,
           std::ofstream& out, bool& first, const double* ray,
           const double* dir, int& n) {
  std::string g = group(path),
              mat = vol->GetMaterial() ? vol->GetMaterial()->GetName() : "";
  auto* shape = vol->GetShape();
  if (g != "cad" && g != "other" && shape && !vol->IsAssembly() &&
      low(mat) != "air" && low(mat) != "vacuum") {
    const auto& b = shape->GetBuffer3D(TBuffer3D::kAll, kTRUE);
    if (b.NbPnts() > 0 && b.NbPols() > 0 && b.fPnts && b.fPols && b.fSegs) {
      if (!first) out << ",\n";
      first = false;
      ++n;
      out << "{\"path\":\"" << esc(path) << "\",\"volume\":\""
          << esc(vol->GetName()) << "\",\"subsystem\":\"" << g
          << "\",\"material\":\"" << esc(mat) << "\",\"shape\":\""
          << shape->ClassName() << "\",\"vertices_mm\":[";
      for (unsigned j = 0; j < b.NbPnts(); ++j) {
        double p[3];
        matrix.LocalToMaster(b.fPnts + 3 * j, p);
        if (j) out << ',';
        vec(out, p);
      }
      out << "],\"polygons\":[";
      int off = 0;
      bool pf = true;
      for (unsigned j = 0; j < b.NbPols(); ++j) {
        int ns = b.fPols[off + 1];
        std::vector<std::pair<int, int>> edges;
        for (int k = 0; k < ns; k++) {
          int si = b.fPols[off + 2 + k];
          edges.push_back({b.fSegs[3 * si + 1], b.fSegs[3 * si + 2]});
        }
        off += ns + 2;
        if (edges.empty()) continue;
        std::vector<int> ids = {edges[0].first, edges[0].second};
        edges.erase(edges.begin());
        while (!edges.empty()) {
          bool found = false;
          for (auto it = edges.begin(); it != edges.end(); ++it) {
            int nxt = -1;
            if (it->first == ids.back())
              nxt = it->second;
            else if (it->second == ids.back())
              nxt = it->first;
            if (nxt >= 0) {
              ids.push_back(nxt);
              edges.erase(it);
              found = true;
              break;
            }
          }
          if (!found) break;
        }
        if (ids.back() == ids.front()) ids.pop_back();
        if (ids.size() < 3) continue;
        if (!pf) out << ',';
        pf = false;
        out << '[';
        for (unsigned k = 0; k < ids.size(); ++k) {
          if (k) out << ',';
          out << ids[k];
        }
        out << ']';
      }
      out << "],\"ray_intersection\":";
      double p[3], d[3];
      matrix.MasterToLocal(ray, p);
      matrix.MasterToLocalVect(dir, d);
      double entry = shape->Contains(p) ? 0 : shape->DistFromOutside(p, d, 3);
      if (entry >= 0 && std::isfinite(entry) && entry < 1e8) {
        double q[3];
        for (int k = 0; k < 3; k++) q[k] = p[k] + (entry + 1e-6) * d[k];
        double len = shape->DistFromInside(q, d, 3);
        if (shape->Contains(q) && std::isfinite(len) && len < 1e8) {
          double a[3], z[3];
          for (int k = 0; k < 3; k++) {
            a[k] = ray[k] + entry * dir[k];
            z[k] = ray[k] + (entry + 1e-6 + len) * dir[k];
          }
          out << "{\"entry_t_mm\":" << entry
              << ",\"exit_t_mm\":" << entry + 1e-6 + len << ",\"entry_mm\":";
          vec(out, a);
          out << ",\"exit_mm\":";
          vec(out, z);
          out << '}';
        } else
          out << "null";
      } else
        out << "null";
      out << '}';
    }
  }
  if (g == "cad") return;
  for (int i = 0; i < vol->GetNdaughters(); ++i) {
    auto* child = vol->GetNode(i);
    TGeoHMatrix m(matrix);
    m.Multiply(child->GetMatrix());
    visit(child->GetVolume(), m, path + "/" + child->GetName(), out, first, ray,
          dir, n);
  }
}
}  // namespace CosmicScene
void export_cosmic_scene(const char* gdml, const char* output, double x = 0,
                         double y = 0, double z = -1000, double dx = 0,
                         double dy = 0, double dz = 1) {
  TGeoManager::SetDefaultUnits(TGeoManager::kG4Units);
  auto* geo = TGeoManager::Import(gdml);
  if (!geo || !geo->GetTopVolume()) {
    gSystem->Exit(2);
    return;
  }
  double ray[] = {x, y, z}, dir[] = {dx, dy, dz};
  double norm = sqrt(dx * dx + dy * dy + dz * dz);
  for (double& v : dir) v /= norm;
  std::ofstream out(output);
  out << std::setprecision(12) << "{\"source_gdml\":\""
      << CosmicScene::esc(gdml)
      << "\",\"units\":\"mm\",\"method\":\"ROOT surface meshes and shape "
         "DistFromOutside/Inside\",\"caveat\":\"Illustrative trajectory; no "
         "Geant4 transport, energy loss, or simulated hits. Intersections are "
         "geometric material-solid crossings, not detector "
         "response.\",\"ray_origin_mm\":";
  CosmicScene::vec(out, ray);
  out << ",\"ray_direction\":";
  CosmicScene::vec(out, dir);
  out << ",\"objects\":[\n";
  bool first = true;
  int n = 0;
  TGeoHMatrix identity;
  CosmicScene::visit(geo->GetTopVolume(), identity,
                     geo->GetTopVolume()->GetName(), out, first, ray, dir, n);
  out << "\n]}\n";
  std::cout << "Exported " << n << " detector material solids to " << output
            << std::endl;
}
