#ifndef EVENTDISPLAY_EVESHAPEDRAWER_H_
#define EVENTDISPLAY_EVESHAPEDRAWER_H_

#include "TEveGeoShape.h"
#include "TGeoTube.h"
#include "TGeoShape.h"
#include "TGeoMatrix.h"

#include "DetDescr/DetectorGeometry.h" //for BoundingBox

#include "TVector3.h"
#include <math.h>
#include <iostream>

namespace ldmx {

    class EveShapeDrawer {

        public:

            TEveGeoShape* drawHexPrism(Double_t xPos, Double_t yPos, Double_t zPos, Double_t xRot, Double_t yRot, Double_t zRot, Double_t h, Double_t r, Int_t color, Int_t transparency, TString name) {
            
                TGeoCombiTrans* locAndOrien = new TGeoCombiTrans(xPos, yPos, zPos, new TGeoRotation(name, xRot, yRot, zRot));

                TEveGeoShape* hexPrism = new TEveGeoShape(name);
                TGeoShape* tube = new TGeoTube(name, 0, r, h/2);
                tube->SetUniqueID(uid_++);
                hexPrism->SetShape(tube);
                hexPrism->SetFillColor(color);
                hexPrism->SetMainTransparency(transparency);
                hexPrism->SetNSegments(6);
                hexPrism->SetTransMatrix(*locAndOrien);

                return hexPrism;
            }

            
            TEveGeoShape* drawHexPrism(HexPrism prism, Double_t xRot, Double_t yRot, Double_t zRot, Int_t color, Int_t transparency, TString name) {

                return drawHexPrism(
                            prism.x , prism.y , prism.z,
                            xRot , yRot , zRot ,
                            prism.height , prism.radius ,
                            color , transparency , name
                        );
            }

            TEveGeoShape* drawRectPrism(Double_t xPos, Double_t yPos, Double_t zPos, Double_t dX, Double_t dY, Double_t dZ, Double_t xRot, Double_t yRot, Double_t zRot, Int_t color, Int_t transparency, TString name) {
        
                TGeoCombiTrans* locAndOrien = new TGeoCombiTrans(xPos, yPos, zPos, new TGeoRotation(name, xRot, yRot, zRot));

                TEveGeoShape* rectPrism = new TEveGeoShape(name);
                TGeoShape* box = new TGeoBBox(name, dX/2, dY/2, dZ/2);
                box->SetUniqueID(uid_++);
                rectPrism->SetShape(box);
                rectPrism->SetFillColor(color);
                rectPrism->SetMainTransparency(transparency);
                rectPrism->SetTransMatrix(*locAndOrien);
            
                return rectPrism;
            }

            TEveGeoShape* drawRectPrism( BoundingBox boundingbox , Double_t xRot, Double_t yRot, Double_t zRot, Int_t color, Int_t transparency, TString name) {
                
                std::vector<double> center(3,0);
                std::vector<double> widths(3,0);

                for ( unsigned int iC = 0; iC < 3; iC++ ) {
                   center[iC] =    (boundingbox[iC].second + boundingbox[iC].first)/2.0;
                   widths[iC] = abs(boundingbox[iC].second - boundingbox[iC].first);
                }

                return drawRectPrism(
                        center[0] , center[1] , center[2],
                        widths[0] , widths[1] , widths[2],
                        xRot , yRot , zRot,
                        color, transparency, name
                        );
            }

        private:

            UInt_t uid_ = 0;

    };
}

#endif
