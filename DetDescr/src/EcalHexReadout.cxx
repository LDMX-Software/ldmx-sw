#include "DetDescr/EcalHexReadout.h"
#include "TList.h"

namespace ldmx {

    EcalHexReadout::EcalHexReadout(double width, double side) {
        ecalMap = new TH2Poly();

        // key assumptions about orientation:
        //   cells are etched flat side down, on modules which are flat side down.
        // geometry:
        //   hexagons have two radii: r (half of flat-to-flat) and R (half of corner-to-corner).
        //     r = (sqrt(3)/2)R = s, where s is length of a flat side.
        //   therefore column-to-column distance in a flat-side-down hex grid is 1.5*R,
        //     and row-to-row distance is 2r = sqrt(3)R.
        //   for seven ecal modules with this orientation, total x and y extents are:
        //     deltaY = 6r' + 2g = 3sqrt(3)R' + 2g
        //     deltaX = 4R' + s' + 2g/cos(30 deg) = (4+sqrt(3)/2)R' + 4g/sqrt(3)
        //     where g is uniform gap width between modules, and primed variables correspond to modules.
        // this ID grid:
        //   input was side length s and total grid width w.
        //   a grid of cells will be constructed and cover at least the areas -w/2 to w/2 in x and y.
        //   therefore USER MUST GUARANTEE THAT w > max(deltaX, deltaY) as defined above.
        //   a cell will be centered at (0,0)

        // imagine a flat-side-down hexagon. start at lower-left point, which is at ( -s/2, -r )
        unsigned ncellwide = TMath::Ceil(width / (1.5 * side));
        unsigned ncelltall = TMath::Ceil(width / (2. * side));
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
