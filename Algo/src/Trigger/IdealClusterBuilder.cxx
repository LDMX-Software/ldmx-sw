#include "Trigger/IdealClusterBuilder.h" 

namespace trigger { 

    void ClusterGeometry::AddTP(int tid, int cell_id, int module_id, 
				float x, float y){
	id_map[tid] = std::make_pair(cell_id,module_id);
	reverse_id_map[std::make_pair(cell_id,module_id)]=tid;
	positions[tid] = std::make_pair(x,y);
    }
    void ClusterGeometry::AddNeighbor(int id1, int id2){
	if( neighbors.count(id1) ) neighbors[id1].push_back(id2);
	else neighbors[id1]={id2};
	if( neighbors.count(id2) ) neighbors[id2].push_back(id1);
	else neighbors[id2]={id1};
	//cout << "Nbs: " << id1 << " " << id2 << endl;
    }
    bool ClusterGeometry::CheckNeighbor(int id1, int id2){
	// true if neighbors
	auto &ns = neighbors[id1];
	return std::find(ns.begin(), ns.end(), id2) != ns.end();
    }
    void ClusterGeometry::Initialize(){
	// calculate pairwise distances
	distances.clear();
	for(auto pair1 = id_map.begin(); pair1 != id_map.end(); pair1++){
	    for(auto pair2 = pair1; pair2 != id_map.end(); pair2++){
		if (pair1==pair2) continue;
		auto &id1 = pair1->first;
		auto &id2 = pair2->first;
		auto &xy1 = positions[id1];
		auto &xy2 = positions[id2];
		float d = sqrt(pow(xy1.first - xy2.first,2)
			       +pow(xy1.second - xy2.second,2));
		distances[std::make_pair(id1,id2)] = d;
		distances[std::make_pair(id2,id1)] = d;
	    }
	}
	// find neighbors
	float n_dist = 1.8*GetDist(GetID(0,0), GetID(1,0));
	//float n_dist = 1.2*GetDist(GetID(0,0), GetID(1,0));
	for(auto pair1 = id_map.begin(); pair1 != id_map.end(); pair1++){
	    for(auto pair2 = pair1; pair2 != id_map.end(); pair2++){
		if (pair1==pair2) continue;
		if(GetDist(pair1->first, pair2->first) < n_dist)
		    AddNeighbor(pair1->first, pair2->first);
	    }
	}
	is_initialized = true;
    }


