/**
 * @file PositionDifference.h
 * @brief Class that represents difference in position
 * between TS clusters/tracks and beam electrons in a TS pad.
 * @author Erik Lundblad, Lund University
 */

#ifndef TRIGSCINT_EVENT_POSITIONDIFFERENCE_H_
#define TRIGSCINT_EVENT_POSITIONDIFFERENCE_H_

namespace ldmx {

/**
 * @class PositionDifference
 * @brief Position difference between TS clusters/tracks
 * and beam electron position in a TS pad
 */
class PositionDifference {
    public:
        // Constructor
        PositionDifference() = default;

        /**
         * Sets the x position difference
         * @param dx The position difference in x
         */
        void setDx(double dx) { dx_ = dx; }

        /**
         * Sets the y position difference
         * @param dy The position difference in y
         */
        void setDy(double dy) { dy_ = dy; }

        /**
         * Sets the z position difference
         * @param dz The position difference in z
         */
        void setDz(double dz) { dz_ = dz; }

        /**
         * Set cluster/track uncertainty in x
         */
        void setSigmaX(double sx) { sx_ = sx; }

        /**
         * Set cluster/track uncertainty in y
         */
        void setSigmaY(double sy) { sy_ = sy; }

        /**
         * Set cluster/track uncertainty in z
         */
        void setSigmaZ(double sz) {sz_ = sz; }

        /**
         * @return dx The difference in x coordinate
         */
        double getDx() const { return dx_; } 

        /**
         * @return dy The difference in y coordinate
         */
        double getDy() const { return dy_; }
        
        /**
         * @return dz The difference in z coordinate
         */
        double getDz() const { return dz_; }

        /**
         * @return sx The cluster/track uncertainty in x
         */
        double getSigmaX() const { return sx_; }

        /**
         * @return sy The cluster/track uncertainty in y
         */
        double getSigmaY() const { return sy_; }

        /**
         * @return sz The cluster/track uncertainty in z
         */
        double getSigmaZ() const { return sz_; }
    
    private:
        /**
         * Position difference in x. Initially nonsene value
         */
        double dx_{-9999.};
        /**
         * Position difference in y. Initially nonsene value
         */
        double dy_{-9999.};
        /**
         * Position difference in z. Initially nonsene value
         */
        double dz_{-9999.};

        /**
         * Cluster/track uncertainty in xthat we wish to compare to
         */
        double sx_{-9999.};

        /**
         * Cluster/track uncertainty in xthat we wish to compare to
         */
        double sy_{-9999.};

        /**
         * Cluster/track uncertainty in xthat we wish to compare to
         */
        double sz_{-9999.};
};  // PositionDifference
}   // namespace trigscint

#endif // TRIGSCINT_EVENT_POSITIONDIFFERENCE_H_