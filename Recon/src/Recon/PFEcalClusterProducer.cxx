//#include "DetDescr/EcalGeometry.h"
// #include "Recon/Event/HgcrocDigiCollection.h"

#include "Recon/PFEcalClusterProducer.h"


namespace recon {

void fillClusterInfo(ldmx::EcalCluster &cl, std::vector<ldmx::EcalHit> hits,
		       std::vector<unsigned int> toUse={}, bool select=false){
  float e(0),x(0),y(0),z(0),xx(0),yy(0),zz(0),n(0);
  float w = 1; // weight
  float sumw = 0;
  float minEnergy=0; // min hit energy to consider
  std::vector<float> xvals{};
  std::vector<float> yvals{};
  std::vector<float> zvals{};
  if (!select){ // use all the hit indices...
    for(unsigned int i=0; i<hits.size();i++){
      toUse.push_back(i);
    }
  }
  for(unsigned int i : toUse){
    const auto &h = hits[i];
    //if (h.getEnergy() < minEnergy) continue;
    // w = log( h.getEnergy() - log(minEnergy) );
    e += h.getEnergy();
    x += w * h.getXPos();
    y += w * h.getYPos();
    z += w * h.getZPos();
    xx += w * h.getXPos() * h.getXPos();
    yy += w * h.getYPos() * h.getYPos();
    zz += w * h.getZPos() * h.getZPos();
    n += 1;
    sumw += w;
    xvals.push_back(x);
    yvals.push_back(y);
    zvals.push_back(z);
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
  cl.setEnergy(e);
  cl.setNHits(n);
  cl.setCentroidXYZ(x,y,z);
  cl.setRMSXYZ(xx,yy,zz);

  if (xvals.size()>2){
    for(int i=0; i<xvals.size();i++){ // mean subtract
      xvals[i] = xvals[i] - x;
      yvals[i] = yvals[i] - y;
      zvals[i] = zvals[i] - z;
    }
    TGraph gxz(zvals.size(), zvals.data(), xvals.data());
    auto r_xz = gxz.Fit("pol1","SQ"); // p0 + x*p1
    cl.setDXDZ( r_xz->Value(1) );
    cl.setEDXDZ( r_xz->ParError(1) );
    TGraph gyz(zvals.size(), zvals.data(), yvals.data());
    auto r_yz = gyz.Fit("pol1","SQ"); // p0 + x*p1
    cl.setDYDZ( r_yz->Value(1) );
    cl.setEDYDZ( r_yz->ParError(1) );
  }
  return;
  //pfClusters.push_back(cl);
}

void PFEcalClusterProducer::configure(framework::config::Parameters& ps) {
  hitCollName_ = ps.getParameter<std::string>("hitCollName");
  clusterCollName_ = ps.getParameter<std::string>("clusterCollName"); 
  singleCluster_ = ps.getParameter<bool>("doSingleCluster");
}
bool isIn(unsigned int i, std::vector<unsigned int> l){
  return std::find(l.begin(), l.end(), i) != l.end();
}
float dist(ldmx::EcalHit a, ldmx::EcalHit b){
  return sqrt( pow(a.getXPos() - b.getXPos(),2)
	       + pow(a.getYPos() - b.getYPos(),2)
	       + pow(a.getZPos() - b.getZPos(),2));
}
void PFEcalClusterProducer::produce(framework::Event& event) {

  if (!event.exists(hitCollName_)) return;
  const auto ecalRecHits = event.getCollection<ldmx::EcalHit>(hitCollName_);
  //std::cout << "found " << ecalRecHits.size() << " hits" << std::endl;

  const auto& geo = getCondition<ldmx::EcalGeometry>(ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);

  std::vector<ldmx::EcalCluster> pfClusters;
  if(!singleCluster_){ 
    // make configurable
    float minEnergy=0;
    int minNHit=5;
    float minDist=5;
    // DBSCAN
    const auto &hits = ecalRecHits;
    const int n = ecalRecHits.size();
    std::vector<std::vector<unsigned int> > idx_clusters;
    std::vector<unsigned int> tried; tried.reserve(n);
    std::vector<unsigned int> used; used.reserve(n);
    //std::sort(hits.begin(), hits.end(), [](const ldmx::EcalHit a, const ldmx::EcalHit b) { return a.getEnergy() > b.getEnergy(); });
    for(unsigned int i=0;i<n;i++){
      if( isIn(i,tried) ) continue;
      tried.push_back(i);
      if (hits[i].getEnergy() < minEnergy) continue;
      std::set<unsigned int> neighbors;
      unsigned int nNearby=1;
      // find neighbors
      for(unsigned int j=0;j<n;j++){
	if( i!=j && dist(hits[i],hits[j]) < minDist){
	  neighbors.insert(j);
	  if (hits[j].getEnergy() >= minEnergy) nNearby++;
	}
      }
      if (nNearby > minNHit){
	std::vector<unsigned int> idx_cluster{i}; // start a cluster
	used.push_back(i);
	for(unsigned int j : neighbors){
	  if ( !isIn(j,tried) ){
	    tried.push_back(j);
	    std::vector<unsigned int> neighbors2;
	    for(unsigned int k=0;k<n;k++){
	      if( k!=j && dist(hits[i],hits[j]) < minDist){
		neighbors2.push_back(k);
	      }
	    }
	    for(unsigned int k : neighbors2)
	      neighbors.insert(k);
	  }
	  if ( !isIn(j,used) ) idx_cluster.push_back(j);
	}
	idx_clusters.push_back( idx_cluster );
      }
    }
    for(const auto idx_cluster : idx_clusters){
      //std::vector<ldmx::EcalHit>
      ldmx::EcalCluster cl;
      fillClusterInfo(cl,ecalRecHits, idx_cluster, true);
      pfClusters.push_back(cl);
    }
      

    // // use the approach from EcalClusterProducer
    // ecal::TemplatedClusterFinder<ecal::MyClusterWeight> cf;
    // for(const auto &h : ecalRecHits) cf.add(&h, geo);
    // cf.cluster(100.0 , 10.); // default from ecalClusters.py
    // //cf.cluster(seedThreshold_, cutoff_);
    // std::vector<ecal::WorkingCluster> wcVec = cf.getClusters();
    // //std::map<int, double> cWeights = cf.getWeights();

    // // write cluster
    // for (int aWC = 0; aWC < wcVec.size(); aWC++) {
    //   ldmx::EcalCluster cluster;
    //   cluster.setEnergy(wcVec[aWC].centroid().E());
    //   cluster.setCentroidXYZ(wcVec[aWC].centroid().Px(),
    // 			     wcVec[aWC].centroid().Py(),
    // 			     wcVec[aWC].centroid().Pz());
    //   cluster.setNHits(wcVec[aWC].getHits().size());
    //   cluster.addHits(wcVec[aWC].getHits());
    //   pfClusters.push_back(cluster);
    // }
    
  } else { // create a single, large cluster
    ldmx::EcalCluster cl;
    fillClusterInfo(cl,ecalRecHits);
    pfClusters.push_back(cl);
  }
  event.add(clusterCollName_, pfClusters);
}

void PFEcalClusterProducer::onFileOpen() {
  ldmx_log(debug) << "Opening file!";

  return;
}

void PFEcalClusterProducer::onFileClose() {
  ldmx_log(debug) << "Closing file!";

  return;
}

void PFEcalClusterProducer::onProcessStart() {
  ldmx_log(debug) << "Process starts!";

  return;
}

void PFEcalClusterProducer::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";

  return;
}

}  // namespace recon

DECLARE_PRODUCER_NS(recon, PFEcalClusterProducer);