    /* std::vector<Cluster>  */
    /* IdealClusterBuilder::Build2dClustersLayer(std::vector<Hit> hits){ */
    std::vector<Cluster> 
    IdealClusterBuilder::Build2dClustersLayer(std::vector<Hit> hits){
	//Re-index by id
	std::map<int,Hit> hits_by_id;
	for(auto &hit : hits) hits_by_id[hit.id]=hit;

	if(debug){
	    cout << "--------\nBuild2dClustersLayer Input Hits" << endl;
	    for(auto &hitpair : hits_by_id) hitpair.second.Print();
	}
        
	// Find seeds
	std::vector<Cluster> clusters;
	for(auto &hitpair : hits_by_id){
	    auto &hit = hitpair.second;
	    bool isLocalMax = true;
	    for(auto n : g->neighbors[hit.id]){
		//cout << "  checking " << n << endl;
		if (hits_by_id.count(n) && hits_by_id[n].e > hit.e) isLocalMax = false;
	    }
	    //if(debug) cout << hit.e << " " << hit.id << " " 
	    // << hit.layer << " isMax=" << isLocalMax << endl;
	    if (isLocalMax && (hit.e > seed_thresh)){
		hit.used=true;
		Cluster c;
		c.hits.push_back( hit );
		c.e = hit.e;
		c.x = hit.x;
		c.y = hit.y;
		c.z = hit.z;
		c.seed = hit.id;
		c.module = g->id_map[hit.id].second;
		c.layer = hit.layer;
		clusters.push_back(c);
	    }
	}

	if(debug){
	    cout << "--------\nAfter seed-finding" << endl;
	    for(auto &hitpair : hits_by_id) hitpair.second.Print();
	    for(auto &c : clusters) c.Print();
	}


	// Add neighbors up to the specified limit
	int i_neighbor=0;
	while(i_neighbor < n_neighbors){
	
	    // find (unused) neighbors for all clusters
	    std::map<int, std::vector<int> > assoc_clus2hitIDs;
	    for(int iclus=0; iclus<clusters.size(); iclus++){
		auto &clus=clusters[iclus];
		std::vector<int> neighbors;
		for(const auto &hit : clus.hits){
		    for(auto n : g->neighbors[hit.id]){
			if(hits_by_id.count(n) && 
			   !hits_by_id[n].used && 
			   hits_by_id[n].e > neighb_thresh ){
			    neighbors.push_back(n);
			}
		    }
		}
		assoc_clus2hitIDs[iclus] = neighbors;
	    }

	    // check how many clusters to which each hit is assoc
	    std::map<int, std::vector<int> > assoc_hitID2clusters;
	    for(auto clus2hitID : assoc_clus2hitIDs){
		auto iclus = clus2hitID.first;
		auto &hitIDs = clus2hitID.second;
		for(const auto &hitID : hitIDs){
		    if( assoc_hitID2clusters.count(hitID) )
			assoc_hitID2clusters[hitID].push_back(iclus);
		    else
			assoc_hitID2clusters[hitID] = {iclus};
		}
	    }
	
	    // add associated hits to clusters 
	    //   (w/ optional e-splitting)
	    for(auto hitID2clusters : assoc_hitID2clusters){
		auto hitID = hitID2clusters.first;
		auto iclusters = hitID2clusters.second;
		if (iclusters.size() == 1){
		    // simply add cell to the cluster
		    auto &hit = hits_by_id[hitID];
		    auto iclus = iclusters[0];
		    hit.used=true;
		    clusters[ iclus ].hits.push_back( hit );
		    clusters[ iclus ].e += hit.e;
		} else {
		    auto &hit = hits_by_id[hitID];
		    hit.used=true;
		    float esum=0;
		    for (auto iclus : iclusters){
			esum += clusters[iclus].e;
		    }
		    for (auto iclus : iclusters){
			Hit newHit = hit;
			if(split_energy)
			    newHit.e = hit.e * clusters[iclus].e / esum;
			clusters[iclus].hits.push_back(newHit);
			clusters[iclus].e += newHit.e;
		    }
		}
	    }

	    // rebuild the clusters and return
	    for(auto &c : clusters){
		c.e=0;
		c.x=0;
		c.y=0;
		c.z=0;
		c.xx=0;
		c.yy=0;
		c.zz=0;
		float sumw=0;
		for(auto h : c.hits){
		    auto hit=h;
		    //if(debug) cout << hit.x << " " << hit.y << " " << hit.z << endl;
		    c.e += h.e;
		    //cout << "2d: " << h.e << " " << log(h.e/MIN_TP_ENERGY) << endl;
		    float w = std::max(0., log(h.e/MIN_TP_ENERGY)); // use log-e wgt
		    c.x += h.x * w;
		    c.y += h.y * w;
		    c.z += h.z * w;
		    c.xx += h.x * h.x * w;
		    c.yy += h.y * h.y * w;
		    c.zz += h.z * h.z * w;
		    sumw += w;
		}
		c.x /= sumw;
		c.y /= sumw;
		c.z /= sumw;
		c.xx /= sumw; //now is <x^2>
		c.yy /= sumw;
		c.zz /= sumw;
		c.xx = sqrt(c.xx - c.x * c.x); //now is sqrt(<x^2>-<x>^2)
		c.yy = sqrt(c.yy - c.y * c.y); 
		c.zz = sqrt(c.zz - c.z * c.z);
	    }


	    i_neighbor ++;

	    if(debug){
		cout << "--------\nAfter " << i_neighbor 
		     << " neighbors" << endl;
		for(auto &hitpair : hits_by_id) hitpair.second.Print();
		for(auto &c : clusters) c.Print();
	    }
	}

	return clusters;
    }

    void IdealClusterBuilder::Build2dClusters(){

	// first partition hits by layer
	std::map<int, std::vector<Hit> > layer_hits; // id(xy) to Hit	     
	for(const auto hit : all_hits){
	    auto l = hit.layer;
	    if (layer_hits.count(l)){
		layer_hits[l].push_back(hit);
	    } else {
		layer_hits[l]={hit};
	    }
	}
    

	// run clustering in each layer and add to the list
	for(auto &pair : layer_hits){
	    if(debug){
		cout << "Found " << pair.second.size()
		     << " hits in layer " << pair.first << endl;
	    }
	    auto clus = Build2dClustersLayer(pair.second);
	    all_clusters.insert(all_clusters.end(), 
				clus.begin(), clus.end());
	}
    

    }

