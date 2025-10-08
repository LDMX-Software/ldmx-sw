#include "Trigger/NtupleWriter.h"

#include "SimCore/Event/SimTrackerHit.h"
#include "Trigger/Event/TrigEnergySum.h"
#include "Trigger/Event/TrigMip.h"
#include "Trigger/Event/TrigParticle.h"

namespace trigger {
NtupleWriter::NtupleWriter(const std::string& name, framework::Process& process)
    : Producer(name, process) {}

void NtupleWriter::configure(framework::config::Parameters& ps) {
  out_path_ = ps.get<std::string>("outPath");

  target_sp_hits_event_passname_ =
      ps.get<std::string>("target_sp_hits_event_passname");
  target_sp_passname_ = ps.get<std::string>("target_sp_passname");

  ecal_sp_hits_events_passname_ =
      ps.get<std::string>("ecal_sp_hits_events_passname");
  ecal_sp_passname_ = ps.get<std::string>("ecal_sp_passname");

  ecal_trig_sums_event_passname_ =
      ps.get<std::string>("ecal_trig_sums_event_passname");
  ecal_trig_sums_passname_ = ps.get<std::string>("ecal_trig_sums_passname");

  trig_electrons_event_passname_ =
      ps.get<std::string>("trig_electrons_event_passname");
  trig_electrons_passname_ = ps.get<std::string>("trig_electrons_passname");

  hcal_trig_quads_events_passname_ =
      ps.get<std::string>("hcal_trig_quads_events_passname");
  hcal_trig_quads_passname_ = ps.get<std::string>("hcal_trig_quads_passname");
}

// precision-limiting function
// inline float prec(float x, unsigned int nBits=22){ return
// float(int(x*(1<<nBits)))/(1<<nBits);}
inline float prec(float x) { return x; }

void NtupleWriter::produce(framework::Event& event) {
  framework::NtupleManager& n{framework::NtupleManager::getInstance()};

  std::string in_tag;
  in_tag = "TargetScoringPlaneHits";
  if (write_truth_ && event.exists(in_tag, target_sp_hits_event_passname_)) {
    const std::vector<ldmx::SimTrackerHit> hits =
        event.getCollection<ldmx::SimTrackerHit>(in_tag, target_sp_passname_);

    ldmx::SimTrackerHit h, h_max_ele;  // the desired truth hits
    for (const auto& hit : hits) {
      auto xyz = hit.getPosition();
      if (xyz[2] > 0 && xyz[2] < 1) {
        if (hit.getTrackID() == 1) h = hit;
        if (hit.getPdgID() == 11 && (hit.getEnergy() > h_max_ele.getEnergy()))
          h_max_ele = hit;
      } else {
        continue;  // select one sp
      }
    }
    if (h.getPdgID() == 0)
      h = h_max_ele;  // save max energy in case track1 isn't found (A')
    std::string coll = "Truth";
    n.setVar(coll + "_e", prec(h.getEnergy()));
    n.setVar(coll + "_x", prec(h.getPosition()[0]));
    n.setVar(coll + "_y", prec(h.getPosition()[1]));
    n.setVar(coll + "_px", prec(h.getMomentum()[0]));
    n.setVar(coll + "_py", prec(h.getMomentum()[1]));
    n.setVar(coll + "_pz", prec(h.getMomentum()[2]));
    n.setVar(coll + "_pdgId", h.getPdgID());
  }
  in_tag = "EcalScoringPlaneHits";
  if (write_truth_ && event.exists(in_tag, ecal_sp_hits_events_passname_)) {
    const std::vector<ldmx::SimTrackerHit> hits =
        event.getCollection<ldmx::SimTrackerHit>(in_tag, ecal_sp_passname_);
    ldmx::SimTrackerHit h, h_max_ele;  // the desired truth hits
    for (const auto& hit : hits) {
      auto xyz = hit.getPosition();
      if (xyz[2] > 239.99 && xyz[2] < 240.01) {
        if (hit.getTrackID() == 1) h = hit;
        if (hit.getPdgID() == 11 && (hit.getEnergy() > h_max_ele.getEnergy()))
          h_max_ele = hit;
      } else {
        continue;  // select one sp
      }
    }
    if (h.getPdgID() == 0)
      h = h_max_ele;  // save max energy in case track1 isn't found (A')
    std::string coll = "TruthEcal";
    n.setVar(coll + "_e", prec(h.getEnergy()));
    n.setVar(coll + "_x", prec(h.getPosition()[0]));
    n.setVar(coll + "_y", prec(h.getPosition()[1]));
    n.setVar(coll + "_px", prec(h.getMomentum()[0]));
    n.setVar(coll + "_py", prec(h.getMomentum()[1]));
    n.setVar(coll + "_pz", prec(h.getMomentum()[2]));
    n.setVar(coll + "_pdgId", h.getPdgID());
  }

  in_tag = "ecalTrigSums";
  if (write_ecal_sums_ &&
      event.exists(in_tag, ecal_trig_sums_event_passname_)) {
    const auto sums =
        event.getCollection<TrigEnergySum>(in_tag, ecal_trig_sums_passname_);
    // const int nEcalLayers = 34;
    vector<float> energy_after_layer;  // (nEcalLayers, 0.);
    for (const auto& sum : sums) {
      if (!(sum.energy() > 0)) continue;
      if (sum.layer() >= energy_after_layer.size())
        energy_after_layer.resize(sum.layer() + 1);
      for (int i = 0; i <= sum.layer(); i++) {
        energy_after_layer[i] += sum.energy();
      }
    }
    n.setVar("Ecal_e_afterLayer", energy_after_layer);
    n.setVar("Ecal_e_nLayer", int(energy_after_layer.size()));
  }
  in_tag = "hcalTrigQuadsBackLayerSums";
  if (write_hcal_sums_ &&
      event.exists(in_tag, hcal_trig_quads_events_passname_)) {
    const auto sums =
        event.getCollection<TrigEnergySum>(in_tag, hcal_trig_quads_passname_);
    vector<float> energy_after_layer;
    for (const auto& sum : sums) {
      if (!(sum.hwEnergy() > 0)) continue;
      if (sum.layer() >= energy_after_layer.size())
        energy_after_layer.resize(sum.layer() + 1);
      for (int i = 0; i <= sum.layer(); i++) {
        energy_after_layer[i] += sum.hwEnergy();
      }
    }
    n.setVar("Hcal_e_afterLayer", energy_after_layer);
    n.setVar("Hcal_e_nLayer", int(energy_after_layer.size()));
  }

  in_tag = "hcalTrigQuadsSideLayerSums";
  if (write_hcal_sums_ &&
      event.exists(in_tag, hcal_trig_quads_events_passname_)) {
    const auto sums =
        event.getCollection<TrigEnergySum>(in_tag, hcal_trig_quads_passname_);
    float energy_in_side_hcal = 0.f;
    for (const auto& sum : sums) {
      if (!(sum.hwEnergy() > 0)) continue;
      for (int i = 0; i <= sum.layer(); i++) {
        energy_in_side_hcal += sum.hwEnergy();
      }
    }
    n.setVar("SideHcal_e", energy_in_side_hcal);
  }

  in_tag = "ecalTrigMIPs";
  if (write_ecal_trig_mi_ps_ &&
      event.exists(in_tag, ecal_trig_sums_event_passname_)) {
    const auto mips =
        event.getCollection<TrigMip>(in_tag, ecal_trig_sums_passname_);
    std::vector<int> lengths;
    std::vector<int> n_holes;
    std::vector<float> iso_energies;
    for (const auto& mip : mips) {
      lengths.push_back(mip.length());
      n_holes.push_back(mip.nHoles());
      iso_energies.push_back(mip.sumEinIsolationRegion());
    }
    n.setVar("Ecal_mip_length", lengths);
    n.setVar("Ecal_mip_nHoles", n_holes);
    n.setVar("Ecal_mip_isolationEnergy", iso_energies);
  }

  in_tag = "hcalTrigMIPs";
  if (write_hcal_trig_mi_ps_ &&
      event.exists(in_tag, hcal_trig_quads_events_passname_)) {
    const auto mips =
        event.getCollection<TrigMip>(in_tag, hcal_trig_quads_passname_);
    std::vector<int> lengths;
    std::vector<int> n_holes;
    // std::vector<float> isoEnergies;
    for (const auto& mip : mips) {
      lengths.push_back(mip.length());
      n_holes.push_back(mip.nHoles());
      // isoEnergies.push_back(mip.SumEinIsolationRegion());
    }
    n.setVar("Hcal_mip_length", lengths);
    n.setVar("Hcal_mip_nHoles", n_holes);
    // n.setVar("Hcal_mip_isolationEnergy", isoEnergies);
  }

  in_tag = "trigElectrons";
  if (write_ele_ && event.exists(in_tag, trig_electrons_event_passname_)) {
    const auto eles =
        event.getCollection<TrigParticle>(in_tag, trig_electrons_passname_);
    const int n_ele = eles.size();
    int max_e = -1;
    float max_e_val = 0;
    int max_pt = -1;
    float max_pt_val = 0;
    vector<float> v_e(n_ele);
    vector<float> v_e_c(n_ele);
    vector<float> v_z_c(n_ele);
    vector<float> v_px(n_ele);
    vector<float> v_py(n_ele);
    vector<float> v_pz(n_ele);
    vector<float> v_dx(n_ele);
    vector<float> v_dy(n_ele);
    vector<float> v_x(n_ele);
    vector<float> v_y(n_ele);
    vector<int> v_tp(n_ele);
    vector<int> v_depth(n_ele);
    for (unsigned int i = 0; i < n_ele; i++) {
      if (eles[i].energy() > max_e_val) {
        max_e_val = eles[i].energy();
        max_e = i;
      }
      if (eles[i].pt() > max_pt_val) {
        max_pt_val = eles[i].pt();
        max_pt = i;
      }
      v_e[i] = prec(eles[i].energy());
      v_e_c[i] = prec(eles[i].getClusEnergy());
      v_z_c[i] = prec(eles[i].endz());
      v_px[i] = prec(eles[i].px());
      v_py[i] = prec(eles[i].py());
      v_pz[i] = prec(eles[i].pz());
      v_dx[i] = prec(eles[i].endx() - eles[i].vx());
      v_dy[i] = prec(eles[i].endy() - eles[i].vy());
      v_x[i] = prec(eles[i].vx());
      v_y[i] = prec(eles[i].vy());
      v_tp[i] = prec(eles[i].getClusTP());
      v_depth[i] = prec(eles[i].getClusDepth());
    }
    std::string coll = "Electron";
    n.setVar("n" + coll, n_ele);
    n.setVar("maxE", max_e);
    n.setVar("maxPt", max_pt);
    n.setVar(coll + "_e", v_e);
    n.setVar(coll + "_eClus", v_e_c);
    n.setVar(coll + "_zClus", v_z_c);
    n.setVar(coll + "_px", v_px);
    n.setVar(coll + "_py", v_py);
    n.setVar(coll + "_pz", v_pz);
    n.setVar(coll + "_dx", v_dx);
    n.setVar(coll + "_dy", v_dy);
    n.setVar(coll + "_x", v_x);
    n.setVar(coll + "_y", v_y);
    n.setVar(coll + "_tp", v_tp);
    n.setVar(coll + "_depth", v_depth);
  }
}

void NtupleWriter::onProcessStart() {
  // auto hdir = getHistoDirectory();
  out_file_ = new TFile(out_path_.c_str(), "recreate");
  out_file_->SetCompressionSettings(209);
  // 100*alg+level
  // 2=LZMA, 9 = max compression
  framework::NtupleManager& n{framework::NtupleManager::getInstance()};
  n.create(tag_);

  if (write_ele_) {
    std::string coll = "Electron";
    n.addVar<int>(tag_, "n" + coll);
    n.addVar<int>(tag_, "maxE");
    n.addVar<int>(tag_, "maxPt");
    n.addVar<vector<float>>(tag_, coll + "_e");
    n.addVar<vector<float>>(tag_, coll + "_eClus");
    n.addVar<vector<float>>(tag_, coll + "_zClus");
    n.addVar<vector<float>>(tag_, coll + "_px");
    n.addVar<vector<float>>(tag_, coll + "_py");
    n.addVar<vector<float>>(tag_, coll + "_pz");
    n.addVar<vector<float>>(tag_, coll + "_dx");
    n.addVar<vector<float>>(tag_, coll + "_dy");
    n.addVar<vector<float>>(tag_, coll + "_x");  // at target
    n.addVar<vector<float>>(tag_, coll + "_y");
    n.addVar<vector<int>>(tag_, coll + "_tp");
    n.addVar<vector<int>>(tag_, coll + "_depth");
  }
  if (write_truth_) {
    n.addVar<float>(tag_, "Truth_x");
    n.addVar<float>(tag_, "Truth_y");
    n.addVar<float>(tag_, "Truth_px");
    n.addVar<float>(tag_, "Truth_py");
    n.addVar<float>(tag_, "Truth_pz");
    n.addVar<float>(tag_, "Truth_e");
    n.addVar<int>(tag_, "Truth_pdgId");
    n.addVar<float>(tag_, "TruthEcal_x");
    n.addVar<float>(tag_, "TruthEcal_y");
    n.addVar<float>(tag_, "TruthEcal_px");
    n.addVar<float>(tag_, "TruthEcal_py");
    n.addVar<float>(tag_, "TruthEcal_pz");
    n.addVar<float>(tag_, "TruthEcal_e");
    n.addVar<int>(tag_, "TruthEcal_pdgId");
  }
  if (write_ecal_trig_mi_ps_) {
    n.addVar<std::vector<int>>(tag_, "Ecal_mip_length");
    n.addVar<std::vector<int>>(tag_, "Ecal_mip_nHoles");
    n.addVar<std::vector<float>>(tag_, "Ecal_mip_isolationEnergy");
  }
  if (write_hcal_trig_mi_ps_) {
    n.addVar<std::vector<int>>(tag_, "Hcal_mip_length");
    n.addVar<std::vector<int>>(tag_, "Hcal_mip_nHoles");
    // n.addVar<std::vector<float>>(tag_, "Hcal_mip_isolationEnergy");
  }
  if (write_ecal_sums_) {
    n.addVar<vector<float>>(tag_, "Ecal_e_afterLayer");
    n.addVar<int>(tag_, "Ecal_e_nLayer");
  };
  if (write_hcal_sums_) {
    n.addVar<vector<float>>(tag_, "Hcal_e_afterLayer");
    n.addVar<int>(tag_, "Hcal_e_nLayer");
    n.addVar<float>(tag_, "SideHcal_e");
  };
}
void NtupleWriter::onProcessEnd() {
  out_file_->Write();
  out_file_->Close();
}

}  // namespace trigger
DECLARE_PRODUCER(trigger::NtupleWriter);
