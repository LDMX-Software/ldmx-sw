/**
 * @file DetectorGeometry.cxx
 * @brief Implementation file for class DetectorGeometry
 */

#include "DetDescr/DetectorGeometry.h"

namespace ldmx {
    
    DetectorGeometry::DetectorGeometry() {
        
        ///////////////////////////////////////////////////////////////////////////////////
        // HCAL

        hcalParityVertical_ = 1;
        hcalThicknessScint_ = 15.0; 

        hcalWidthScint_ = 100.0;

        hcalNLayers_[ HcalSection::BACK   ] = 100;
        hcalNLayers_[ HcalSection::TOP    ] = 32;
        hcalNLayers_[ HcalSection::BOTTOM ] = 32;
        hcalNLayers_[ HcalSection::LEFT   ] = 32;
        hcalNLayers_[ HcalSection::RIGHT  ] = 32;
        
        hcalNStrips_[ HcalSection::BACK   ] = 31;
        hcalNStrips_[ HcalSection::TOP    ] = 3;
        hcalNStrips_[ HcalSection::BOTTOM ] = 3;
        hcalNStrips_[ HcalSection::LEFT   ] = 3;
        hcalNStrips_[ HcalSection::RIGHT  ] = 3;
         
        double ecal_z  = 290.;
        double ecal_xy = 525.;
        double back_transverse_width = 3100.;
        double ecal_front_z = 200.;

        hcalLengthScint_[ HcalSection::BACK   ] = back_transverse_width;
        hcalLengthScint_[ HcalSection::TOP    ] = (back_transverse_width+ecal_xy)/2.;
        hcalLengthScint_[ HcalSection::BOTTOM ] = (back_transverse_width+ecal_xy)/2.;
        hcalLengthScint_[ HcalSection::LEFT   ] = (back_transverse_width+ecal_xy)/2.;
        hcalLengthScint_[ HcalSection::RIGHT  ] = (back_transverse_width+ecal_xy)/2.;
         
        hcalZeroLayer_[ HcalSection::BACK   ] = ecal_front_z + hcalNStrips_[ HcalSection::TOP ] * hcalWidthScint_;
        hcalZeroLayer_[ HcalSection::TOP    ] = ecal_xy/2.;
        hcalZeroLayer_[ HcalSection::BOTTOM ] = ecal_xy/2.;
        hcalZeroLayer_[ HcalSection::LEFT   ] = ecal_xy/2.;
        hcalZeroLayer_[ HcalSection::RIGHT  ] = ecal_xy/2.;
         
        hcalZeroStrip_[ HcalSection::BACK   ] = back_transverse_width/2.; 
        hcalZeroStrip_[ HcalSection::TOP    ] = ecal_front_z;
        hcalZeroStrip_[ HcalSection::BOTTOM ] = ecal_front_z;
        hcalZeroStrip_[ HcalSection::LEFT   ] = ecal_front_z;
        hcalZeroStrip_[ HcalSection::RIGHT  ] = ecal_front_z;

        // absorber + scintillator + 2*air
        hcalLayerThickness_[ HcalSection::BACK   ] = 25. + hcalThicknessScint_ + 2*2.;
        hcalLayerThickness_[ HcalSection::TOP    ] = 20. + hcalThicknessScint_ + 2*2.;
        hcalLayerThickness_[ HcalSection::BOTTOM ] = 20. + hcalThicknessScint_ + 2*2.;
        hcalLayerThickness_[ HcalSection::LEFT   ] = 20. + hcalThicknessScint_ + 2*2.;
        hcalLayerThickness_[ HcalSection::RIGHT  ] = 20. + hcalThicknessScint_ + 2*2.;

        ///////////////////////////////////////////////////////////////////////////////////
        // ECAL

        ecalHexRadius_ = 85.;

        ecalHexGap_ = 0.0;

        ecalZeroLayer_ = ecal_front_z;

        ecalNCellsWide_ = 23;

        ecalSiThickness_ = 0.5;

        ecalDepth_ = 290.0;

        //TODO Recalculate these planes automatically
        ecalSiPlanes_ = {
            4.550, 7.300, 13.800, 18.200, 26.050, 31.950, 41.050, 47.450, 56.550, 62.950,
            72.050, 78.450, 87.550, 93.950, 103.050, 109.450, 118.550, 124.950, 134.050,
            140.450, 149.550, 155.950, 165.050, 171.450, 184.050, 193.950, 206.550, 216.450,
            229.050, 238.950, 251.550, 261.450, 274.050, 283.950
        }; // With respect to the front face of the ECAL

        //Helper Class for Hex Readout
        ecalHexReader_ = std::make_unique<EcalHexReadout>( ecalHexRadius_ , ecalHexGap_, ecalNCellsWide_ );

        ecalXYTower_.emplace_back( 0.0 , 0.0 );
        for ( int towerIndex = 0; towerIndex < 6; towerIndex++ ) {
            ecalXYTower_.emplace_back( 
                        sin( M_PI/3 * towerIndex)*( 2*ecalHexRadius_ + ecalHexGap_ ),
                        cos( M_PI/3 * towerIndex)*( 2*ecalHexRadius_ + ecalHexGap_ )
                    );
        }
        

        /////////////////////////////////////////////////////////////
        // RECOIL TRACKER
        //      The gdml file for the recoil tracker is kinda opaque.
        //      The layer and module IDs are calculated from the copy number of each of the sensor volumes.
        //          layer  = copyNum / 10 (integer division)
        //          module = copyNum % 10
        //      The first 8 layer IDs are the first 4 layers of stereo sensors.
        //          Each stereo layer contains a front layer that is not tilted at an angle and a back layer that is tilted.
        //      The last 2 layer IDs correspond to the 2 layers of mono sensors.
        //          Each mono layer contains 10 modules (ids 0 - 9) that have a complicated position arrangement.
        //
        //      In order to avoid mistakes, the position and angle of each module will be hard coded here instead of
        //      calculated from design specifications like the HCAL case.

        recoilStereoStripLength_ = 98.0;

        recoilStereoXWidth_ = 40.34;

        recoilStereoYWidth_ = 100.0;

        recoilStereoSeparation_ = 3.0;

        recoilStereoAngle_ = 0.1;

        recoilMonoStripLength_ = 78.0;

        recoilMonoXWidth_ = 50.0;

        recoilMonoYWidth_ = 80.0;

        recoilMonoSeparation_ = 1.0;

        recoilSensorThickness_ = 0.52;

        //The following keys for the position and angle maps should correspond to the copynumber in the recoil.gdml file
        //At writing, the layerIDs and moduleIDs are set in the simulation from this copy number (TrackerSD.cxx in SimApplication)
        
        std::vector<double> recoilStereoLayerZPos = {
            7.5, 22.5, 37.5, 52.5
        };

        recoilModulePos_[10]  = { 0 , 0 , recoilStereoLayerZPos.at(0) - recoilStereoSeparation_ };
        recoilModulePos_[20]  = { 0 , 0 , recoilStereoLayerZPos.at(0) + recoilStereoSeparation_ };
           
        recoilModulePos_[30]  = { 0 , 0 , recoilStereoLayerZPos.at(1) - recoilStereoSeparation_ };
        recoilModulePos_[40]  = { 0 , 0 , recoilStereoLayerZPos.at(1) + recoilStereoSeparation_ };
           
        recoilModulePos_[50]  = { 0 , 0 , recoilStereoLayerZPos.at(2) - recoilStereoSeparation_ };
        recoilModulePos_[60]  = { 0 , 0 , recoilStereoLayerZPos.at(2) + recoilStereoSeparation_ };
           
        recoilModulePos_[70]  = { 0 , 0 , recoilStereoLayerZPos.at(3) - recoilStereoSeparation_ };
        recoilModulePos_[80]  = { 0 , 0 , recoilStereoLayerZPos.at(3) + recoilStereoSeparation_ };
           
        std::vector<double> recoilMonoLayerZPos = { 
            90.0, 180.0 
        };

        recoilModulePos_[90]  = { 2*recoilMonoXWidth_ , 0.5*recoilMonoYWidth_ , recoilMonoLayerZPos.at(0) + recoilMonoSeparation_ };
        recoilModulePos_[91]  = {   recoilMonoXWidth_ , 0.5*recoilMonoYWidth_ , recoilMonoLayerZPos.at(0) - recoilMonoSeparation_ };
        recoilModulePos_[92]  = {                 0.0 , 0.5*recoilMonoYWidth_ , recoilMonoLayerZPos.at(0) + recoilMonoSeparation_ };
        recoilModulePos_[93]  = {-1*recoilMonoXWidth_ , 0.5*recoilMonoYWidth_ , recoilMonoLayerZPos.at(0) - recoilMonoSeparation_ };
        recoilModulePos_[94]  = {-2*recoilMonoXWidth_ , 0.5*recoilMonoYWidth_ , recoilMonoLayerZPos.at(0) + recoilMonoSeparation_ };
        recoilModulePos_[95]  = { 2*recoilMonoXWidth_ ,-0.5*recoilMonoYWidth_ , recoilMonoLayerZPos.at(0) + recoilMonoSeparation_ };
        recoilModulePos_[96]  = {   recoilMonoXWidth_ ,-0.5*recoilMonoYWidth_ , recoilMonoLayerZPos.at(0) - recoilMonoSeparation_ };
        recoilModulePos_[97]  = {                 0.0 ,-0.5*recoilMonoYWidth_ , recoilMonoLayerZPos.at(0) + recoilMonoSeparation_ };
        recoilModulePos_[98]  = {-1*recoilMonoXWidth_ ,-0.5*recoilMonoYWidth_ , recoilMonoLayerZPos.at(0) - recoilMonoSeparation_ };
        recoilModulePos_[99]  = {-2*recoilMonoXWidth_ ,-0.5*recoilMonoYWidth_ , recoilMonoLayerZPos.at(0) + recoilMonoSeparation_ };

        recoilModulePos_[100] = { 2*recoilMonoXWidth_ , 0.5*recoilMonoYWidth_ , recoilMonoLayerZPos.at(1) + recoilMonoSeparation_ };
        recoilModulePos_[101] = {   recoilMonoXWidth_ , 0.5*recoilMonoYWidth_ , recoilMonoLayerZPos.at(1) - recoilMonoSeparation_ };
        recoilModulePos_[102] = {                 0.0 , 0.5*recoilMonoYWidth_ , recoilMonoLayerZPos.at(1) + recoilMonoSeparation_ };
        recoilModulePos_[103] = {-1*recoilMonoXWidth_ , 0.5*recoilMonoYWidth_ , recoilMonoLayerZPos.at(1) - recoilMonoSeparation_ };
        recoilModulePos_[104] = {-2*recoilMonoXWidth_ , 0.5*recoilMonoYWidth_ , recoilMonoLayerZPos.at(1) + recoilMonoSeparation_ };
        recoilModulePos_[105] = { 2*recoilMonoXWidth_ ,-0.5*recoilMonoYWidth_ , recoilMonoLayerZPos.at(1) + recoilMonoSeparation_ };
        recoilModulePos_[106] = {   recoilMonoXWidth_ ,-0.5*recoilMonoYWidth_ , recoilMonoLayerZPos.at(1) - recoilMonoSeparation_ };
        recoilModulePos_[107] = {                 0.0 ,-0.5*recoilMonoYWidth_ , recoilMonoLayerZPos.at(1) + recoilMonoSeparation_ };
        recoilModulePos_[108] = {-1*recoilMonoXWidth_ ,-0.5*recoilMonoYWidth_ , recoilMonoLayerZPos.at(1) - recoilMonoSeparation_ };
        recoilModulePos_[109] = {-2*recoilMonoXWidth_ ,-0.5*recoilMonoYWidth_ , recoilMonoLayerZPos.at(1) + recoilMonoSeparation_ };

        //Recoil Angles
        recoilModuleAngle_[10]  = 0.0;
        recoilModuleAngle_[20]  = recoilStereoAngle_;
           
        recoilModuleAngle_[30]  = 0.0;
        recoilModuleAngle_[40]  = -recoilStereoAngle_;
           
        recoilModuleAngle_[50]  = 0.0;
        recoilModuleAngle_[60]  = recoilStereoAngle_;
           
        recoilModuleAngle_[70]  = 0.0;
        recoilModuleAngle_[80]  = -recoilStereoAngle_;
           
        recoilModuleAngle_[90]  = 0.0;
        recoilModuleAngle_[91]  = 0.0;
        recoilModuleAngle_[92]  = 0.0;
        recoilModuleAngle_[93]  = 0.0;
        recoilModuleAngle_[94]  = 0.0;
        recoilModuleAngle_[95]  = 0.0;
        recoilModuleAngle_[96]  = 0.0;
        recoilModuleAngle_[97]  = 0.0;
        recoilModuleAngle_[98]  = 0.0;
        recoilModuleAngle_[99]  = 0.0;

        recoilModuleAngle_[100] = 0.0;
        recoilModuleAngle_[101] = 0.0;
        recoilModuleAngle_[102] = 0.0;
        recoilModuleAngle_[103] = 0.0;
        recoilModuleAngle_[104] = 0.0;
        recoilModuleAngle_[105] = 0.0;
        recoilModuleAngle_[106] = 0.0;
        recoilModuleAngle_[107] = 0.0;
        recoilModuleAngle_[108] = 0.0;
        recoilModuleAngle_[109] = 0.0;

        //TODO Tagger
        //TODO Trigger Pad

    }