    void IdealClusterBuilder::Build3dClusters(){
	if(debug){
	    cout << "--------\nBuilding 3d clusters" << endl;
	}

	// first partition 2d clusters by layer
	std::vector<std::vector<Cluster> > layer_clusters;
	layer_clusters.resize(LAYER_MAX); // first 20 layers
	for(auto &clus : all_clusters){
	    layer_clusters[clus.layer].push_back(clus);
	}

	// sort by layer
	for(auto &clusters : layer_clusters) 
	    ESort(clusters);


	if(debug){
	    cout << "--------\n3d: sorted 2d inputs" << endl;
	    for(auto &clusters : layer_clusters) 
		for(auto &c : clusters) c.Print(g);
	}


	// Pass through clusters from layer 0 to last,
	//   starting with highest energy
	bool building=true;
	std::vector<Cluster> clusters3d;
	while (building){
	    //find the seed cluster
	    Cluster cluster3d;
	    cluster3d.is2D = false;
	    cluster3d.first_layer=LAYER_SHOWERMAX;
	    cluster3d.last_layer=LAYER_SHOWERMAX;
	    int test_layer;
	    for(int ilayer=0; ilayer < LAYER_MAX; ilayer++){
		if(LAYER_SHOWERMAX + ilayer < LAYER_MAX){
		    // walk to back of ECal from shower max
		    test_layer = LAYER_SHOWERMAX + ilayer; // 7,8,9,...19
		} else {
		    // then to front of ECal
		    test_layer = LAYER_MAX-ilayer-1; // 20-13-1=6,5,4,...
		}

		auto &clusters2d=layer_clusters[test_layer];
		
		// still must find the 3d seed
		if(cluster3d.depth==0){
		    // only seed from the middle layers
		    if (test_layer > LAYER_SEEDMAX || test_layer<LAYER_SEEDMIN) continue;
		    if(clusters2d.size()){ 
			if(debug){
			    cout << " 3d seed: ";
			    clusters2d[0].Print(g);
			}
			//cout << "got seed" << endl;
			// found seed
			cluster3d.clusters2d={ clusters2d[0] };
			cluster3d.first_layer = test_layer;
			cluster3d.last_layer = test_layer;
			cluster3d.depth = 1;
			// remove (first) 2d cluster from list
			clusters2d.erase( clusters2d.begin() );
		    }
		} else {
		    // looking to extend the 3d seed
		    // grow if 2d seed is a neighbor
		    //auto &last_seed2d = clusters3d.back().seed;
		    auto &last_seed2d = cluster3d.clusters2d.back().seed;
		    // case where we begin extending cluster backward->forward
		    if(test_layer==LAYER_SHOWERMAX-1) 
			last_seed2d = cluster3d.clusters2d.front().seed;
		    if(debug){
		    // 	cout << cluster3d.clusters2d.size() << endl;
		    // 	cluster3d.clusters2d.back().Print();
			cout << "  check 3d w/ seed id " << last_seed2d << endl;
			//cout << "   from #cands: " << clusters2d.size() << endl;
		    }
		    for(int iclus2d=0; iclus2d<clusters2d.size(); iclus2d++){
			if(debug){
			    cout << "  -- " << iclus2d << endl;
			//     clusters2d[iclus2d].Print(g);
			    //cout << "  check ext " << seed2d << endl;
			}
			auto &seed2d = clusters2d[iclus2d].seed;
			if(last_seed2d==seed2d || 
			   g->CheckNeighbor(last_seed2d,seed2d)){
			    // if(debug){
			    // 	cout << " extend: ";
			    // 	clusters2d[iclus2d].Print(g);
			    // }
			    // add to 3d cluster
			    cluster3d.clusters2d.push_back(clusters2d[iclus2d]);
			    cluster3d.depth++;
			    if(test_layer < cluster3d.first_layer)
				cluster3d.first_layer = test_layer;
			    if(test_layer > cluster3d.last_layer)
				cluster3d.last_layer = test_layer;
			    //remove from list
			    clusters2d.erase( clusters2d.begin()+iclus2d );
			    //proceed to next layer
			    break;
			}
		    }
		}
	    }
	    // done with all layers. finish or store cluster
	    if(cluster3d.depth==0){
		building=false;
	    } else {
		//cout << "storing 3d cluster" << endl;
		if(cluster3d.depth >= DEPTH_GOOD)
		    clusters3d.push_back(cluster3d);
	    }
	}

	// post-process 3d clusters here
	for(auto &c : clusters3d){
	    c.e=0;
	    c.x=0;
	    c.y=0;
	    c.z=0;
	    c.xx=0;
	    c.yy=0;
	    c.zz=0;
	    float sumw=0;
	    for(auto c2 : c.clusters2d){
		c.e += c2.e;
		//cout << "3d: " << c2.e << " " << log(c2.e/MIN_TP_ENERGY) << endl;
		float w = std::max(0., log(c2.e/MIN_TP_ENERGY)); // use log-e wgt
		c.x += c2.x * w;
		c.y += c2.y * w;
		c.z += c2.z * w;
		c.xx += c2.x * c2.x * w;
		c.yy += c2.y * c2.y * w;
		c.zz += c2.z * c2.z * w;
		sumw += w;
	    }
	    // cout << "sum: " << sumw << endl;
	    // cout << "x: " << c.x << endl;
	    c.x /= sumw;
	    // cout << "x: " << c.x << endl;
	    c.y /= sumw;
	    c.z /= sumw;
	    c.xx /= sumw; //now is <x^2>
	    c.yy /= sumw;
	    c.zz /= sumw;
	    c.xx = sqrt(c.xx - c.x * c.x); //now is sqrt(<x^2>-<x>^2)
	    c.yy = sqrt(c.yy - c.y * c.y); 
	    c.zz = sqrt(c.zz - c.z * c.z);
	    Fit(c); //calc dx/dz, dy/dz
	}

	if(debug){
	    cout << "--------\nFound 3d clusters" << endl;
	    for(auto &c : clusters3d) c.Print3d();
	}



	// std::map<int, std::vector<Cluster> > layer_clusters; // id(xy) to Hit
	// for(const auto clus : all_clusters){
	//     auto l = clus.layer;
	//     if (layer_clusters.count(l)){
	// 	layer_clusters[l].push_back(clus);
	//     } else {
	// 	layer_clusters[l]={clus};
	//     }
	// }
	// // sort by layer
	// for(auto &pair : layer_clusters){
	//     auto &clusters = pair.second;
	    
	//     if(debug && clusters.size()>2){
	// 	cout << "--------\nBefore sort " << endl;
	// 	for(auto &c : clusters) c.Print();
	//     }
	//     ESort(clusters);
	//     if(debug && clusters.size()>2){
	// 	cout << "-- After sort " << endl;
	// 	for(auto &c : clusters) c.Print();
	//     }
	// }

	all_clusters.clear();
	all_clusters.insert(all_clusters.begin(), 
			    clusters3d.begin(), clusters3d.end());

    }

