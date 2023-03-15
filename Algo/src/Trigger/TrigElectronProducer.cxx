#include "Trigger/TrigElectronProducer.h"
#include "SimCore/Event/SimTrackerHit.h"
// #include "SimCore/Event/SimParticle.h"
#include "Trigger/Event/TrigCaloCluster.h"
#include "Trigger/Event/TrigParticle.h"
#include "DetDescr/EcalGeometry.h"

#include "TCanvas.h"
#include "TString.h"

namespace trigger {

void TrigElectronProducer::configure(framework::config::Parameters& ps) {
  spCollName_ = ps.getParameter<std::string>("scoringPlaneCollName");
  clusterCollName_ = ps.getParameter<std::string>("clusterCollName");
  eleCollName_ = ps.getParameter<std::string>("eleCollName");
  propMapName_ = ps.getParameter<std::string>("propMapName");
}

void TrigElectronProducer::produce(framework::Event& event) {

  if (!event.exists(clusterCollName_)) return;
  auto ecalClusters{
      event.getObject<TrigCaloClusterCollection>(clusterCollName_)};

  if (!event.exists(spCollName_)) return;
  const std::vector<ldmx::SimTrackerHit> TargetSPHit = event.getCollection<ldmx::SimTrackerHit>(spCollName_);
  // ldmx::SimTrackerHit targetPrimary;
  // std::map<int,int> tk_to_iTargetSPHit;
  float xT=0, yT=0;
  for(const auto& hit : TargetSPHit){
    if(hit.getTrackID()!=1) continue;
    if( !(abs(hit.getPdgID())==11) ) continue;
    auto xyz = hit.getPosition();
    if( xyz[2]<0 || xyz[2]>1 ) continue; // select one sp
    xT=xyz[0];
    yT=xyz[1];
    // more details:
    // https://github.com/LDMX-Software/ldmx-analysis/blob/ch/dev/src/TriggerAnalyzer.cxx
  }

  // std::vector<ldmx::SimParticle> eles;
  TrigParticleCollection eles;
  for(const auto &clus : ecalClusters){
    TrigParticle el;
    // ldmx::SimParticle el;

    // We fit for the response in an eGun sample like this:
    // Events->Draw("Electron_e[0]/Truth_e[0]:Truth_e[0]","Truth_e[0]>500 && Truth_e[0]<3500","prof")
    // TF1* func = new TF1("func","[0]/x+[1]",500,3500);
    // htemp->Fit(func)
    float calib_e = clus.e() / (98.099700/clus.e() + 0.77202700); 

    
    // const float dX = clus.x() - xT;
    // float R = calib_e*(11500/4000.); // convert 11.5m/4GeV (in mm/MeV)
    // float zd = 240.; // 240mm z detector
    // float a = dX/zd;
    // float b = (dX*dX+zd*zd)/(2*R*zd);
    // float pred_px = clus.e() * (-b+a*sqrt(1+a*a-b*b))/(1+a*a);
    // float pred_py = clus.e()/4e3 * (clus.y() - yT) * 13.3; //mev/mm

    float pred_px = clus.z()>0 ? getPx(calib_e, (240./clus.z()) * (clus.x() - xT)) : 0; // adjust, tracing back to ecal face
    float pred_py = clus.z()>0 ? getPy(calib_e, (240./clus.z()) * (clus.y() - yT)) : 0;
    
    // std::cout << "expectX " << pred_px << " versus " << fit_px << std::endl;
    // std::cout << "expectY " << pred_py << " versus " << fit_py << std::endl;
    float pred_pz = sqrt(std::max(pow(calib_e,2) - (pow(pred_px,2)+pow(pred_py,2)),0.));

    // produce el
    Point targ(xT, yT, 0);
    LorentzVector p4(pred_px, pred_py, pred_pz, calib_e);
    eles.emplace_back(p4, targ, 11);
    Point calo(clus.x(), clus.y(), clus.z()); // n.b. "Z" is not at the ECal Face!
    eles.back().setEndPoint(calo); //clus.x(), clus.y(), clus.z()); how to handle?
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
  event.add(eleCollName_, eles);

}

void TrigElectronProducer::onFileOpen() {
  ldmx_log(debug) << "Opening file!";

  return;
}

void TrigElectronProducer::onFileClose() {
  ldmx_log(debug) << "Closing file!";

  return;
}
void TrigElectronProducer::setupMaps(bool isX) {
  TProfile2D* prof = isX ? propMapx_ : propMapy_;
  const int N = prof->GetXaxis()->GetNbins();
  for(int i=1; i<=N; i++){
    TF1* func = new TF1("func","pol1",-20,20); // going to fit for px/e
    TProfile* proj = prof->ProfileY("h",i,i);
    proj->Fit("func","q","",-1,1);
    bool debug=false;
    if(debug){
      TCanvas c("c","");
      proj->Draw();
      func->Draw("same");
      c.SaveAs( TString::Format("debugFit_%d_%d.pdf",int(isX), i) );
    }
    delete proj;
    if(isX){
      fitsX_.push_back(func);
    } else {
      fitsY_.push_back(func);
    }
  }
  return;
}


float TrigElectronProducer::getP(bool isX, float e, float d) {
  if (fabs(d)>300) return 0; // something has gone very wrong
  const bool debug = false;
  TProfile2D* prof = isX ? propMapx_ : propMapy_;
  if(!prof){
    if(debug) std::cout << "null pointer" << std::endl;
    return 0;
  }
  int bin1, bin2; 
  bin1 = prof->GetXaxis()->FindBin(e);
  float frac = e - prof->GetXaxis()->GetBinCenter(bin1);
  float diff = fabs(prof->GetXaxis()->GetBinCenter(bin1) - prof->GetXaxis()->GetBinCenter(bin2));
  bin2 = diff>0 ? bin1+1 : bin1-1;
  bin1 = std::max(1, std::min(bin1, prof->GetXaxis()->GetNbins()));
  bin2 = std::max(1, std::min(bin2, prof->GetXaxis()->GetNbins()));
  float res1, res2;
  if (isX){
    res1 = fitsX_[bin1-1]->GetX(d);
    res2 = fitsX_[bin2-1]->GetX(d);
  } else {
    res1 = fitsY_[bin1-1]->GetX(d);
    res2 = fitsY_[bin2-1]->GetX(d);
  }

  if(debug) printf("%f %f %f %f :: %f %f %f \n", d, e, prof->GetXaxis()->GetBinCenter(bin1), 
	 prof->GetXaxis()->GetBinCenter(bin2), res1, res2, abs(frac/diff) * res2 + (1 - abs(frac/diff)) * res1);
  return e*(abs(frac/diff) * res2 + (1 - abs(frac/diff)) * res1);
  
}

void TrigElectronProducer::onProcessStart() {
  ldmx_log(debug) << "Process starts!";

  auto d = gDirectory;
  TFile* f = new TFile(propMapName_.c_str(),"read");
  propMapx_ = (TProfile2D*) f->Get("profx"); propMapx_->SetDirectory(0);
  propMapy_ = (TProfile2D*) f->Get("profy"); propMapy_->SetDirectory(0);
  f->Close();

  setupMaps(0); // X
  setupMaps(1); // Y

  return;
}

void TrigElectronProducer::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";

  // free maps
  for(int i=0; i<fitsX_.size();i++) delete fitsX_[i];
  for(int i=0; i<fitsY_.size();i++) delete fitsY_[i];
  
  return;
}

}  // namespace trigger

DECLARE_PRODUCER_NS(trigger, TrigElectronProducer);
