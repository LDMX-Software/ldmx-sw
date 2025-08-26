/**
 * @file EveShapeDrawer.h
 * @author Josh Hiltbrand, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef EVENTDISPLAY_EVESHAPEDRAWER_H_
#define EVENTDISPLAY_EVESHAPEDRAWER_H_

#include <math.h>

#include <iostream>

#include "EventDisplay/DetectorGeometry.h"  //for BoundingBox
#include "Math/TVector3D.h"
#include "TEveGeoShape.h"
#include "TGeoMatrix.h"
#include "TGeoShape.h"
#include "TGeoTube.h"

namespace eventdisplay {

/**
 * @class EveShapeDrawer
 * @brief Helper class for drawing common shapes.
 */
class EveShapeDrawer {
 public:
  /**
   * Get Instance of Drawer
   */
  static EveShapeDrawer& getInstance() {
    static EveShapeDrawer eve_shape_drawer;
    return eve_shape_drawer;
  }

  /**
   * Draw a hexagonal prism.
   *
   * @param xPos x coordinate for center of prism [mm]
   * @param yPos y coordinate for center of prism [mm]
   * @param zPos z coordinate for center of prism [mm]
   * @param xRot rotation around x-axis [degrees]
   * @param yRot rotation around y-axis [degrees]
   * @param zRot rotation around z-axis [degrees]
   * @param h height of prism [mm]
   * @param r radius of prism (center to corner) [mm]
   * @param color color of prism
   * @param transparency transparency of prism
   * @param name name of prism
   */
  TEveGeoShape* drawHexPrism(Double_t xPos, Double_t yPos, Double_t zPos,
                             Double_t xRot, Double_t yRot, Double_t zRot,
                             Double_t h, Double_t r, Int_t color,
                             Int_t transparency, TString name) {
    TGeoCombiTrans* loc_and_orien = new TGeoCombiTrans(
        xPos, yPos, zPos, new TGeoRotation(name, xRot, yRot, zRot));

    TEveGeoShape* hex_prism = new TEveGeoShape(name);
    TGeoShape* tube = new TGeoTube(name, 0, r, h / 2);
    tube->SetUniqueID(uid_++);
    hex_prism->SetShape(tube);
    hex_prism->SetFillColor(color);
    hex_prism->SetMainTransparency(transparency);
    hex_prism->SetNSegments(6);
    hex_prism->SetTransMatrix(*loc_and_orien);

    return hex_prism;
  }

  /**
   * Draw a hexagonal prism.
   *
   * @param HexPrism description of hexagonal prism geometry
   * @param xRot rotation around x-axis [degrees]
   * @param yRot rotation around y-axis [degrees]
   * @param zRot rotation around z-axis [degrees]
   * @param color color of prism
   * @param transparency transparency of prism
   * @param name name of prism
   */
  TEveGeoShape* drawHexPrism(HexPrism prism, Double_t xRot, Double_t yRot,
                             Double_t zRot, Int_t color, Int_t transparency,
                             TString name) {
    return drawHexPrism(prism.x, prism.y, prism.z, xRot, yRot, zRot,
                        prism.height, prism.radius, color, transparency, name);
  }

  /**
   * Draw a rectangular prism
   *
   * @param xPos x coordinate for center of prism [mm]
   * @param yPos y coordinate for center of prism [mm]
   * @param zPos z coordinate for center of prism [mm]
   * @param dX width in x direction [mm]
   * @param dY width in y direction [mm]
   * @param dZ width in z direction [mm]
   * @param xRot rotation around x-axis [degrees]
   * @param yRot rotation around y-axis [degrees]
   * @param zRot rotation around z-axis [degrees]
   * @param color color of prism
   * @param transparency transparency of prism
   * @param name name of prism
   */
  TEveGeoShape* drawRectPrism(Double_t xPos, Double_t yPos, Double_t zPos,
                              Double_t dX, Double_t dY, Double_t dZ,
                              Double_t xRot, Double_t yRot, Double_t zRot,
                              Int_t color, Int_t transparency, TString name) {
    TGeoCombiTrans* loc_and_orien = new TGeoCombiTrans(
        xPos, yPos, zPos, new TGeoRotation(name, xRot, yRot, zRot));

    TEveGeoShape* rect_prism = new TEveGeoShape(name);
    TGeoShape* box = new TGeoBBox(name, dX / 2, dY / 2, dZ / 2);
    box->SetUniqueID(uid_++);
    rect_prism->SetShape(box);
    rect_prism->SetFillColor(color);
    rect_prism->SetMainTransparency(transparency);
    rect_prism->SetTransMatrix(*loc_and_orien);

    return rect_prism;
  }

  /**
   * Draw a rectangular prism
   *
   * @param boundingbox containing description of rectangular prism
   * @param xRot rotation around x-axis [degrees]
   * @param yRot rotation around y-axis [degrees]
   * @param zRot rotation around z-axis [degrees]
   * @param color color of prism
   * @param transparency transparency of prism
   * @param name name of prism
   */
  TEveGeoShape* drawRectPrism(BoundingBox boundingbox, Double_t xRot,
                              Double_t yRot, Double_t zRot, Int_t color,
                              Int_t transparency, TString name) {
    std::vector<double> center(3, 0);
    std::vector<double> widths(3, 0);

    for (unsigned int i_c = 0; i_c < 3; i_c++) {
      center[iC] = (boundingbox[iC].second + boundingbox[iC].first) / 2.0;
      widths[iC] = abs(boundingbox[iC].second - boundingbox[iC].first);
    }

    return drawRectPrism(center[0], center[1], center[2], widths[0], widths[1],
                         widths[2], xRot, yRot, zRot, color, transparency,
                         name);
  }

 private:
  UInt_t uid_ = 0;  //* Unique ID counter for the drawings
};

}  // namespace eventdisplay

#endif
