/**
 * @file TestBeamHitAnalyzer.cxx
 * @brief An analyzer drawing the most basic quanities of Trigger Scintillator
 * bars
 * @author Lene Kristian Bryngemark, Stanford University
 */

#include "TrigScint/TestBeamHitAnalyzer.h"

namespace trigscint {

TestBeamHitAnalyzer::TestBeamHitAnalyzer(const std::string& name,
                                         framework::Process& process)
    : Analyzer(name, process) {}

void TestBeamHitAnalyzer::configure(framework::config::Parameters& parameters) {
  input_col_ = parameters.get<std::string>("inputCollection");
  input_pass_name_ = parameters.get<std::string>("inputPassName");
  peds_ = parameters.get<std::vector<double> >("pedestals");
  start_sample_ = parameters.get<int>("startSample");

  std::cout << " [ TestBeamHitAnalyzer ] In configure(), got parameters "
            << "\n\t inputCollection = " << input_col_
            << "\n\t inputPassName = " << input_pass_name_
            << "\n\t startSample = " << start_sample_
            << "\n\t pedestals[0] = " << peds_[0] << "\t." << std::endl;

  return;
}

void TestBeamHitAnalyzer::analyze(const framework::Event& event) {
  const auto channels{event.getCollection<trigscint::TestBeamHit>(
      input_col_, input_pass_name_)};

  int ev_nb = event.getEventNumber();
  // int nChan = channels.size();
  int lead_bar = -1;
  int sublead_bar = -1;
  float pe_lead = -1;
  float pe_sublead = -1;
  bool exists_intermediate_pe = false;

  for (auto chan : channels) {
    int bar = chan.getBarID();
    float pe = chan.getPE();
    if (ev_nb < n_ev_ && bar < n_channels_) {
      // stick within the predefined histogram array
      h_ev_disp_->Fill(ev_nb, bar, pe);
    }  // if within event display range
    if (pe > pe_lead) {
      pe_sublead = pe_lead;
      sublead_bar = lead_bar;
      pe_lead = pe;
      lead_bar = bar;
    } else if (pe >
               pe_sublead) {  // need a specific check, bars not sorted in
                              // PE so leadPE might be found before or after
      pe_sublead = pe;
      sublead_bar = bar;
    }
    h_pe_[bar]->Fill(pe);
    if (chan.getQualityFlag() == 0 && bar < 12 && 15 < pe && pe < 40)
      exists_intermediate_pe = true;

    // cross talk/correlations
    for (auto chan_probe : channels) {
      int bar_probe = chan_probe.getBarID();
      if (bar_probe >= bar)  // we don't define the lower diagonal of the matrix
                             // of histograms
        h_cross_talk_[bar][bar_probe]->Fill(pe, chan_probe.getPE());
    }

  }  // over channels

  if (exists_intermediate_pe && fill_nb_ < n_ev_) {
    for (auto chan : channels) {
      int bar = chan.getBarID();
      if (bar < 12) {  // stick within the predefined histogram array
        float pe = chan.getPE();
        h_ev_disp_pe_->Fill(fill_nb_, bar, pe);
      }
    }  // if within event display range
    fill_nb_++;
  }  // if PE range triggers writing these event displays

  if (sublead_bar == -1) {
    sublead_bar = lead_bar;
    pe_sublead = pe_lead;
  }
  h_pe_vs_delta_[lead_bar]->Fill(lead_bar - sublead_bar, pe_lead);
  h_delta_pe_vs_delta_[lead_bar]->Fill(lead_bar - sublead_bar,
                                       pe_lead - pe_sublead);

  //	if ( (subleadBar%2) == (leadBar%2) ) // in same layer (or even, hit).
  // skip if we're not seeing a max across layers
  //  return;  -- on the other hand this is evident from the plot: delta is even

  h_pe_max_vs_delta_->Fill(lead_bar - sublead_bar, pe_lead);

  return;
}

void TestBeamHitAnalyzer::onProcessStart() {
  std::cout << "\n\n Process starts! My analyzer should do something -- like "
               "print this \n\n"
            << std::endl;

  getHistoDirectory();

  int n_time_samp = 7;
  int p_emax = 400;
  int n_p_ebins = 2 * p_emax;
  // float Qmax = PEmax / (6250. / 4.e6);
  // float Qmin = -10;
  // int nQbins = (Qmax - Qmin) / 4;

  for (int i_b = 0; i_b < n_channels_; i_b++) {
    h_pe_[i_b] = new TH1F(Form("h_pe_chan%i", i_b), Form(";PE, chan%i", i_b),
                          n_p_ebins, 0, p_emax);
    h_pe_in_clusters_[i_b] =
        new TH1F(Form("h_pe_in_clusters_chan%i", i_b), Form(";PE, chan%i", i_b),
                 n_p_ebins, 0, p_emax);
    h_pe_vs_delta_[i_b] = new TH2F(
        Form("h_pe_vs_delta_chan%i", i_b),
        Form(";#Delta_{barID};PE, chan%i has max PE", i_b), n_channels_ + 1,
        -n_channels_ / 2 - 0.5, n_channels_ / 2 + 0.5, n_p_ebins, 0, p_emax);
    h_delta_pe_vs_delta_[i_b] =
        new TH2F(Form("h_delta_pe_vs_delta_chan%i", i_b),
                 Form(";#Delta_{barID};#Delta_PE, chan%i has max PE", i_b),
                 n_channels_ + 1, -n_channels_ / 2 - 0.5, n_channels_ / 2 + 0.5,
                 n_p_ebins, 0, p_emax);
  }

  // make event displays for events where there are channels with weird event
  // hit PE counts
  for (int i_e = 0; i_e < n_ev_; i_e++) {
    for (int i_b = 0; i_b < n_channels_; i_b++) {
      h_out_[i_e][i_b] =
          new TH1F(Form("hCharge_chan%i_ev%i", i_b, i_e),
                   Form(";time sample; Q, chan %i, ev %i [fC]", i_b, i_e),
                   n_time_samp, -0.5, n_time_samp - 0.5);
    }
  }

  h_pe_max_vs_delta_ =
      new TH2F("h_pe_max_vs_delta_", ";#Delta_{barID};PE, max hit", n_channels_,
               -n_channels_ / 2, n_channels_ / 2, n_p_ebins, 0, p_emax);
  h_ev_disp_ =
      new TH2F(Form("h_ev_disp_ev%i", n_ev_), ";Event number; Bar ID; PE",
               n_ev_, 0.5, n_ev_ + 0.5, n_channels_, -0.5, n_channels_ - 0.5);
  h_ev_disp_pe_ =
      new TH2F("h_ev_disp_pe_cut", ";Event number; Bar ID; PE", n_ev_, 0.5,
               n_ev_ + 0.5, n_channels_, -0.5, n_channels_ - 0.5);

  fill_nb_ = 0;

  for (int i_btag = 0; i_btag < n_channels_; i_btag++) {
    for (int i_bprobe = i_btag; i_bprobe < n_channels_; i_bprobe++) {
      // use one side of diagonal
      h_cross_talk_[i_btag][i_bprobe] =
          new TH2F(Form("h_pe_chan%i_vs_chan%i", i_bprobe, i_btag),
                   Form(";PE, channel %i; PE, channel %i", i_bprobe, i_btag),
                   n_p_ebins, 0, p_emax, n_p_ebins, 0, p_emax);
    }
  }

  return;
}

void TestBeamHitAnalyzer::onProcessEnd() { return; }

}  // namespace trigscint

DECLARE_ANALYZER(trigscint::TestBeamHitAnalyzer)
