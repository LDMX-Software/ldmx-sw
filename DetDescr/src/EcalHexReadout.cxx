#include "DetDescr/EcalHexReadout.h"
#include "TList.h"

namespace ldmx {

    EcalHexReadout::EcalHexReadout(double width, double side) {
        ecalMap = new TH2Poly();

        // Center a cell at (x,y)=(0,0) and ensure coverage up to/past width/2 in all 4 directions,
        // assuming each cell is lying on a side.
        unsigned ncellwide = width / (1.5 * side);
        unsigned ny = ncellwide + 1;
        unsigned nx = ncellwide + 4;
        double xstart = -((double) ncellwide + 0.5) * side;
        double ystart = -((double) ncellwide + 1) * side * sqrt(3) / 2;

        // std::cout << " -- Initialising HoneyComb with parameters: " << std::endl << " ---- (xstart,ystart) = (" << xstart << "," << ystart << ")" << ", side = " << side << ", nx = " << nx << ", ny=" << ny << std::endl;

        buildMap(xstart, ystart, side, ny, nx);

        TListIter next(ecalMap->GetBins());
        TObject *obj = 0;
        TH2PolyBin *polyBin = 0;

        while ((obj = next())) {
            polyBin = (TH2PolyBin*) obj;
            int id = polyBin->GetBinNumber();
            double x = (polyBin->GetXMax() + polyBin->GetXMin()) / 2.;
            double y = (polyBin->GetYMax() + polyBin->GetYMin()) / 2.;
            cellIdtoCoords.insert(std::pair<int,XYCoords>(id, std::make_pair(x, y)));
        }
    }

    void EcalHexReadout::buildMap(Double_t xstart, Double_t ystart, Double_t a, Int_t k, Int_t s) {
        // Add the bins
        Double_t numberOfHexagonsInAColumn;
        Double_t x[6], y[6];
        Double_t xloop, yloop, ytemp;
        Double_t sqrt_three = sqrt(3);
        xloop = xstart;
        yloop = ystart + a * sqrt_three / 2.0;

        for (int sCounter = 0; sCounter < s; sCounter++) {
            ytemp = yloop; // Resets the temp variable

            // Determine the number of hexagons in that column
            if (sCounter % 2 == 0) {
                numberOfHexagonsInAColumn = k;
            } else {
                numberOfHexagonsInAColumn = k - 1;
            }

            for (int kCounter = 0; kCounter < numberOfHexagonsInAColumn; kCounter++) {

                // Go around the hexagon
                x[0] = xloop;
                y[0] = ytemp;
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
                ecalMap->AddBin(6, x, y);

                // Go up
                ytemp += a * sqrt_three;
            }

            // Increment the starting position
            if (sCounter % 2 == 0)
                yloop += a * sqrt_three / 2.0;
            else
                yloop -= a * sqrt_three / 2.0;
            xloop += 1.5 * a;
        }
    }

    std::vector<int> EcalHexReadout::getInnerRingCellIds(int cellID) {
        return {
            cellID + SHIFT_UP, 
            cellID + SHIFT_DOWN, 
            cellID + SHIFT_RIGHT_UP, 
            cellID + SHIFT_RIGHT_DOWN, 
            cellID + SHIFT_LEFT_UP, 
            cellID + SHIFT_LEFT_DOWN
        };
    }

    std::vector<int> EcalHexReadout::getOuterRingCellIds(int cellID) {
        return {
            cellID + 2*SHIFT_UP, 
            cellID + SHIFT_RIGHT_UP + SHIFT_UP, 
            cellID + SHIFT_LEFT_UP + SHIFT_UP, 
            cellID + 2*SHIFT_DOWN, 
            cellID + SHIFT_RIGHT_DOWN + SHIFT_DOWN, 
            cellID + SHIFT_LEFT_DOWN + SHIFT_DOWN, 
            cellID + 2*SHIFT_RIGHT_UP, 
            cellID + 2*SHIFT_RIGHT_UP + SHIFT_DOWN, 
            cellID + 2*SHIFT_RIGHT_UP + SHIFT_DOWN, 
            cellID + 2*SHIFT_LEFT_UP, 
            cellID + 2*SHIFT_LEFT_UP + SHIFT_DOWN, 
            cellID + 2*SHIFT_LEFT_UP + SHIFT_DOWN
        };
    }

}