    BoundingBox DetectorGeometry::getBoundingBox( HcalHit* hit ) const {
        
        //pairs that will go into BoundingBox
        std::pair<double,double> X(0,0), Y(0,0), Z(0,0);

        HcalSection section = (HcalSection)( hit->getSection() );
        int layer = hit->getLayer();
        int strip = hit->getStrip();

        //calculate center of layer,strip with respect to detector section
        double layercenter = layer*hcalLayerThickness_.at( section ) + 0.5*hcalThicknessScint_;
        double stripcenter = (strip + 0.5)*hcalWidthScint_;

        //calculate error in layer,strip position
        double elayer = 0.5*hcalThicknessScint_;
        double estrip = 0.5*hcalWidthScint_;
        
        double x,y,z;
        if ( section == HcalSection::BACK ) {
            
            z = hcalZeroLayer_.at( section ) + layercenter;
            Z.first  = z-elayer;
            Z.second = z+elayer;
            
            //only horizontal layers implemented currently
            if ( false ) { //( (layer ^ hcalParityVertical_) & 1) == 0 ) { //checks for same parity
                //Vertical Layers
                
                x = hcalZeroStrip_.at( section ) + stripcenter;
                X.first  = x - estrip;
                X.second = x + estrip;
                
                y = hit->getY();
                Y.first  = y - hcalUncertaintyTimingPos_;
                Y.second = y + hcalUncertaintyTimingPos_;

            } else {
                //Horizontal Layers
                
                x = hit->getX();
                X.first  = x - hcalUncertaintyTimingPos_;
                X.second = x + hcalUncertaintyTimingPos_;

                y = hcalZeroStrip_.at( section ) + stripcenter;
                Y.first  = y - estrip;
                Y.second = y + estrip;

            } //calculate depending on layer

        } else {
            
            z = hcalZeroStrip_.at( section ) + stripcenter;
            Z.first  = z - estrip;
            Z.second = z + estrip;

            if ( section == HcalSection::TOP or section == HcalSection::BOTTOM ) {
                
                x = hit->getX();
                X.first  = x - hcalUncertaintyTimingPos_;
                X.second = x + hcalUncertaintyTimingPos_;
                
                if ( section == HcalSection::TOP ) {
                    y = hcalZeroLayer_.at( section ) + layercenter;
                } else {
                    y = hcalZeroLayer_.at( section ) - layercenter;
                } //top or bottom hcal

                Y.first  = y - elayer;
                Y.second = y + elayer;
                
            } else if ( section == HcalSection::LEFT or section == HcalSection::RIGHT ) {
                
                y = hit->getY();
                Y.first  = y - hcalUncertaintyTimingPos_;
                Y.second = y + hcalUncertaintyTimingPos_;

                if ( section == HcalSection::LEFT ) {
                    x = hcalZeroLayer_.at( section ) + layercenter;
                } else {
                    x = hcalZeroLayer_.at( section ) - layercenter;
                } //left or right hcal

                X.first  = x - elayer;
                X.second = x + elayer;
    
            } else {
                std::cerr << "[ DetectorGeometry::getBoundingBox ] : Unknown Hcal Section!" << std::endl;
                std::cerr << "    Returning a valid BoundingBox but with values that are all zero." << std::endl;
            } //side hcal
        
        } //calculate depending on section

        BoundingBox hbox;
        hbox.push_back( X );
        hbox.push_back( Y );
        hbox.push_back( Z );
        return hbox;
    }
    
