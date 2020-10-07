/**
 * @file TrigScintCluster.h
 * @brief Class that stores cluster information from the Trigger Scintillator
 * @author Lene Kristian Bryngemark, Stanford University : adaptation to Trigger Scintillator
 */

#ifndef EVENT_TRIGSCINTCLUSTER_H_
#define EVENT_TRIGSCINTCLUSTER_H_

// STL
#include <iostream>
#include <set>

// ldmx-sw
#include "Event/EventConstants.h"
#include "Event/TrigScintHit.h"


namespace ldmx {

  /**
   * @class TrigScintCluster 
   * @brief Stores cluster information from the trigger scintillator pads. Adds on the ECal cluster functionality
   */
  class TrigScintCluster : {

  public:
    
    /**
     * Class constructor.
     */
    TrigScintCluster() = default;

    /**
     * Class destructor.
     */
    virtual ~TrigScintCluster();

    /**
     * Print a description of this object.
     */
    void Print(Option_t *option = "") const ; //override;

    /**
     * Reset the TrigScintCluster object.
     */
    void Clear(Option_t *option = "") ; // override;


    /**
     * Take in the hits that make up the cluster.
     * @param idx The digi hit's entry number in the event's digi 
     * collection.
     * @param hit The digi hit
     */

    void addHit(uint idx, const TrigScintHit* hit);

    /**
     * @param idx The digi collection index of the hit seeding the cluster
     */ 
    void setSeed( int idx ) { seed_ = idx ; } 

    /**
     *Set the cluster energy 
     * @param energy The cluster energy deposition (in units given by hit) 
     */
    void setEnergy(double energy) {
      energy_ = energy;
    }

    /**
     *Set the cluster photoelectron count (PE)
     * @param PE The cluster photoelectron count
     */
    void setPE(int PE) {
      PE_ = PE;
    }

	
    /**
     *The number of hits forming the cluster
     * @param nHits Number of hits forming the cluster
     */
    void setNHits(int nHits) {
      nHits_ = nHits;
    }

    /**
     *The channel numbers of hits forming the cluster
     * @param hitIDs vector of channel numbers of hits forming the cluster
     */
    void setIDs(std::vector<unsigned int>& hitIDs) {
      hitIDs_ = hitIDs;
    }

	
    /**
     *The cluster centroid in x,y,z (not implemented)
     * @param x Cluster x coordinate
     * @param y Cluster y coordinate
     * @param z Cluster z coordinate
     */
    void setCentroidXYZ(double x, double y, double z) {
      centroidX_ = x;
      centroidY_ = y;
      centroidZ_ = z;
    }
	
	
    /**
     * @param centroid The channel ID centroid 
     */
    void setCentroid(double centroid) {
      centroid_ = centroid;
    }
	
	
    /** Set time of hit. */
    void setTime(float t){
      time_ = t;
    }
	
    /** Get time of hit. */
    float getTime() const {
      return time_;
    }

    /** Set beam energy fraction of hit. */
    void setBeamEfrac(float e){
      beamEfrac_ = e;
    }

    /** Get beam energy fraction of hit. */
    float getBeamEfrac() const {
      return beamEfrac_;
    }

    /** Get cluster seed channel nb */
    int getSeed() const { return seed_ ; } 

    /** Get cluster total energy deposition */
    double getEnergy() const {
      return energy_;
    }

    /** Get cluster total photoelectron count */
    double getPE() const {
      return PE_;
    }

    /** Get the number of hits constituting the cluster */
    int getNHits() const {
      return nHits_;
    }

    /** Get cluster centroid in x [mm] (not implmented) */
    double getCentroidX() const {
      return centroidX_;
    }

    /** Get cluster centroid in y [mm] (not implmented) */
    double getCentroidY() const {
      return centroidY_;
    }
            
    /** Get cluster centroid in z [mm] (not implmented) */
    double getCentroidZ() const {
      return centroidZ_;
    }

    /** Get vector of channel IDs of hits forming the cluster */
    const std::vector<unsigned int>& getHitIDs() const {
      return hitIDs_;
    }
    
    /** Get the cluster centroid in units of channel nb */
    double getCentroid() const {
      return centroid_;
    }
    
    bool operator < ( const TrigScintCluster &rhs ) const {
      return this->getEnergy() < rhs.getEnergy();
    }
	
     

  private:

    //hits forming the cluster 
    std::vector<unsigned int> hitIDs_;

    //total cluster energy depostion
    double energy_{0};

    //number of hits forming the cluster
    int nHits_{0};

    //total cluster photoelectron count
    int PE_{0};

    //index of cluster seeding hit
    int seed_{-1};

    //hit centroid in units of channel nb: energy weighted average of the IDs of the hits forming the cluster
    double centroid_{0};

    //hit centroid in x [mm] (not implemented)
    double centroidX_{0};
	
    //hit centroid in y [mm] (not implemented)
    double centroidY_{0};

    //hit centroid in z [mm] (not implemented)
    double centroidZ_{0};

    //fraction of cluster energy deposited in a sim hit associated with beam electrons 
    float beamEfrac_{0.};

    //cluster time: energy weighted average of the times of the hits forming the cluster
    float time_{0.};

    /**
     * The ROOT class definition.
     */
    ClassDef(TrigScintCluster, 1);
  };
}

#endif /* EVENT_TRIGSCINTCLUSTER_H_ */