    void IdealClusterBuilder::BuildClusters(){
	if(debug){
	    cout << "--------\nAll hits" << endl;
	    for(auto &hit : all_hits) hit.Print();
	}

	if(use_towers){
	    // project hits in z to form towers
	    std::map<int, Hit> towers; // id(xy) to Hit	     
	    for(const auto hit : all_hits){
		if ( towers.count(hit.id) ){
		    towers[hit.id].e += hit.e;
		    towers[hit.id].nSubHit++;
		} else {
		    towers[hit.id] = hit;
		    towers[hit.id].layer=0;
		    towers[hit.id].z=0;
		    towers[hit.id].nSubHit=1;
		}
	    }
	    all_hits.clear();
	    for(const auto t : towers) all_hits.push_back(t.second);

	    if(debug){
		cout << "--------\nHits after towers" << endl;
		for(auto &hit : all_hits) hit.Print();
	    }
	}

	// Cluster the hits in each plane
	Build2dClusters();

	if(!use_towers){
	    Build3dClusters();
	}
	ESort(all_clusters);

    };

    void IdealClusterBuilder::Fit(Cluster &c3){
	// TODO: think about whether to incorporate uncertainties
	//   into the fit (RMSs), or weight each layer in the fit.

	// skip short clusters
	if(c3.clusters2d.size()<4) return;
	
	//std::vector logE;
	std::vector<float> x;
	std::vector<float> y;
	std::vector<float> z;
	for(const auto c2 : c3.clusters2d){
	    //logE.push_back( log(c2.e) );
	    x.push_back( c2.x );
	    y.push_back( c2.y );
	    z.push_back( c2.z );
	}
	TGraph gxz(z.size(), z.data(), x.data());
	auto r_xz = gxz.Fit("pol1","SQ"); // p0 + x*p1
	c3.dxdz = r_xz->Value(1);
	c3.dxdze = r_xz->ParError(1);

	TGraph gyz(z.size(), z.data(), y.data());
	auto r_yz = gyz.Fit("pol1","SQ"); // p0 + x*p1
	c3.dydz = r_yz->Value(1);
	c3.dydze = r_yz->ParError(1);
    }

}
