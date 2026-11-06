#include "Trigger/TrigElectronProducer.h"

// #include "SimCore/Event/SimParticle.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TString.h"
#include "TrigScint/Event/TrigScintTrack.h"
#include "Trigger/Event/TrigCaloCluster.h"
#include "Trigger/Event/TrigParticle.h"

namespace trigger {

void TrigElectronProducer::configure(framework::config::Parameters& ps) {
  cluster_coll_name_ = ps.get<std::string>("cluster_coll_name");
  ele_coll_name_ = ps.get<std::string>("ele_coll_name");
  prop_map_name_ = ps.get<std::string>("prop_map_name");
  cluster_coll_passname_ = ps.get<std::string>("cluster_coll_passname");
  cluster_collname_events_passname_ =
      ps.get<std::string>("cluster_collname_events_passname");
  ts_track_coll_name_ = ps.get<std::string>("ts_track_coll_name");
  ts_passname_ = ps.get<std::string>("ts_passname");
  ts_y0_ = static_cast<float>(ps.get<double>("ts_y0"));
  ts_channel_pitch_ = static_cast<float>(ps.get<double>("ts_channel_pitch"));
}

void TrigElectronProducer::produce(framework::Event& event) {
  if (!event.exists(cluster_coll_name_, cluster_collname_events_passname_))
    return;
  auto ecal_clusters{event.getObject<TrigCaloClusterCollection>(
      cluster_coll_name_, cluster_coll_passname_)};

  // target y from the trigger scintillator
  const auto y_t{tsTargetY(event)};

  // std::vector<ldmx::SimParticle> eles;
  TrigParticleCollection eles;
  for (const auto& clus : ecal_clusters) {
    TrigParticle el;
    // ldmx::SimParticle el;

    // We fit for the response in an eGun sample like this:
    // Events->Draw("Electron_e[0]/Truth_e[0]:Truth_e[0]","Truth_e[0]>500 &&
    // Truth_e[0]<3500","prof")
    // --> update fit to use the electron energy at the ecal face (not target)
    // -->
    // Events->Draw("Electron_eClus[0]/TruthEcal_e[0]:TruthEcal_e[0]","TruthEcal_e[0]>500
    // && TruthEcal_e[0]<3500","prof") TF1* func = new
    // TF1("func","[0]/x+[1]",500,3500); htemp->Fit(func) float calib_e =
    // clus.e() / (98.099700/clus.e() + 0.77202700);
    float calib_e =
        clus.e() / (184.103 / clus.e() + 0.753756);  // divide by response

    // const float dX = clus.x() - xT;
    // float R = calib_e*(11500/4000.); // convert 11.5m/4GeV (in mm/MeV)
    // float zd = 240.; // 240mm z detector
    // float a = dX/zd;
    // float b = (dX*dX+zd*zd)/(2*R*zd);
    // float pred_px = clus.e() * (-b+a*sqrt(1+a*a-b*b))/(1+a*a);
    // float pred_py = clus.e()/4e3 * (clus.y() - yT) * 13.3; //mev/mm

    // Events->Draw("TruthEcal_y-Truth_y -
    // Electron_dy*240/Electron_zClus","TruthEcal_e>1e3")

    // the TS measures only y, so px is not predicted
    const float pred_px = 0;
    // y deflection, tracing back to the ECal face
    const float pred_py =
        y_t && clus.z() > 0
            ? getPy(calib_e, (240. / clus.z()) * (clus.y() - *y_t))
            : 0;

    // std::cout << "expectX " << pred_px << " versus " << fit_px << std::endl;
    // std::cout << "expectY " << pred_py << " versus " << fit_py << std::endl;
    float pred_pz = sqrt(
        std::max(pow(calib_e, 2) - (pow(pred_px, 2) + pow(pred_py, 2)), 0.));

    // produce el
    Point targ(0, y_t.value_or(0), 0);
    LorentzVector p4(pred_px, pred_py, pred_pz, calib_e);
    eles.emplace_back(p4, targ, 11);
    Point calo(clus.x(), clus.y(),
               clus.z());  // n.b. "Z" is not at the ECal Face!
    eles.back().setEndPoint(
        calo);  // clus.x(), clus.y(), clus.z()); how to handle?
    eles.back().setClusEnergy(clus.e());
    eles.back().setClusTP(clus.nTP());
    eles.back().setClusDepth(clus.depth());

    // el.setEnergy(clus.e());
    // el.setPdgID(11);
    // el.setCharge(-1);
    // el.setMass(0.000511);
    // el.setVertex(0, xT, yT);
    // el.setMomentum(pred_px, pred_py, pred_pz);
    // eles.push_back(el);
  }
  event.add(ele_coll_name_, eles);
}

void TrigElectronProducer::setupMaps(bool isX) {
  TProfile2D* prof = isX ? prop_mapx_ : prop_mapy_;
  const int n = prof->GetXaxis()->GetNbins();
  for (int i = 1; i <= n; i++) {
    TF1* func = new TF1("func", "pol1", -20, 20);  // going to fit for px/e
    TProfile* proj = prof->ProfileY("h", i, i);
    proj->Fit("func", "q", "", -1, 1);
    bool debug = false;
    if (debug) {
      TCanvas c("c", "");
      proj->Draw();
      func->Draw("same");
      c.SaveAs(TString::Format("debug_fit_%d_%d.pdf", int(isX), i));
    }
    delete proj;
    if (isX) {
      fits_x_.push_back(func);
    } else {
      fits_y_.push_back(func);
    }
  }
  return;
}

float TrigElectronProducer::getP(bool isX, float e, float d) {
  if (fabs(d) > 300) return 0;  // something has gone very wrong
  const bool debug = false;
  TProfile2D* prof = isX ? prop_mapx_ : prop_mapy_;
  if (!prof) {
    if (debug) std::cout << "null pointer" << std::endl;
    return 0;
  }
  int bin1{-9999}, bin2{-9999};
  bin1 = prof->GetXaxis()->FindBin(e);
  float frac = e - prof->GetXaxis()->GetBinCenter(bin1);
  float diff = fabs(prof->GetXaxis()->GetBinCenter(bin1) -
                    prof->GetXaxis()->GetBinCenter(bin2));
  bin2 = diff > 0 ? bin1 + 1 : bin1 - 1;
  bin1 = std::max(1, std::min(bin1, prof->GetXaxis()->GetNbins()));
  bin2 = std::max(1, std::min(bin2, prof->GetXaxis()->GetNbins()));
  float res1, res2;
  if (isX) {
    res1 = fits_x_[bin1 - 1]->GetX(d);
    res2 = fits_x_[bin2 - 1]->GetX(d);
  } else {
    res1 = fits_y_[bin1 - 1]->GetX(d);
    res2 = fits_y_[bin2 - 1]->GetX(d);
  }

  if (debug)
    printf("%f %f %f %f :: %f %f %f \n", d, e,
           prof->GetXaxis()->GetBinCenter(bin1),
           prof->GetXaxis()->GetBinCenter(bin2), res1, res2,
           std::abs(frac / diff) * res2 + (1 - std::abs(frac / diff)) * res1);
  return e *
         (std::abs(frac / diff) * res2 + (1 - std::abs(frac / diff)) * res1);
}

void TrigElectronProducer::onProcessStart() {
  ldmx_log(debug) << "Process starts!";

  TFile* f = new TFile(prop_map_name_.c_str(), "read");
  prop_mapx_ = (TProfile2D*)f->Get("profx");
  prop_mapx_->SetDirectory(0);
  prop_mapy_ = (TProfile2D*)f->Get("profy");
  prop_mapy_->SetDirectory(0);
  f->Close();

  setupMaps(0);  // X
  setupMaps(1);  // Y

  return;
}

void TrigElectronProducer::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";

