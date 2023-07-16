// #include "DetDescr/EcalGeometry.h"
// #include "Recon/Event/HgcrocDigiCollection.h"
#include <iostream>
#include <set>
#include "Recon/DBScanClusterBuilder.h"

using std::cout;
using std::endl;

namespace recon {

  DBScanClusterBuilder::DBScanClusterBuilder(){
    minHitEnergy_      = 0;
    clusterHitDist_    = 100;
    minClusterHitMult_ = 2;
  }

  DBScanClusterBuilder::DBScanClusterBuilder(float minHitEnergy, float clusterHitDist, float minClusterHitMult){
    minHitEnergy_      = minHitEnergy;
    clusterHitDist_    = clusterHitDist;
    minClusterHitMult_ = minClusterHitMult;
  }

  std::vector<std::vector<const ldmx::CalorimeterHit*> > DBScanClusterBuilder::runDBSCAN( 
    const std::vector<const ldmx::CalorimeterHit*> &hits, bool debug = false){
  
    const int n = hits.size();
    std::vector<std::vector<const ldmx::CalorimeterHit*> > idx_clusters;
    std::vector<unsigned int> tried; tried.reserve(n);
    std::vector<unsigned int> used; used.reserve(n);
    for(unsigned int i=0;i<n;i++){
      if( isIn(i,tried) ) continue;
      tried.push_back(i);
      if (debug) cout << "trying " << i << endl;
      if (hits[i]->getEnergy() < minHitEnergy_) continue;
      std::set<unsigned int> neighbors;
      unsigned int nNearby=1;
      // find neighbors
      for(unsigned int j=0;j<n;j++){
	if( i!=j && dist(hits[i],hits[j]) < clusterHitDist_){
	  neighbors.insert(j);
	  if (hits[j]->getEnergy() >= minHitEnergy_) nNearby++;
	}
      }
      if (nNearby >= minClusterHitMult_){
	std::vector<const ldmx::CalorimeterHit*> idx_cluster{hits[i]}; // start a cluster
	used.push_back(i);
	if (debug) cout << "- starting a cluster from " << i << endl;
	for(unsigned int j : neighbors){
	  if ( !isIn(j,tried) ){
	    tried.push_back(j);
	    if (debug) cout << "== tried " << j << endl;
	    std::vector<unsigned int> neighbors2;
	    for(unsigned int k=0;k<n;k++){
	      if( dist(hits[k],hits[j]) < clusterHitDist_){
		neighbors2.push_back(k);
	      }
	    }
	    for(unsigned int k : neighbors2)
	      neighbors.insert(k);
	  }
	  if ( !isIn(j,used) ) {
	    if (debug) cout << "== used " << j << endl;
	    used.push_back(j);
	    idx_cluster.push_back(hits[j]);
	  }
	}
	idx_clusters.push_back( idx_cluster );
      }
    }
    return idx_clusters;
    if (debug)
      cout << "done. writing this many clusters out: " 
	   << idx_clusters.size() << endl;

}

 void DBScanClusterBuilder::fillClusterInfoFromHits(ldmx::CaloCluster *cl,
    std::vector<const ldmx::CalorimeterHit*> hits,
    float minHitEnergy, bool logEnergyWeight){
			      
  float e(0),x(0),y(0),z(0),xx(0),yy(0),zz(0),n(0);
  float w = 1; // weight
  float sumw = 0;
  std::vector<float> xvals{};
  std::vector<float> yvals{};
  std::vector<float> zvals{};
  std::vector<float> raw_xvals{};
  std::vector<float> raw_yvals{};
  std::vector<float> raw_zvals{};
  std::vector<float> raw_evals{};

  for(const ldmx::CalorimeterHit* h : hits){
    if (h->getEnergy() < minHitEnergy) continue;
    if (logEnergyWeight) w = log( h->getEnergy() - log(minHitEnergy) );
    e += h->getEnergy();
    x += w * h->getXPos();
    y += w * h->getYPos();
    z += w * h->getZPos();
    xx += w * h->getXPos() * h->getXPos();
    yy += w * h->getYPos() * h->getYPos();
    zz += w * h->getZPos() * h->getZPos();
    n += 1;
    sumw += w;
    xvals.push_back(x);
    yvals.push_back(y);
    zvals.push_back(z);
    raw_xvals.push_back(h->getXPos());
    raw_yvals.push_back(h->getYPos());
    raw_zvals.push_back(h->getZPos());
    raw_evals.push_back(h->getEnergy());
  }
  x /= sumw; // now is <x>
  y /= sumw;
  z /= sumw;
  xx /= sumw; // now is <x^2>
  yy /= sumw;
  zz /= sumw;
  xx = sqrt(xx - x * x); //now is sqrt(<x^2>-<x>^2)
  yy = sqrt(yy - y * y); 
  zz = sqrt(zz - z * z);
  cl->setEnergy(e);
  cl->setNHits(n);
  cl->setCentroidXYZ(x,y,z);
  cl->setRMSXYZ(xx,yy,zz);
  cl->setHitValsX(raw_xvals);
  cl->setHitValsY(raw_yvals);
  cl->setHitValsZ(raw_zvals);
  cl->setHitValsE(raw_evals);

  if (xvals.size()>2){
    for(int i=0; i<xvals.size();i++){ // mean subtract
      xvals[i] = xvals[i] - x;
      yvals[i] = yvals[i] - y;
      zvals[i] = zvals[i] - z;
    }
    TGraph gxz(zvals.size(), zvals.data(), xvals.data());
    auto r_xz = gxz.Fit("pol1","SQ"); // p0 + x*p1
    cl->setDXDZ( r_xz->Value(1) );
    cl->setEDXDZ( r_xz->ParError(1) );
    TGraph gyz(zvals.size(), zvals.data(), yvals.data());
    auto r_yz = gyz.Fit("pol1","SQ"); // p0 + x*p1
    cl->setDYDZ( r_yz->Value(1) );
    cl->setEDYDZ( r_yz->ParError(1) );
  }
  return;
}

}  // namespace recon

