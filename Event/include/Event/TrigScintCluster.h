#ifndef EVENT_TRIGSCINTCLUSTER_H
#define EVENT_TRIGSCINTCLUSTER_H

// ROOT
#include "TObject.h"

// STL
#include <iostream>
#include <set>

// ldmx-sw
#include "Event/EventConstants.h"
#include "Event/TrigScintHit.h"
#include "Event/EcalCluster.h"


namespace ldmx {

    /**
     * @class TrigScintCluster 
     * @brief Stores cluster information from the trigger scintillator pads. 
     *  Adds on the ECal cluster functionality
     */
    class TrigScintCluster : public EcalCluster {

        public:

            /**
             * Class constructor.
             */
            TrigScintCluster();

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
             * @param hit The digi hit's entry number in the events digi 
             * collection.
             */

            void addHit(uint idx, const TrigScintHit* hit);

            void setSeed( int idx ) { seed_ = idx ; } 

            void setEnergy(double energy) {
                energy_ = energy;
            }

            void setPE(int PE) {
                PE_ = PE;
            }


            void setNHits(int nHits) {
                nHits_ = nHits;
            }

            void setIDs(std::vector<unsigned int>& hitIDs) {
                hitIDs_ = hitIDs;
            }

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

            int getSeed() const { return seed_ ; } 


            double getEnergy() const {
                return energy_;
            }

            double getPE() const {
                return PE_;
            }

            int getNHits() const {
                return nHits_;
            }

            double getCentroidX() const {
                return centroidX_;
            }

            double getCentroidY() const {
                return centroidY_;
            }

            double getCentroidZ() const {
                return centroidZ_;
            }

            const std::vector<unsigned int>& getHitIDs() const {
                return hitIDs_;
            }


            double getCentroid() const {
                return centroid_;
            }



        private:

            std::vector<unsigned int> hitIDs_;
            double energy_{0};
            int nHits_{0};
            int PE_{0};
            int seed_{-1};
            double centroid_{0};
            double centroidX_{0};
            double centroidY_{0};
            double centroidZ_{0};


            float beamEfrac_{0.};
            float time_{0.};

            ClassDef(TrigScintCluster, 1);
    };
}

#endif /* EVENT_TRIGSCINTCLUSTER_H */