    BoundingBox DetectorGeometry::getBoundingBox( const std::vector<HcalHit*>  &hitVec ) const {
        
        std::vector<double> pointSum ( 3 , 0.0 ); //sums of weighted coordinates
        std::vector<double> weightSum( 3 , 0.0 ); //sums of weights for each coordinate
        
        //calculate real space point for each hit
        for ( HcalHit* hit : hitVec ) {
            
            BoundingBox box = getBoundingBox( hit );
            
            //Add weighted values to sums
            double weight;
            for ( unsigned int iC = 0; iC < 3; iC++ ) {
                
                double cer = abs(box[iC].second - box[iC].first)/2.0;

                weight = 1.0 / ( cer*cer );
                weightSum[ iC ] += weight;
                pointSum[ iC ] += weight*( ( box[iC].second + box[iC].first )/2.0 );
            }
        } //go through hitVec
        
        //Construct final BoundingBox
        BoundingBox hbox;
        for ( int iC = 0; iC < 3; iC++ ) {
            double c = pointSum[ iC ] / weightSum[ iC ];
            double ec = 1.0 / sqrt( weightSum[ iC ] );
            hbox.emplace_back( c - ec , c + ec );
        }

        return hbox;
    }

    BoundingBox DetectorGeometry::getBoundingBox( HcalSection section ) const {

        std::pair< double, double > X(0,0), Y(0,0), Z(0,0);

        double total_strip_width = hcalNStrips_.at( section ) * hcalWidthScint_;
        double total_thickness = hcalNLayers_.at( section ) * hcalLayerThickness_.at( section );
        if ( section == HcalSection::BACK ) {
           
            X.first  = -hcalZeroStrip_.at( HcalSection::BACK );
            X.second = X.first + total_strip_width;

            Y.first  = -hcalLengthScint_.at( HcalSection::BACK )/2.0;
            Y.second =  hcalLengthScint_.at( HcalSection::BACK )/2.0;

            Z.first  = hcalZeroLayer_.at( HcalSection::BACK );
            Z.second = Z.first + total_thickness;

        } else {

            Z.first  = hcalZeroStrip_.at( section );
            Z.second = Z.first + total_strip_width;

            if ( section == HcalSection::LEFT ) {
                
                X.first  = hcalZeroLayer_.at( HcalSection::LEFT );
                X.second = X.first + total_thickness;

                Y.second = hcalZeroLayer_.at( HcalSection::TOP );
                Y.first  = Y.second - hcalLengthScint_.at( HcalSection::LEFT );

            } else if ( section == HcalSection::RIGHT ) {

                X.second = -hcalZeroLayer_.at( HcalSection::RIGHT );
                X.first  = X.second - total_thickness;

                Y.first  = -hcalZeroLayer_.at( HcalSection::BOTTOM );
                Y.second = Y.first + hcalLengthScint_.at( HcalSection::RIGHT );

            } else if ( section == HcalSection::TOP ) {

                Y.first  = hcalZeroLayer_.at( HcalSection::TOP );
                Y.second = Y.first + total_thickness;

                X.first  = -hcalZeroLayer_.at( HcalSection::RIGHT );
                X.second = X.first + hcalLengthScint_.at( HcalSection::TOP );

            } else if ( section == HcalSection::BOTTOM ) {

                Y.second = -hcalZeroLayer_.at( HcalSection::BOTTOM );
                Y.first  = Y.second - total_thickness;

                X.second = hcalZeroLayer_.at( HcalSection::LEFT );
                X.first  = X.second - hcalLengthScint_.at( HcalSection::BOTTOM );

            } else {
                std::cerr << "[ Warning ] : Unrecognized HcalSection in DetectorGeometry::getBoundingBox." << std::endl;
                std::cerr << "    Will return an incorrect geometry description!" << std::endl;
            }
        }

        BoundingBox boundingbox;
        boundingbox.push_back( X );
        boundingbox.push_back( Y );
        boundingbox.push_back( Z );

        return boundingbox;
    }

