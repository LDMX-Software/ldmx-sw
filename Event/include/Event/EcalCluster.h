/**
 * @file EcalCluster.h
 * @brief Class that stores cluster information from the ECal 
 * @author Josh Hiltbrand, University of Minnesota
 */

#ifndef EVENT_ECALCLUSTER_H_
#define EVENT_ECALCLUSTER_H_

// ROOT
#include "TObject.h" //For ClassDef
#include "TString.h"

// STL
#include <ostream>
#include <set>

// ldmx-sw
#include "Event/EventConstants.h"
#include "Event/EcalHit.h"

namespace ldmx {

    /**
     * @class EcalCluster 
     * @brief Stores cluster information from the ECal 
     */
    class EcalCluster {

        public:

            /**
             * Class constructor.
             */
            EcalCluster();

            /**
             * Class destructor.
             */
            virtual ~EcalCluster();

            /**
             * Print a description of this object.
             */
            void Print(std::ostream& o) const;

            /**
             * Reset the EcalCluster object.
             */
            void Clear();

            /**
             * Take in the hits that make up the cluster.
             * @param hit The digi hit's entry number in the events digi 
             * collection.
             */
            void addHits(const std::vector<const EcalHit*> hitsVec);

            /**
             * Sets total energy for the cluster.
             * @param energy The total energy of the cluster.
             */
            void setEnergy(double energy) {
                energy_ = energy;
            }

            /**
             * Sets total number of hits in the cluster.
             * @param nHits The total number of hits.
             */
            void setNHits(int nHits) {
                nHits_ = nHits;
            }

            /**
             * Sets a sorted vector for the IDs of the hits
             * that make up the cluster.
             * @param IDs Sorted vector of hit IDs.
             */
            void setIDs(std::vector<unsigned int>& hitIDs) {
                hitIDs_ = hitIDs;
            }

            /**
             * Sets the three coordinates of the cluster centroid 
             * @param x The x coordinate.
             * @param y The y coordinate.
             * @param z The z coordinate.
             */
            void setCentroidXYZ(double x, double y, double z) {
                centroidX_ = x;
                centroidY_ = y;
                centroidZ_ = z;
            }

            double getEnergy() const {
                return energy_;
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

            bool operator < ( const EcalCluster &rhs ) const {
                return this->getEnergy() < rhs.getEnergy();
            }

        private:

            std::vector<unsigned int> hitIDs_;
            double energy_{0};
            int nHits_{0};
            double centroidX_{0};
            double centroidY_{0};
            double centroidZ_{0};

            ClassDef(EcalCluster, 1);
    };
}

#endif
