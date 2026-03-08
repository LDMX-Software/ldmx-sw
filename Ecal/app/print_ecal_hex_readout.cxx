
#include <math.h>

#include <any>
#include <map>
#include <string>

#include "DetDescr/EcalGeometry.h"
#include "DetDescr/EcalID.h"
#include "DetDescr/EcalTriggerID.h"
#include "Ecal/EcalTriggerGeometry.h"
#include "Framework/Configure/Parameters.h"
#include "TCanvas.h"  //for dumping map to file
#include "TLine.h"    //for module_ hex border
#include "TStyle.h"   //for no stats box
#include "TText.h"

/**
 * @app ldmx-print-ecal-hex-readout
 *
 * Prints the ecal cell ID <-> position map to
 * a pdf.
 */
int main() {
  // first create the ecal hex readout

  // These are the v12 parameters
  //  all distances in mm
  std::vector<double> ecal_sens_layers_z = {
      7.850,   13.300,  26.400,  33.500,  47.950,  56.550,  72.250,
      81.350,  97.050,  106.150, 121.850, 130.950, 146.650, 155.750,
      171.450, 180.550, 196.250, 205.350, 221.050, 230.150, 245.850,
      254.950, 270.650, 279.750, 298.950, 311.550, 330.750, 343.350,
      362.550, 375.150, 394.350, 406.950, 426.150, 438.750};
  framework::config::Parameters params;
  params.addParameter("layer_z_positions", ecal_sens_layers_z);
  params.addParameter("ecal_front_z", 220.);
  params.addParameter("module_min_r", 85.0);
  params.addParameter("n_cell_r_height", 35.3);
  params.addParameter("gap", 1.5);
  params.addParameter("corners_side_up", false);
  params.addParameter("layer_shift_x", 0.);
  params.addParameter("layer_shift_y", 0.);
  params.addParameter("layer_shift_odd", false);
  params.addParameter("layer_shift_odd_bilayer", false);
  params.addParameter("verbose", 1);

  ldmx::EcalGeometry* geometry_ptr = ldmx::EcalGeometry::debugMake(params);
  ldmx::EcalGeometry& geometry(*geometry_ptr);

  /// fills the poly map with the corresponding IDs and then returns a handle to
  /// it
  auto poly_map = geometry.getCellPolyMap();

  TCanvas* c = new TCanvas("c", "c", 900, 900);  // make square canvas
  c->SetMargin(0.15, 0.05, 0.1, 0.1);
  gStyle->SetOptStat(0);  // no stat box
  poly_map->SetTitle(
      "Local Cell ID to Local Cell Position Map;"
      "P Position Relative to Module [mm];"
      "Q Position Relative to Module [mm]");
  poly_map->GetXaxis()->SetTickLength(0.);
  poly_map->GetYaxis()->SetTickLength(0.);
  poly_map->Draw("TEXT");  // print with bin context labeled as text

  double hex_corner_radius = 85.0 * (2 / sqrt(3));
  std::vector<std::pair<double, double> > hex_corners = {
      std::make_pair(+1. * hex_corner_radius, 0.),
      std::make_pair(+1. * hex_corner_radius * cos(M_PI / 3),
                     +1. * hex_corner_radius * sin(M_PI / 3)),
      std::make_pair(-1. * hex_corner_radius * cos(M_PI / 3),
                     +1. * hex_corner_radius * sin(M_PI / 3)),
      std::make_pair(-1. * hex_corner_radius, 0.),
      std::make_pair(-1. * hex_corner_radius * cos(M_PI / 3),
                     -1. * hex_corner_radius * sin(M_PI / 3)),
      std::make_pair(+1. * hex_corner_radius * cos(M_PI / 3),
                     -1. * hex_corner_radius * sin(M_PI / 3)),
      std::make_pair(+1. * hex_corner_radius, 0.)};
  TLine module_hex_border{0., 0., 0., 0.};
  module_hex_border.SetLineColorAlpha(kRed, 0.5);
  module_hex_border.SetLineWidth(2);
  for (int i = 1; i < hex_corners.size(); i++) {
    module_hex_border.DrawLine(
        hex_corners.at(i - 1).first, hex_corners.at(i - 1).second,
        hex_corners.at(i).first, hex_corners.at(i).second);
  }

  c->Update();
  c->SaveAs("Cell_ID_Cell_Position_Map.pdf");

  poly_map->SetTitle(
      "Local Cell U,V to Local Cell Position Map;X Position Relative to Module "
      "[mm];Y Position Relative to Module [mm]");
  poly_map->GetXaxis()->SetTickLength(0.);
  poly_map->GetYaxis()->SetTickLength(0.);
  poly_map->SetMaximum(1000);
  poly_map->SetMinimum(500);
  poly_map->Draw("hist");  // print with bin context labeled as text

  for (int icell = 0; icell < 432; icell++) {
    ldmx::EcalID id(0, 0, icell);
    std::pair<double, double> pt = geometry.getPositionInModule(icell);
    char text[100];
    std::pair<unsigned int, unsigned int> uv = id.getCellUV();
    sprintf(text, "(%d,%d)", uv.first, uv.second);
    TText* tt = new TText(pt.first, pt.second, text);
    tt->SetTextAlign(22);
    tt->SetTextSize(0.012);
    tt->Draw("SAME");
  }

  c->Update();
  c->SaveAs("Cell_UV_Cell_Position_Map.pdf");

  // and now for triggers
  ecal::EcalTriggerGeometry trig_g(0x100, geometry_ptr);
  poly_map->SetTitle(
      "Trigger Cell Summing Map;"
      "P Position Relative to Module [mm];"
      "Q Position Relative to Module [mm]");
  poly_map->GetXaxis()->SetTickLength(0.);
  poly_map->GetYaxis()->SetTickLength(0.);
  poly_map->SetMaximum(4);
  poly_map->SetMinimum(0);

  for (int icell = 0; icell < 432; icell++) {
    ldmx::EcalID id(0, 0, icell);
    ldmx::EcalTriggerID tid = trig_g.belongsTo(id);

    std::cout << id << "->" << tid << std::endl;

    int ival;
    if (tid.triggercell() < 16)
      ival = 1 + tid.triggercell() % 3;
    else if (tid.triggercell() < 20)
      ival = 1 + (tid.triggercell() + 1) % 3;
    else if (tid.triggercell() < 24)
      ival = 1 + (tid.triggercell() - 1) % 3;
    else if (tid.triggercell() < 28)
      ival = 1 + (tid.triggercell()) % 3;
    else if (tid.triggercell() < 32)
      ival = 1 + (tid.triggercell() + 1) % 3;
    else {
      switch (tid.triggercell()) {
        case (32):
          ival = 2;
          break;
        case (33):
          ival = 3;
          break;
        case (34):
          ival = 1;
          break;
        case (35):
          ival = 0;
          break;
        case (36):
          ival = 2;
          break;
        case (37):
          ival = 3;
          break;
        case (38):
          ival = 2;
          break;
        case (39):
          ival = 1;
          break;
        case (40):
          ival = 0;
          break;
        case (41):
          ival = 2;
          break;
        case (42):
          ival = 0;
          break;
        case (43):
          ival = 2;
          break;
        case (44):
          ival = 3;
          break;
        case (45):
          ival = 3;
          break;
        case (46):
          ival = 1;
          break;
        case (47):
          ival = 0;
          break;
        default:
          ival = (tid.triggercell() % 4);
      }
    }

    poly_map->SetBinContent(icell + 1, ival);
  }

  poly_map->Draw("COL");  // print with bin context labeled as text

  for (int tcell = 0; tcell < 48; tcell++) {
    ldmx::EcalTriggerID tid(0, 0, tcell);
    std::pair<double, double> pt = trig_g.localPosition(tid);

    char text[100];
    sprintf(text, "(%d)", tcell);
    TText* tt = new TText(pt.first, pt.second, text);
    tt->SetTextAlign(22);
    tt->SetTextSize(0.012);
    tt->Draw("SAME");
  }

  c->Update();
  c->SaveAs("TriggerCell_Position_Map.pdf");

  return 0;
}
