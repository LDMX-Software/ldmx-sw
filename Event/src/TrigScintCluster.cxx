#include "Event/TrigScintCluster.h"

ClassImp(ldmx::TrigScintCluster)

namespace ldmx {

  TrigScintCluster::TrigScintCluster() :  EcalCluster() {
    
  }
  
  TrigScintCluster::~TrigScintCluster() {
    Clear();
    
  }
  
  void TrigScintCluster::Print(Option_t *option) const {
    
    std::cout << "TrigScintCluster { " << "Energy or PE: " << energy_ << ", " << "Number of hits: " << nHits_ << ", " << "Seed channel " << seed_ << ", Channel centroid: " << centroid_ << " }" << std::endl;
    std::cout << "  --  Constituent hit channel ids: {  ";
    for (const auto& idx : getHitIDs() )
      std::cout  << idx << "  ";
    std::cout << "}"<< std::endl;
    //for (int iHit = 0; iHit < hits_->GetEntries(); ++iHit) {
        //    TrigScintHit* aHit = (TrigScintHit*) hits_->At(iHit);
        //    std::cout << "Hit " << iHit << " : " << "with energy " << aHit->getEnergy() << std::endl;
        //}
  }
  
  void TrigScintCluster::Clear(Option_t*) {
    
    EcalCluster::Clear();
    setCentroid(0); 
    setSeed(-1);    
  }
  
  //  void TrigScintCluster::Copy(TObject& ob) const {
  //
  //EcalCluster::Copy( ob );
  //TrigScintCluster& tr = (TrigScintCluster&) (ob);
  //tr.centroid_ = centroid_;
    
  //}
  
}

