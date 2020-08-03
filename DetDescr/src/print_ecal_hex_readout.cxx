
#include "DetDescr/EcalHexReadout.h"

#include "TCanvas.h" //for dumping map to file
#include "TStyle.h" //for no stats box
#include "TLine.h" //for module hex border

#include <math.h>

/**
 * @app ldmx-print-ecal-hex-readout
 *
 * Prints the ecal cell ID <-> position map to
 * a pdf.
 */
int main() {

    //first create the ecal hex readout

    // These are the v12 parameters
    //  all distances in mm
    double moduleRadius = 85.0;
    double numCellRHeight = 35.2; 
    double moduleGap = 1.5;
    double ecalFrontZ = 220;
    std::vector<double> ecalSensLayersZ = {
         7.850,
        13.300,
        26.400,
        33.500,
        47.950,
        56.550,
        72.250,
        81.350,
        97.050,
        106.150,
        121.850,
        130.950,
        146.650,
        155.750,
        171.450,
        180.550,
        196.250,
        205.350,
        221.050,
        230.150,
        245.850,
        254.950,
        270.650,
        279.750,
        298.950,
        311.550,
        330.750,
        343.350,
        362.550,
        375.150,
        394.350,
        406.950,
        426.150,
        438.750
    };

    ldmx::EcalHexReadout hexReadout(
            moduleRadius,
            moduleGap,
            numCellRHeight,
            ecalSensLayersZ,
            ecalFrontZ
            );

    auto polyMap = hexReadout.getCellPolyMap();

    //fill poly map with IDs
    for ( auto const& [ cellID , cellCenter ] : hexReadout.getCellPositionMap() ) {
        polyMap->Fill( cellCenter.first , cellCenter.second , cellID );
    }

    TCanvas *c = new TCanvas( "c" , "c" , 900 , 800 ); //make square canvas
    c->SetMargin( 0.1 , 0.1 , 0.1 , 0.1 );
    gStyle->SetOptStat(0); //no stat box
    polyMap->SetTitle( "Local Cell ID to Local Cell Position Map;X Position Relative to Module [mm];Y Position Relative to Module [mm]" );
    polyMap->GetXaxis()->SetTickLength(0.);
    polyMap->GetYaxis()->SetTickLength(0.);
    polyMap->Draw( "TEXT" ); //print with bin context labeled as text

    double hexCornerRadius = 85.0*(2/sqrt(3));
    std::vector< std::pair< double,double> > hexCorners = {
        std::make_pair<double,double>(+1.*hexCornerRadius , 0.),
        std::make_pair<double,double>(+1.*hexCornerRadius*cos(M_PI/3),+1.*hexCornerRadius*sin(M_PI/3)),
        std::make_pair<double,double>(-1.*hexCornerRadius*cos(M_PI/3),+1.*hexCornerRadius*sin(M_PI/3)),
        std::make_pair<double,double>(-1.*hexCornerRadius,0.),
        std::make_pair<double,double>(-1.*hexCornerRadius*cos(M_PI/3),-1.*hexCornerRadius*sin(M_PI/3)),
        std::make_pair<double,double>(+1.*hexCornerRadius*cos(M_PI/3),-1.*hexCornerRadius*sin(M_PI/3)),
        std::make_pair<double,double>(+1.*hexCornerRadius , 0.)
    };
    TLine moduleHexBorder;
    moduleHexBorder.SetLineColor( kRed );
    moduleHexBorder.SetLineWidth( 2 );
    for ( int i = 1; i < hexCorners.size(); i++ ) {
        moduleHexBorder.DrawLine(
                hexCorners.at(i-1).first , hexCorners.at(i-1).second , 
                hexCorners.at(i).first , hexCorners.at(i).second 
                );
    }

    c->Update();
    c->SaveAs( "Cell_ID_Cell_Position_Map.pdf" );

    return 0;
}
