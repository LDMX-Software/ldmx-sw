/**
 * @file TestBeamClusterAnalyzer.cxx
 * @brief An analyzer drawing the most basic quanities of Trigger Scintillator
 * bars
 * @author Lene Kristian Bryngemark, Stanford University
 */

#include "TrigScint/TestBeamClusterAnalyzer.h"

namespace trigscint {

TestBeamClusterAnalyzer::TestBeamClusterAnalyzer(const std::string& name,
                                                 framework::Process& process)
    : Analyzer(name, process) {}

void TestBeamClusterAnalyzer::configure(
    framework::config::Parameters& parameters) {
  input_col_ = parameters.get<std::string>("inputCollection");
  input_pass_name_ = parameters.get<std::string>("inputPassName");
  //  wide_input_col_ =
  //  parameters.get<std::string>("3hitInputCollection");
  // wide_input_pass_name_ =
  // parameters.get<std::string>("3hitInputPassName");

  ldmx_log(trace) << "In configure(), got parameters "
                  << "\n\t inputCollection = " << input_col_
                  << "\n\t inputPassName = " << input_pass_name_;
  //        << "\n\t 3hitInputCollection = " << wide_input_col_
  //        << "\n\t 3hitInputPassName = " << wide_input_pass_name_

  return;
}

void TestBeamClusterAnalyzer::analyze(const framework::Event& event) {
  if (!event.exists(input_col_, input_pass_name_)) {
    ldmx_log(info) << "No cluster collection " << input_col_ << "_"
                   << input_pass_name_ << " found. Skipping analysis of event";
    return;
  }

  const auto clusters{event.getCollection<ldmx::TrigScintCluster>(
      input_col_, input_pass_name_)};

  int n1hit = 0;
  int n2hit = 0;
  int n3hit = 0;

  int n_clusters = clusters.size();
  int idx = 0;
  for (auto cluster : clusters) {
    int seed = cluster.getSeed();
    int n_hits = cluster.getNHits();
    if (n_hits == 3)
      n3hit++;
    else if (n_hits == 2)
      n2hit++;
    else if (n_hits == 1)
      n1hit++;

    float pe = cluster.getPE();

    h_pe_in_clusters_[seed]->Fill(pe);

    /* // this requires different implementation. use getHitIDs and use the
    indices in there
       // in a loop over hits in the event to extract the PEs
       // -- later.
    for (auto hits : cluster.getConstituents() )
      h_pe_in_hits_[seed]->Fill(PE);
    */
    // instead of always checking distance between the first two, instead, fill
    // with distance from current to previous this should give us a better idea
    // about if we're dominated by close-by activity in secondaries
    if (idx > 0) {  // look back at the previous cluster when more than one
      h_delta_centroids_->Fill(
          fabs(clusters[idx].getCentroid() - clusters[idx - 1].getCentroid()));
      h_delta_vs_seed_->Fill(
          clusters[idx - 1].getSeed(),
          fabs(clusters[idx].getCentroid() - clusters[idx - 1].getCentroid()));
    }
    idx++;  // increment afterwards
  }  // over clusters

  /*

  if (n2hit)
        hN3N2->Fill((float)n3hit/n2hit);
  if (n1hit) {
        hN3N1->Fill((float)n3hit/n1hit);
        hN2N1->Fill((float)n2hit/n1hit);
  }
  */
  h_n3_n2_->Fill(n2hit, n3hit);
  h_n3_n1_->Fill(n1hit, n3hit);
  h_n2_n1_->Fill(n1hit, n2hit);

  h_n_clusters_->Fill(n_clusters);
  // todo: get hit collection to fill Nhits later?
  h_n_hits_->Fill(3 * n3hit + 2 * n2hit + n1hit);

  return;
}

void TestBeamClusterAnalyzer::onProcessStart() {
  ldmx_log(trace)
      << "\n\n Process starts! My analyzer should do something -- like "
         "print this";

  getHistoDirectory();

  int p_emax = 600;
  int n_p_ebins = 0.25 * p_emax;
  // float Qmax = PEmax / (6250. / 4.e6);
  // float Qmin = -10;
  // int nQbins = (Qmax - Qmin) / 4;

  for (int i_b = 0; i_b < n_channels_; i_b++) {
    h_pe_in_hits_[i_b] =
        new TH1F(Form("hPE_chan%i", i_b), Form(";PE, chan%i", i_b), n_p_ebins,
                 0, p_emax);
    h_pe_in_clusters_[i_b] =
        new TH1F(Form("h_pe_in_clusters_chan%i", i_b), Form(";PE, chan%i", i_b),
                 n_p_ebins, 0, p_emax);
  }

  h_delta_vs_seed_ = new TH2F(
      "h_delta_vs_speed", ";bar_id_{seed};#delta_{centroid}", n_channels_ + 1,
      -0.5, n_channels_ - 0.5, 5 * n_channels_, 0, n_channels_);

  h_delta_centroids_ = new TH1F("h_delta_centroids", ";#delta_{centroid}",
                                5 * n_channels_, -0.5, n_channels_ - 0.5);

  h_n_hits_ = new TH1F(
      "hNHits", "Number of hits in the event; n_{hits}; Events", 10, 0, 10);
  h_n_clusters_ = new TH1F(
      "h_n_clusters", "Number of clusters in the event; n_{clusters}; Events",
      10, 0, 10);

  /*
  hN3N2 = new TH1F("hN3N2", "Ratio of 3-hit to 2-hit clusters;
  n_{3-hit}/n_{2-hit}; Events", 10, 0, 4); hN3N1 = new TH1F("hN3N1", "Ratio of
  3-hit to 1-hit clusters; n_{3-hit}/n_{1-hit}; Events", 10, 0, 4); hN2N1 = new
  TH1F("hN2N1", "Ratio of 2-hit to 1-hit clusters; n_{2-hit}/n_{1-hit}; Events",
  10, 0, 4);
  */
  int n_cl = 6;

  h_n3_n2_ = new TH2F(
      "hN3N2", "Number of 3-hit vs 2-hit clusters; n_{2-hit};n_{3-hit}; Events",
      n_cl, -0.5, n_cl - 0.5, n_cl, -0.5, n_cl - 0.5);
  h_n3_n1_ = new TH2F(
      "hN3N1", "Number of 3-hit vs 1-hit clusters; n_{1-hit};n_{3-hit}; Events",
      n_cl, -0.5, n_cl - 0.5, n_cl, -0.5, n_cl - 0.5);
  h_n2_n1_ = new TH2F(
      "hN2N1", "Number of 2-hit vs 1-hit clusters; n_{1-hit};n_{2-hit}; Events",
      n_cl, -0.5, n_cl - 0.5, n_cl, -0.5, n_cl - 0.5);

  return;
}

void TestBeamClusterAnalyzer::onProcessEnd() { return; }

}  // namespace trigscint

DECLARE_ANALYZER(trigscint::TestBeamClusterAnalyzer)