    HexPrism DetectorGeometry::getHexPrism( unsigned int cellID , unsigned int moduleID , int layer ) const {

        unsigned int combinedID = ecalHexReader_->combineID( cellID , moduleID );

        XYCoords xy = ecalHexReader_->getCellCenterAbsolute( combinedID );

        HexPrism hexpris;
        hexpris.x = xy.first;
        hexpris.y = xy.second;
        hexpris.z = ecalZeroLayer_ + ecalSiPlanes_.at( layer );
        hexpris.height = ecalSiThickness_;
        hexpris.radius = ecalHexRadius_ / ecalNCellsWide_;

        return hexpris;
    }

    HexPrism DetectorGeometry::getHexPrism( EcalHit* hit ) const {

        unsigned int hitID = hit->getID();
        unsigned int cellID = hitID >> 15;
        unsigned int moduleID = (hitID << 17) >> 29;
        int layer = hit->getLayer();

        return this->getHexPrism( cellID , moduleID , layer );
    }

    HexPrism DetectorGeometry::getHexPrism( int towerIndex ) const {

        HexPrism hexpris;

        if ( towerIndex < 0 or towerIndex > 6 ) {
            std::cerr << "[ Warning ] : towerIndex " << towerIndex << " out of bounds!" << std::endl;
            std::cerr << "    Will return a malformed HexPrism." << std::endl;
            return hexpris;
        }

        hexpris.x = ecalXYTower_.at( towerIndex ).first;
        hexpris.y = ecalXYTower_.at( towerIndex ).second;
        hexpris.z = ecalZeroLayer_ + ecalDepth_/2;
        hexpris.height = ecalDepth_;
        hexpris.radius = ecalHexRadius_ * 2 / sqrt(3); //need radius to corner, not to side

        return hexpris;
    }

