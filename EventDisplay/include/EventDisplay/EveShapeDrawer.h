#ifndef EVENTDISPLAY_EVESHAPEDRAWER_H_
#define EVENTDISPLAY_EVESHAPEDRAWER_H_

#include "TEveStraightLineSet.h"
#include "TEveBox.h"

namespace ldmx {

    class EveShapeDrawer {

        public:

            TEveStraightLineSet* drawHexColumn(Double_t xCenter, Double_t yCenter, Double_t frontZ, Double_t backZ, Double_t h, Int_t color, const char* colName) {
            
                TEveStraightLineSet* lineset = new TEveStraightLineSet(colName);
                // Add the bins
                Double_t x[6], y[6];
                Double_t sqrt_three = sqrt(3);
                Double_t a = h / sqrt_three;
                Double_t xstart = xCenter - a;
                Double_t ystart = yCenter;
            
                // Go around the hexagon
                x[0] = xstart;
                y[0] = ystart;
                x[1] = x[0] + a / 2.0;
                y[1] = y[0] + a * sqrt_three / 2.0;
                x[2] = x[1] + a;
                y[2] = y[1];
                x[3] = x[2] + a / 2.0;
                y[3] = y[1] - a * sqrt_three / 2.0;
                x[4] = x[2];
                y[4] = y[3] - a * sqrt_three / 2.0;
                x[5] = x[1];
                y[5] = y[4];
            
                for (int nline = 0; nline < 6; ++nline) {
            
                    int nextpt = nline+1;
                    if (nline == 5) {
                        nextpt = 0;
                    }
            
                    lineset->AddLine(x[nline], y[nline], frontZ, x[nextpt], y[nextpt], frontZ);
                    lineset->AddLine(x[nline], y[nline], backZ, x[nextpt], y[nextpt], backZ);
                    lineset->AddLine(x[nline], y[nline], frontZ, x[nline], y[nline], backZ);
                }
            
                lineset->SetLineColor(color);
                return lineset;
            }
            
            TEveBox* drawBox(Float_t xPos, Float_t yPos, Float_t frontZ, Float_t xWidth, Float_t yWidth, Float_t backZ, Float_t zRotateAngle, Int_t lineColor, Int_t transparency, const char* name) {
            
                TEveBox *box = new TEveBox(name);
            
                Float_t vs[8][3] = {
                        {xPos-xWidth/2,  yPos-yWidth/2,  frontZ},
                        {xPos-xWidth/2,  yPos+yWidth/2,  frontZ},
                        {xPos+xWidth/2,  yPos+yWidth/2,  frontZ},
                        {xPos+xWidth/2,  yPos-yWidth/2,  frontZ},
                        {xPos-xWidth/2,  yPos-yWidth/2,  backZ},
                        {xPos-xWidth/2,  yPos+yWidth/2,  backZ},
                        {xPos+xWidth/2,  yPos+yWidth/2,  backZ},
                        {xPos+xWidth/2,  yPos-yWidth/2,  backZ}
                };
            
                Float_t rotatedvs[8][3];
            
                for (int m = 0; m < 8; ++m) {
            
                    TVector3 rotatedVec = {vs[m][0],vs[m][1],vs[m][2]};
                    rotatedVec.RotateZ(zRotateAngle);
                    rotatedvs[m][0] = rotatedVec[0];
                    rotatedvs[m][1] = rotatedVec[1];
                    rotatedvs[m][2] = rotatedVec[2];
                }
            
                box->SetVertices(*rotatedvs);
                box->SetLineColor(lineColor);
                box->SetFillColor(lineColor);
                box->SetMainTransparency(transparency);
            
                return box;
            }
    };
}

#endif