  // free maps
  for (int i = 0; i < fits_x_.size(); i++) delete fits_x_[i];
  for (int i = 0; i < fits_y_.size(); i++) delete fits_y_[i];

  return;
}

std::optional<float> TrigElectronProducer::tsTargetY(
    const framework::Event& event) {
  const auto& coll{ts_track_coll_name_};
  if (!event.exists(coll, ts_passname_)) {
    if (!warned_missing_ts_) {
      ldmx_log(warn) << "No " << coll << " (pass '" << ts_passname_
                     << "') in the event: trigger electrons get py = 0.";
      warned_missing_ts_ = true;
    }
    return std::nullopt;
  }
  // lowest residual with ties going to the highest PE
  std::optional<float> centroid;
  float residual{0};
  float pe{0};
  for (const auto& track :
       event.getCollection<ldmx::TrigScintTrack>(coll, ts_passname_)) {
    if (centroid &&
        (track.getResidual() > residual ||
         (track.getResidual() == residual && track.getPE() <= pe))) {
      continue;
    }
    centroid = track.getCentroid();
    residual = track.getResidual();
    pe = track.getPE();
  }
  if (!centroid) return std::nullopt;
  return ts_y0_ + ts_channel_pitch_ * (*centroid);
}

}  // namespace trigger

DECLARE_PRODUCER(trigger::TrigElectronProducer);