    double DetectorGeometry::getRotAngle( int layerID , int moduleID ) const {
        
        int combined = layerID*10 + moduleID;

        if ( recoilModuleAngle_.find( combined ) == recoilModuleAngle_.end() ) {
            std::cerr << "[ Warning ] : DetectorGeometry::getRotAngle : Input layerID (" << layerID
                << ") and input moduleID (" << moduleID << ") are not included in the geometry!" << std::endl;
            return 0.0;
        }

        return recoilModuleAngle_.at( combined );

    }

    BoundingBox DetectorGeometry::getBoundingBox( int layerID , int moduleID ) const {
        
        int combined = layerID*10 + moduleID;

        BoundingBox bbox;
        if ( recoilModulePos_.find( combined ) == recoilModulePos_.end() ) {
            std::cerr << "[ Warning ] : DetectorGeometry::getBoundingBox : Input layerID (" << layerID
                << ") and input moduleID (" << moduleID << ") are not included in the geometry!" << std::endl;
            return bbox;
        }

        double xWidth = recoilStereoXWidth_;
        double yWidth = recoilStereoStripLength_;
        if ( layerID > 8 ) {
            xWidth = recoilMonoXWidth_;
            yWidth = recoilMonoStripLength_;
        }

        bbox.emplace_back( recoilModulePos_.at(combined).at(0) - xWidth/2. , 
                           recoilModulePos_.at(combined).at(0) + xWidth/2. );
        bbox.emplace_back( recoilModulePos_.at(combined).at(1) - yWidth/2. , 
                           recoilModulePos_.at(combined).at(1) + yWidth/2. ); 
        bbox.emplace_back( recoilModulePos_.at(combined).at(2) - recoilSensorThickness_/2. ,
                           recoilModulePos_.at(combined).at(2) + recoilSensorThickness_/2. );

        return bbox;
    }

