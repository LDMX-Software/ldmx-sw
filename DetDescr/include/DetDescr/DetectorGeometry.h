/**
 * @file DetectorGeometry.h
 * @author Tom Eichlersmith, University of Minnesota
 * @brief Header file for class DetectorGeometry
 */

#ifndef DETDESCR_DETECTORGEOMETRY_H
#define DETDESCR_DETECTORGEOMETRY_H

//STL
#include <map> //storage maps
#include <vector> //BoundingBox
#include <utility> //BoundingBox
#include <cmath> //sqrt
#include <iostream> //cerr
#include <memory> //unique_ptr

//LDMX Framework
#include "DetDescr/HcalID.h" //HcalSection enum
#include "DetDescr/EcalHexReadout.h"
#include "Event/SimTrackerHit.h" //recoil hits
#include "Event/HcalHit.h"
#include "Event/EcalHit.h"

namespace ldmx {
    
    /**
     * @type BoundingBox
     * @brief Stores the minimum and maximum of each coordinate for a box.
     *
     * This has all of the information needed to define an axis-aligned rectangular prism.
     */
    typedef std::vector< std::pair< double, double > > BoundingBox;

    /**
     * @struct HexPrism
     * @brief Stores the necessary geometry details for a hexagonal prism.
     */
    struct HexPrism {
        double x;
        double y;
        double z;
        double height;
        double radius;
    };

    /**
     * @class DetectorGeometry
     * @brief Class to translated between detector location (section, layer, strip) and real space.
     */
    class DetectorGeometry {
        public:
            /**
             * Constructor
             * This is where all the detector constants are set.
             */
            DetectorGeometry();

            /**
             * Calculate real space coordinates from detector location.
             *
             * @param hit HcalHit to find real space hit for
             * @return BoundingBox in real space
             */
            BoundingBox getBoundingBox( HcalHit* hit ) const;
            
            /**
             * Calculate real space coordinates of a cluster of hits.
             *
             * Determines cluster's coordinates by a weighted mean of the individuals.
             * 
             * @param hitVec vector of HcalHits to find a "center" for
             * @return BoundingBox in real space
             */
            BoundingBox getBoundingBox( const std::vector< HcalHit* > &hitVec ) const;

            /**
             * Get bounding box for the input section.
             *
             * @param section HcalSection
             * @return BoundingBox that bounds section
             */
            BoundingBox getBoundingBox( HcalSection section ) const;

            /**
             * Calculate bounding hexagonal prism for cell with the input IDs
             *
             * @param cellID int ID for cell
             * @param moduleID int ID for module that contains the cell
             * @param layer int layer contains the module
             * @return HexPrism
             */
            HexPrism getHexPrism( unsigned int cellID , unsigned int moduleID , int layer ) const;

            /**
             * Calculate bounding hexagonal prism for input EcalHit.
             *
             * @param hit EcalHit to find real space description
             * @return HexPrism
             */
            HexPrism getHexPrism( EcalHit* hit ) const;

            /**
             * Get HexPrism for a tower
             *
             * @param towerIndex int index of hexagonal tower
             * @return HexPrism
             */
            HexPrism getHexPrism( int towerIndex ) const;

            /**
             * Get Rotation Angle around z-axis for the input layerID and moduleID
             *
             * @param layerID index for layer of recoil module
             * @param moduleID index for module of recoil module
             * @return rotation angle in radians
             */
            double getRotAngle( int layerID , int moduleID ) const;

            /**
             * Get Bounding Box for input recoil module
             * NOTE: This does not take into account any rotation! Use getRotAngle as well!
             *
             * @param layerID index for layer of module
             * @param moduleID index for module
             * @return BoundingBox that bounds the module
             */
            BoundingBox getBoundingBox( int layerID , int moduleID ) const;

            /**
             * Get Bounding Box for input recoil hit
             * NOTE: This does not take into account any rotation! Use getRotAngle as well!
             *
             * @param recoilHit SimTrackerHit in recoil tracker
             * @return BoundingBox that bounds the hit
             */
            BoundingBox getBoundingBox( SimTrackerHit* recoilHit ) const;
        
        private:

            /////////////////////////////////////////////////////////////
            // HCAL

            /** Number of layers in each section */
            std::map< HcalSection , int > hcalNLayers_;

            /** Number of strips per layer in each section */
            std::map< HcalSection , int > hcalNStrips_;

            /** Length of Scintillator Strip [mm] */
            std::map< HcalSection , double > hcalLengthScint_;

            /** The plane of the zero'th layer of each section [mm] */
            std::map< HcalSection , double > hcalZeroLayer_;
            
            /** The plane of the zero'th strip of each section [mm] */
            std::map< HcalSection , double > hcalZeroStrip_;

            /** Thickness of the layers in each seciton [mm] */
            std::map< HcalSection , double > hcalLayerThickness_;
 
            /** an example layer number of a vertical layer */
            int hcalParityVertical_;

            /** Uncertainty in timing position along a bar/strip [mm] */
            double hcalUncertaintyTimingPos_;

            /** Thickness of Scintillator Strip [mm] */
            double hcalThicknessScint_;

            /** Width of Scintillator Strip [mm] */
            double hcalWidthScint_;

            /////////////////////////////////////////////////////////////
            // ECAL

            /** Radius of hexagons [mm] */
            double ecalHexRadius_;

            /** Gap between adjacent hexagons in transvers direction [mm] */
            double ecalHexGap_;

            /** Plane of the zero'th ecal layer */
            double ecalZeroLayer_;

            /** Number of cells across a given module */
            int ecalNCellsWide_;

            /** Thickness of sensitive Si layers */
            double ecalSiThickness_;

            /** Total depth of ECAL (length in Z direction) */
            double ecalDepth_;

            /** Planes of the Si layers */
            std::vector< double > ecalSiPlanes_;

            /** Helper class to calculate (x,y) coordinate from hexagons */
            std::unique_ptr<EcalHexReadout> ecalHexReader_;

            /** XYCoord for each ECAL Tower (Calculated from ecalHexRadius_ and ecalHexGap_ */
            std::vector< XYCoords > ecalXYTower_;

            /////////////////////////////////////////////////////////////
            // RECOIL TRACKER

            double recoilStereoStripLength_;

            double recoilStereoXWidth_;

            double recoilStereoYWidth_;

            double recoilStereoSeparation_;

            double recoilStereoAngle_;

            double recoilMonoStripLength_;

            double recoilMonoXWidth_;

            double recoilMonoYWidth_;

            double recoilMonoSeparation_;

            double recoilSensorThickness_;

            /** position of each module in recoil detector
             * The key in this map is 10*layerID+moduleID
             */
            std::map< int , std::vector<double> > recoilModulePos_;

            /** angular tilt for each module in recoil detector
             * The key in this map is 10*layerID+moduleID
             */
            std::map< int , double > recoilModuleAngle_;
    };

    /**
     * Namespace Wide Instance
     */
    const DetectorGeometry DETECTOR_GEOMETRY;
}

#endif /* DETDESCR_DETECTORGEOMETRY_H */
