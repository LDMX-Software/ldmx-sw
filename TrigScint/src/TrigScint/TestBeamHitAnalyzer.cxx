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
  inputCol_ = parameters.getParameter<std::string>("inputCollection");
  inputPassName_ = parameters.getParameter<std::string>("inputPassName");
  peds_ = parameters.getParameter<std::vector<double> >("pedestals");
  startSample_ = parameters.getParameter<int>("startSample");

  std::cout << " [ TestBeamHitAnalyzer ] In configure(), got parameters "
            << "\n\t inputCollection = " << inputCol_
            << "\n\t inputPassName = " << inputPassName_
            << "\n\t startSample = " << startSample_
            << "\n\t pedestals[0] = " << peds_[0] << "\t." << std::endl;

  return;
}

void TestBeamHitAnalyzer::analyze(const framework::Event& event) {
  const auto channels{
      event.getCollection<trigscint::TestBeamHit>(inputCol_, inputPassName_)};

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
    if (ev_nb < nEv &&
        bar < nChannels) {  // stick within the predefined histogram array
      hEvDisp->Fill(ev_nb, bar, pe);
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
    hPE[bar]->Fill(pe);
    if (chan.getQualityFlag() == 0 && bar < 12 && 15 < pe && pe < 40)
      exists_intermediate_pe = true;

    // cross talk/correlations
    for (auto chan_probe : channels) {
      int bar_probe = chan_probe.getBarID();
      if (bar_probe >= bar)  // we don't define the lower diagonal of the matrix
                             // of histograms
        hCrossTalk[bar][bar_probe]->Fill(pe, chan_probe.getPE());
    }

  }  // over channels

  if (exists_intermediate_pe && fillNb < nEv) {
    for (auto chan : channels) {
      int bar = chan.getBarID();
      if (bar < 12) {  // stick within the predefined histogram array
        float pe = chan.getPE();
        hEvDispPE->Fill(fillNb, bar, pe);
      }
    }  // if within event display range
    fillNb++;
  }  // if PE range triggers writing these event displays

  if (sublead_bar == -1) {
    sublead_bar = lead_bar;
    pe_sublead = pe_lead;
  }
  hPEVsDelta[lead_bar]->Fill(lead_bar - sublead_bar, pe_lead);
  hDeltaPEVsDelta[lead_bar]->Fill(lead_bar - sublead_bar, pe_lead - pe_sublead);

  //	if ( (subleadBar%2) == (leadBar%2) ) // in same layer (or even, hit).
  // skip if we're not seeing a max across layers
  //  return;  -- on the other hand this is evident from the plot: delta is even

  hPEmaxVsDelta->Fill(lead_bar - sublead_bar, pe_lead);

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

  for (int i_b = 0; i_b < nChannels; i_b++) {
    hPE[i_b] = new TH1F(Form("hPE_chan%i", i_b), Form(";PE, chan%i", i_b),
                        n_p_ebins, 0, p_emax);
    hPEinClusters[i_b] =
        new TH1F(Form("hPEinClusters_chan%i", i_b), Form(";PE, chan%i", i_b),
                 n_p_ebins, 0, p_emax);
    hPEVsDelta[i_b] = new TH2F(
        Form("hPEVsDelta_chan%i", i_b),
        Form(";#Delta_{barID};PE, chan%i has max PE", i_b), nChannels + 1,
        -nChannels / 2 - 0.5, nChannels / 2 + 0.5, n_p_ebins, 0, p_emax);
    hDeltaPEVsDelta[i_b] =
        new TH2F(Form("hDeltaPEVsDelta_chan%i", i_b),
                 Form(";#Delta_{barID};#Delta_PE, chan%i has max PE", i_b),
                 nChannels + 1, -nChannels / 2 - 0.5, nChannels / 2 + 0.5,
                 n_p_ebins, 0, p_emax);
  }

  // make event displays for events where there are channels with weird event
  // hit PE counts
  for (int i_e = 0; i_e < nEv; i_e++) {
    for (int i_b = 0; i_b < nChannels; i_b++) {
      hOut[i_e][i_b] =
          new TH1F(Form("hCharge_chan%i_ev%i", i_b, i_e),
                   Form(";time sample; Q, chan %i, ev %i [fC]", i_b, i_e),
                   n_time_samp, -0.5, n_time_samp - 0.5);
    }
  }

  hPEmaxVsDelta =
      new TH2F("hPEmaxVsDelta", ";#Delta_{barID};PE, max hit", nChannels,
               -nChannels / 2, nChannels / 2, n_p_ebins, 0, p_emax);
  hEvDisp = new TH2F(Form("hEvDisp_ev%i", nEv), ";Event number; Bar ID; PE",
                     nEv, 0.5, nEv + 0.5, nChannels, -0.5, nChannels - 0.5);
  hEvDispPE = new TH2F("hEvDispPEcut", ";Event number; Bar ID; PE", nEv, 0.5,
                       nEv + 0.5, nChannels, -0.5, nChannels - 0.5);

  fillNb = 0;

  for (int i_btag = 0; i_btag < nChannels; i_btag++) {
    for (int i_bprobe = i_btag; i_bprobe < nChannels;
         i_bprobe++) {  // use one side of diagonal
      hCrossTalk[i_btag][i_bprobe] =
          new TH2F(Form("hPE_chan%i_vs_chan%i", i_bprobe, i_btag),
                   Form(";PE, channel %i; PE, channel %i", i_bprobe, i_btag),
                   n_p_ebins, 0, p_emax, n_p_ebins, 0, p_emax);
    }
  }

  return;
}

void TestBeamHitAnalyzer::onProcessEnd() { return; }

}  // namespace trigscint

DECLARE_ANALYZER(trigscint::TestBeamHitAnalyzer)