    BoundingBox DetectorGeometry::getBoundingBox( SimTrackerHit* recoilHit ) const {
        
        int layerID = recoilHit->getLayerID();
        int moduleID = recoilHit->getModuleID();
        int combined = layerID*10 + moduleID;

        BoundingBox bbox;
        if ( recoilModulePos_.find( combined ) == recoilModulePos_.end() ) {
            std::cerr << "[ Warning ] : DetectorGeometry::getBoundingBox : Input layerID (" << layerID
                << ") and input moduleID (" << moduleID << ") are not included in the geometry!" << std::endl;
            return bbox;
        }

        std::vector<float> hitPos = recoilHit->getPosition();

        double xWidth = 1.0;
        double yWidth = recoilStereoStripLength_;
        if ( layerID > 8 ) {
            yWidth = recoilMonoStripLength_;
        }

        //we have to un-rotate the x-position of the hit, so we can rotate it later with the drawer
        double rotAngle = this->getRotAngle( layerID , moduleID );
        double xPos = hitPos.at(0)*cos( -rotAngle ) - hitPos.at(1)*sin( -rotAngle );

        bbox.emplace_back( xPos - xWidth/2. , 
                           xPos + xWidth/2. );
        bbox.emplace_back( recoilModulePos_.at(combined).at(1) - yWidth/2. , 
                           recoilModulePos_.at(combined).at(1) + yWidth/2. ); 
        bbox.emplace_back( recoilModulePos_.at(combined).at(2) - recoilSensorThickness_/2. ,
                           recoilModulePos_.at(combined).at(2) + recoilSensorThickness_/2. );

        return bbox;
    }
}
