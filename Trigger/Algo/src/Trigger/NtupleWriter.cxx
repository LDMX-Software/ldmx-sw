#include "Trigger/NtupleWriter.h"

#include "SimCore/Event/SimTrackerHit.h"
#include "Trigger/Event/TrigEnergySum.h"
#include "Trigger/Event/TrigParticle.h"

namespace trigger {
NtupleWriter::NtupleWriter(const std::string& name, framework::Process& process)
    : Producer(name, process) {}

void NtupleWriter::configure(framework::config::Parameters& ps) {
  outPath_ = ps.getParameter<std::string>("outPath");
}

// precision-limiting function
// inline float prec(float x, unsigned int nBits=22){ return
// float(int(x*(1<<nBits)))/(1<<nBits);}
inline float prec(float x) { return x; }

void NtupleWriter::produce(framework::Event& event) {
  framework::NtupleManager& n{framework::NtupleManager::getInstance()};

  std::string inTag;
  inTag = "TargetScoringPlaneHits";
  if (writeTruth_ && event.exists(inTag)) {
    const std::vector<ldmx::SimTrackerHit> hits =
        event.getCollection<ldmx::SimTrackerHit>(inTag);
    ldmx::SimTrackerHit h, hMaxEle;  // the desired truth hits
    for (const auto& hit : hits) {
      auto xyz = hit.getPosition();
      if (xyz[2] > 0 && xyz[2] < 1) {
        if (hit.getTrackID() == 1) h = hit;
        if (hit.getPdgID() == 11 && (hit.getEnergy() > hMaxEle.getEnergy()))
          hMaxEle = hit;
      } else {
        continue;  // select one sp
      }
    }
    if (h.getPdgID() == 0)
      h = hMaxEle;  // save max energy in case track1 isn't found (A')
    std::string coll = "Truth";
    n.setVar(coll + "_e", prec(h.getEnergy()));
    n.setVar(coll + "_x", prec(h.getPosition()[0]));
    n.setVar(coll + "_y", prec(h.getPosition()[1]));
    n.setVar(coll + "_px", prec(h.getMomentum()[0]));
    n.setVar(coll + "_py", prec(h.getMomentum()[1]));
    n.setVar(coll + "_pz", prec(h.getMomentum()[2]));
    n.setVar(coll + "_pdgId", h.getPdgID());
  }
  inTag = "EcalScoringPlaneHits";
  if (writeTruth_ && event.exists(inTag)) {
    const std::vector<ldmx::SimTrackerHit> hits =
        event.getCollection<ldmx::SimTrackerHit>(inTag);
    ldmx::SimTrackerHit h, hMaxEle;  // the desired truth hits
    for (const auto& hit : hits) {
      auto xyz = hit.getPosition();
      if (xyz[2] > 239.99 && xyz[2] < 240.01) {
        if (hit.getTrackID() == 1) h = hit;
        if (hit.getPdgID() == 11 && (hit.getEnergy() > hMaxEle.getEnergy()))
          hMaxEle = hit;
      } else {
        continue;  // select one sp
      }
    }
    if (h.getPdgID() == 0)
      h = hMaxEle;  // save max energy in case track1 isn't found (A')
    std::string coll = "TruthEcal";
    n.setVar(coll + "_e", prec(h.getEnergy()));
    n.setVar(coll + "_x", prec(h.getPosition()[0]));
    n.setVar(coll + "_y", prec(h.getPosition()[1]));
    n.setVar(coll + "_px", prec(h.getMomentum()[0]));
    n.setVar(coll + "_py", prec(h.getMomentum()[1]));
    n.setVar(coll + "_pz", prec(h.getMomentum()[2]));
    n.setVar(coll + "_pdgId", h.getPdgID());
  }

  inTag = "ecalTrigSums";
  if (writeEcalSums_ && event.exists(inTag)) {
    const auto sums = event.getCollection<TrigEnergySum>(inTag);
    // const int nEcalLayers = 34;
    vector<float> energyAfterLayer;  // (nEcalLayers, 0.);
    for (const auto& sum : sums) {
      if (!(sum.energy() > 0)) continue;
      if (sum.layer() >= energyAfterLayer.size())
        energyAfterLayer.resize(sum.layer() + 1);
      for (int i = 0; i <= sum.layer(); i++) {
        energyAfterLayer[i] += sum.energy();
      }
    }
    n.setVar("Ecal_e_afterLayer", energyAfterLayer);
    n.setVar("Ecal_e_nLayer", int(energyAfterLayer.size()));
  }
  inTag = "hcalTrigQuadsBackLayerSums";
  if (writeHcalSums_ && event.exists(inTag)) {
    const auto sums = event.getCollection<TrigEnergySum>(inTag);
    vector<float> energyAfterLayer;
    for (const auto& sum : sums) {
      if (!(sum.hwEnergy() > 0)) continue;
      if (sum.layer() >= energyAfterLayer.size())
        energyAfterLayer.resize(sum.layer() + 1);
      for (int i = 0; i <= sum.layer(); i++) {
        energyAfterLayer[i] += sum.hwEnergy();
      }
    }
    n.setVar("Hcal_e_afterLayer", energyAfterLayer);
    n.setVar("Hcal_e_nLayer", int(energyAfterLayer.size()));
  }

  inTag = "trigElectrons";
  if (writeEle_ && event.exists(inTag)) {
    const auto eles = event.getCollection<TrigParticle>(inTag);
    const int nEle = eles.size();
    int maxE = -1;
    float maxEVal = 0;
    int maxPt = -1;
    float maxPtVal = 0;
    vector<float> v_e(nEle);
    vector<float> v_eC(nEle);
    vector<float> v_zC(nEle);
    vector<float> v_px(nEle);
    vector<float> v_py(nEle);
    vector<float> v_pz(nEle);
    vector<float> v_dx(nEle);
    vector<float> v_dy(nEle);
    vector<float> v_x(nEle);
    vector<float> v_y(nEle);
    vector<int> v_tp(nEle);
    vector<int> v_depth(nEle);
    for (unsigned int i = 0; i < nEle; i++) {
      if (eles[i].energy() > maxEVal) {
        maxEVal = eles[i].energy();
        maxE = i;
      }
      if (eles[i].pt() > maxPtVal) {
        maxPtVal = eles[i].pt();
        maxPt = i;
      }
      v_e[i] = prec(eles[i].energy());
      v_eC[i] = prec(eles[i].getClusEnergy());
      v_zC[i] = prec(eles[i].endz());
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
    n.setVar("n" + coll, nEle);
    n.setVar("maxE", maxE);
    n.setVar("maxPt", maxPt);
    n.setVar(coll + "_e", v_e);
    n.setVar(coll + "_eClus", v_eC);
    n.setVar(coll + "_zClus", v_zC);
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
  outFile_ = new TFile(outPath_.c_str(), "recreate");
  outFile_->SetCompressionSettings(209);
  // 100*alg+level
  // 2=LZMA, 9 = max compression
  framework::NtupleManager& n{framework::NtupleManager::getInstance()};
  n.create(tag_);

  if (writeEle_) {
    std::string coll = "Electron";
    n.addVar<int>(tag_, "n" + coll);
    n.addVar<int>(tag_, "maxE");
    n.addVar<int>(tag_, "maxPt");
    n.addVar<vector<float> >(tag_, coll + "_e");
    n.addVar<vector<float> >(tag_, coll + "_eClus");
    n.addVar<vector<float> >(tag_, coll + "_zClus");
    n.addVar<vector<float> >(tag_, coll + "_px");
    n.addVar<vector<float> >(tag_, coll + "_py");
    n.addVar<vector<float> >(tag_, coll + "_pz");
    n.addVar<vector<float> >(tag_, coll + "_dx");
    n.addVar<vector<float> >(tag_, coll + "_dy");
    n.addVar<vector<float> >(tag_, coll + "_x");  // at target
    n.addVar<vector<float> >(tag_, coll + "_y");
    n.addVar<vector<int> >(tag_, coll + "_tp");
    n.addVar<vector<int> >(tag_, coll + "_depth");
  }
  if (writeTruth_) {
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
  if (writeEcalSums_) {
    n.addVar<vector<float> >(tag_, "Ecal_e_afterLayer");
    n.addVar<int>(tag_, "Ecal_e_nLayer");
  };
  if (writeHcalSums_) {
    n.addVar<vector<float> >(tag_, "Hcal_e_afterLayer");
    n.addVar<int>(tag_, "Hcal_e_nLayer");
  };
}
void NtupleWriter::onProcessEnd() {
  outFile_->Write();
  outFile_->Close();
}

}  // namespace trigger
DECLARE_PRODUCER_NS(trigger, NtupleWriter);
