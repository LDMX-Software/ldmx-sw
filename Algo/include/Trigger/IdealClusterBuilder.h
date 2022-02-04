#ifndef IDEALCLUSTERBUILDER_H
#define IDEALCLUSTERBUILDER_H

#include <map>
#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>
#include "TGraph.h"
#include "TFitResult.h"

using std::cout;
using std::endl;

namespace trigger { 

    class ClusterGeometry{
    public:
	bool is_initialized = false;

	// cell + module -> TP ID (get from geo service)
	std::map<std::pair<int,int>,int> reverse_id_map;
	std::map<int,std::pair<int,int> > id_map;

	// TP ID to X,Y positions in mm
	std::map<int, std::pair<float,float> > positions;

	// pairwise X,Y distances of all TPs
	std::map<std::pair<int,int>,float> distances;
    
	// list of neighbors associated to each TP
	std::map<int, std::vector<int> > neighbors; 
    
	int GetID(int cell_id, int module_id){
	    return reverse_id_map[std::make_pair(cell_id,module_id)];
	}
	float GetDist(int id1, int id2){
	    return distances[std::make_pair(id1,id2)];
	}

	void AddTP(int tid, int cell_id, int module_id, 
		   float x, float y);  
	void AddNeighbor(int id1, int id2);
	bool CheckNeighbor(int id1, int id2);

	void Initialize();
    };

    /*
      FWIW, Pictorally, the numbering for the center module is:
      28 29 30 31 | 47 46 44 41 
      24 25 26 27 | 45 43 40 37 
      20 21 22 23 | 42 39 36 34 
      16 17 18 19 | 38 35 33 32
      -----------
      12 13 14 15
      08 09 10 11
      04 05 06 07
      00 01 02 03
    */

    class Hit {
    public:
	float x=0;
	float y=0;
	float z=0;
	float e=0;
	int idx=-1;
	int cell_id=-1;
	int module_id=-1;
	int id=-1; // encodes x,y
	int layer=0; // z
	int nSubHit=0; // for towers
	bool used=false;
	void Print(){
	    cout << "Hit ("
		 << "e= " << e
		 << ", id=" << id 
		 << ", layer= " << layer
		 << ", x= " << x
		 << ", y= " << y
		 << ", z= " << z
		 << ", nSub= " << nSubHit
		 << ", used= " << used
		 << ")" 
		 << endl;
	}
    };

    class Cluster {
    public:
	std::vector<Hit> hits;
	std::vector<Cluster> clusters2d; // for 3d
	// calculate and store properties...
	float x=0;
	float y=0;
	float z=0;
	// xyz RMS
	float xx=0;
	float yy=0;
	float zz=0;
	float e=0;
	int seed=-1; //id(xy)
	int module=-1; // uses seed
	int layer=-1;

	// 3d specific
	bool is2D=true;
	bool used=false;
	int first_layer=-1;
	int last_layer=-1;
	int depth=0;
	float dxdz=0;
	float dxdze=0;
	float dydz=0;
	float dydze=0;

	void Print(ClusterGeometry* g=0){
	    //ClusterGeometry* g;
	    if(g==0){
	    cout << "Cluster ("
		 << "e= " << e
		 << ", seed id=" << seed
		 << ", x= " << x
		 << ", y= " << y
		 << ", z= " << z
		 << ", nHit= " << hits.size()
		 << ")" 
		 << endl;
	    } else {
		auto idpair = g->id_map[seed];
		cout << "Cluster ("
		     << "e= " << e
		     << ", seed id=" << seed
		     << ", cell id=" << idpair.first
		     << ", module id=" << idpair.second
		     << ", layer=" << layer
		     << ", x= " << x
		     << ", y= " << y
		     << ", z= " << z
		     << ", nHit= " << hits.size()
		     << ")" 
		     << endl;
	    }
	}
	void Print3d(){
	    cout << "Cluster ("
		 << "e= " << e
		 << ", seed id=" << seed
		 << ", x= " << x
		 << ", y= " << y
		 << ", z= " << z
		 << ", n2dClus= " << clusters2d.size()
		 << ", first_layer=" << first_layer
		 << ", depth=" << depth
		 << ")" 
		 << endl;
	}
	void PrintHits(){
	    Print();
	    for(auto &h : hits){
		cout << "  "; 
		h.Print();
	    }
	}
    };

    class IdealClusterBuilder {
    public:
	std::vector<Hit> all_hits{};
	std::vector<Cluster> all_clusters{};
	ClusterGeometry* g;

	float seed_thresh = 0; // e.g. 100
	float neighb_thresh = 0; // e.g. 100
	int n_neighbors = 1;
	bool split_energy = true;
	//bool use_towers = true;
	bool use_towers = false;
	const int LAYER_MAX = 35;
	const int LAYER_SHOWERMAX = 7;
	const int LAYER_SEEDMIN = 3;
	const int LAYER_SEEDMAX = 15;
	const float MIN_TP_ENERGY = 0.5; // in MeV
	int DEPTH_GOOD = 5;
	/* int order3d[LAYER_MAX]={ */
	/*     7,8,6,9,5,10,4,11,3,12,2,13,1,14,0,15,16,17,18,19 */
	/* }; */
	bool debug = false;

	void AddHit(Hit h){
	    if (h.layer >= LAYER_MAX) return;
	    auto p = std::make_pair(h.cell_id, h.module_id);
	    h.id = g->reverse_id_map[p];
	    all_hits.push_back(h);
	}
	std::vector<Cluster> GetClusters(){return all_clusters;}
	void SetClusterGeo(ClusterGeometry* _g){g=_g;}

	virtual void BuildClusters();
	std::vector<Cluster> Build2dClustersLayer( std::vector<Hit> hits);
	void Build2dClusters();
	void Build3dClusters();
	void Fit(Cluster &c3);

	/* void BuildClusters(); */
	/* void Cluster2dHits(); */

    };

    template<class T>
	void ESort(std::vector<T> &v){
	std::sort( v.begin(),
		   v.end(),
		   [](const auto& lhs,const auto& rhs){
		       return lhs.e > rhs.e;
		   });
    }

}

#endif // IDEALCLUSTERBUILDER_H
