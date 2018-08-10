/**
 * @file HitBox.h
 * @brief Header file for class HitBox
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef TOOLS_HITBOX_H
#define TOOLS_HITBOX_H

#include <vector> //points

namespace ldmx {

    /**
     * @class HitBox
     * @brief Stores the real space information of a hit in the form of an axis-aligned box around an "origin"
     *
     * Each coordinate x,y,z has three values: Minimum, origin, maximum. The Min and Max are prettu
     *  self-explanatory. The origin acts as a center, but since it doesn't have to be midway between
     *  min and max, we don't call it the center.
     */
    class HitBox {
        public:
           
            /**
             * Constructor
             */
            HitBox() : min_( 3 , 0.0 ), origin_( 3 , 0.0 ) , max_( 3 , 0.0 ) { }

            /**
             * Set coordinate i
             */
            void setCoordinate( const int i , const double min , const double origin, const double max ) {
                min_[i] = min;
                origin_[i] = origin;
                max_[i] = max;
                return;
            }

            /**
             * Set the x coordinate
             */
            void setX( const double min , const double origin , const double max ) {
                setCoordinate( 0 , min , origin , max );
                return;
            }

            /**
             * Set the y coordinate
             */
            void setY( const double min , const double origin , const double max ) {
                setCoordinate( 1 , min , origin , max );
                return;
            }

            /**
             * Set the z coordinate
             */
            void setZ( const double min , const double origin , const double max ) {
                setCoordinate( 2 , min , origin , max );
                return;
            }

            /**
             * Get the origin/center of the box
             */
            std::vector<double> getOrigin() const { return origin_; }

            /**
             * Get the minimum corner of the box
             */
            std::vector<double> getMin() const { return min_; }

            /**
             * Get the maximum corner of the box
             */
            std::vector<double> getMax() const { return max_; }
            

        private:
            
            /** coordinates of the minimum point */
            std::vector< double > min_;

            /** coordinates of the origin */
            std::vector< double > origin_;

            /** coordinates of the maximium point */
            std::vector< double > max_;
    };

}

#endif /* TOOLS_HITBOX_H */
