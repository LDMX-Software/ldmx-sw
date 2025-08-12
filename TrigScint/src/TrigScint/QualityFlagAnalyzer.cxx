/**
 * @file QualityFlagAnalyzer.cxx
 * @brief An analyzer drawing the most basic quanities of Trigger Scintillator
 * bars
 * @author Lene Kristian Bryngemark, Stanford University
 */

#include "TrigScint/QualityFlagAnalyzer.h"

namespace trigscint {

QualityFlagAnalyzer::QualityFlagAnalyzer(const std::string& name,
                                         framework::Process& process)
    : Analyzer(name, process) {}

void QualityFlagAnalyzer::configure(framework::config::Parameters& parameters) {
  inputEventCol_ = parameters.getParameter<std::string>("inputEventCollection");
  inputEventPassName_ =
      parameters.getParameter<std::string>("inputEventPassName");
  inputHitCol_ = parameters.getParameter<std::string>("inputHitCollection");
  inputHitPassName_ = parameters.getParameter<std::string>("inputHitPassName");
  peds_ = parameters.getParameter<std::vector<double> >("pedestals");
  gain_ = parameters.getParameter<std::vector<double> >("gain");
  startSample_ = parameters.getParameter<int>("startSample");

  std::cout << " [ QualityFlagAnalyzer ] In configure(), got parameters "
            << "\n\t inputEventCollection = " << inputEventCol_
            << "\n\t inputEventPassName = " << inputEventPassName_
            << "\n\t inputHitCollection = " << inputHitCol_
            << "\n\t inputHitPassName = " << inputHitPassName_
            << "\n\t startSample = " << startSample_
            << "\n\t pedestals[0] = " << peds_[0]
            << "\n\t gain[0] = " << gain_[0] << "\t." << std::endl;

  return;
}

void QualityFlagAnalyzer::analyze(const framework::Event& event) {
  const auto channels{event.getCollection<trigscint::EventReadout>(
      inputEventCol_, inputEventPassName_)};
  const auto hits{event.getCollection<trigscint::TestBeamHit>(
      inputHitCol_, inputHitPassName_)};

  int ev_nb = event.getEventNumber();
  //	while (evNb < 0 ) {
  //  ldmx_log(debug) << "event number = " << evNb << " < 0; incrementing event
  //  number "; evNb++;
  //}
  int n_chan = channels.size();
  ldmx_log(debug) << "in event " << ev_nb << "; nChannels = " << n_chan;

  bool exists_intermediate_pe = false;
  float hit_p_es[nChannels] = {0.};

  // ok. get each channel, and find the associated hit, using bar nb.

  for (auto chan : channels) {
    std::vector<float> q = chan.getQ();
    std::vector<float> q_err = chan.getQError();
    std::vector<int> tdc = chan.getTDC();
    // int nTimeSamp = q.size();
    int bar = chan.getChanID();

    int flag = chan.getQualityFlag();

    // check if this is messing up flag
    for (auto hit : hits) {         // we will be ok even if there is no match
      if (hit.getBarID() == bar) {  //
        flag = hit.getQualityFlag();
        hit_p_es[bar] = hit.getPE();
        if (flag == 0 && bar < 12 && 15 < hit.getPE() && hit.getPE() < 40)
          exists_intermediate_pe = true;
      }
    }

    ldmx_log(debug) << "Got event flag " << flag;

    // if flag = 0, fill for clean versions of the usual event displays
    if (flag == 0 && ev_nb < nEv &&
        bar < nChannels) {  // stick within the predefined histogram array
      for (int i_t = 0; i_t < q.size(); i_t++) {
        ldmx_log(debug) << "in event " << ev_nb << "; channel " << bar
                        << ", got charge[" << i_t << "] = " << q.at(i_t);
        hOut[ev_nb][bar]->SetBinContent(i_t + startSample_, q.at(i_t));
        hOut[ev_nb][bar]->SetBinError(i_t + startSample_, fabs(q_err.at(i_t)));
      }  // if within the number of events to plot individually
    }  // over time samples

    // now select on flags
    // special case: flag = 0 (this will happen to all eventually, so catch it
    // now
    if (flag == 0 && nEvDrawn[nFlags - 1] <
                         nEv) {  // then draw if we haven't collected enough
      int fill_nb = nEvDrawn[nFlags - 1];
      for (int i_t = 0; i_t < q.size(); i_t++) {  // fill this plot with all q
        hOutFlag[nFlags - 1][fill_nb][bar]->SetBinContent(i_t + startSample_,
                                                         q.at(i_t));
        hOutFlag[nFlags - 1][fill_nb][bar]->SetBinError(i_t + startSample_,
                                                       fabs(q_err.at(i_t)));
      }
      // keep track of actual event number
      hOutFlag[nFlags - 1][fill_nb][bar]->GetYaxis()->SetTitle(
          Form("Q, flag 0, chan %i, ev %i [fC]", bar, ev_nb));
      nEvDrawn[nFlags - 1]++;  // update filled event counter for this flag (0)
    }  // if nothing was flagged
    else {                                       // hit was flagged somehow
      for (int i_f = 0; i_f < nFlags - 1; i_f++) {  // do all but the last
        int fill_nb = nEvDrawn[i_f];
        ldmx_log(debug) << "Checking flag " << flags[i_f];
        // we're starting from the high numbers and iteratively subtracting
        if (flag >= flags[i_f]) {
          ldmx_log(debug) << "Checking flag " << flags[i_f];
          if (fill_nb < nEv) {  // then 1. this flag must be raised 2. draw if we
                               // haven't collected enough
            for (int i_t = 0; i_t < q.size(); i_t++) {  // fill this plot with all
                                                     // q
              hOutFlag[i_f][fill_nb][bar]->SetBinContent(i_t + startSample_,
                                                       q.at(i_t));
              hOutFlag[i_f][fill_nb][bar]->SetBinError(i_t + startSample_,
                                                     fabs(q_err.at(i_t)));
            }  // over time samples
            hOutFlag[i_f][fill_nb][bar]->GetYaxis()->SetTitle(
                Form("Q, flag %i, chan %i, ev %i [fC]", flags[i_f], bar, ev_nb));
            nEvDrawn[i_f]++;  // update filled event counter for this flag
          }
          flag -= flags[i_f];  // subtract that flag from sum
        }  // if this flag
      }  // over flags
    }  // if any non-zero flag
  }  // over channels

  // select 15 < PE < 40 events
  if (exists_intermediate_pe &&
      peFillNb < nEv) {  // then 1. this flag must be raised 2. draw if we
                         // haven't collected enough
    ldmx_log(debug) << "Got at least one intermediate PE channel";
    for (auto chan : channels) {
      std::vector<float> q = chan.getQ();
      std::vector<float> q_err = chan.getQError();
      std::vector<int> tdc = chan.getTDC();
      // int nTimeSamp = q.size();
      int bar = chan.getChanID();
      for (int i_t = 0; i_t < q.size(); i_t++) {  // fill this plot with all q
        hOutPE[peFillNb][bar]->SetBinContent(i_t + startSample_, q.at(i_t));
        hOutPE[peFillNb][bar]->SetBinError(i_t + startSample_,
                                           fabs(q_err.at(i_t)));
      }  // over time samples
      hOutPE[peFillNb][bar]->GetYaxis()->SetTitle(
          Form("Q, chan %i, ev %i, PE %.2f", bar, ev_nb, hit_p_es[bar]));
    }  // over channels
    peFillNb++;  // update filled event counter for this flag
  }  // if fill

  return;
}

void QualityFlagAnalyzer::onProcessStart() {
  std::cout << "\n\n Process starts! My analyzer should do something -- like "
               "print this \n\n"
            << std::endl;
  getHistoDirectory();

  int n_time_samp = 40;
  int p_emax = 100;
  int n_p_ebins = 5 * p_emax;
  // float Qmax = PEmax / (6250. / 4.e6);
  // float Qmin = -10;
  // int nQbins = (Qmax - Qmin) / 4;

  ldmx_log(debug) << "Setting up histograms... ";

  for (int i_b = 0; i_b < nChannels; i_b++) {
    hPE[i_b] = new TH1F(Form("hPE_chan%i", i_b), Form(";PE, chan%i", i_b), n_p_ebins,
                       0, p_emax);
  }

  for (int i_e = 0; i_e < nEv; i_e++) {
    for (int i_b = 0; i_b < nChannels; i_b++) {
      hOut[i_e][i_b] =
          new TH1F(Form("hCharge_chan%i_ev%i", i_b, i_e),
                   Form(";time sample; Q, channel %i, event %i [fC]", i_b, i_e),
                   n_time_samp, -0.5, n_time_samp - 0.5);
      hOutPE[i_e][i_b] =
          new TH1F(Form("hCharge_PEcut_chan%i_nb%i", i_b, i_e),
                   Form(";time sample; Q, channel %i, event %i [fC]", i_b, i_e),
                   n_time_samp, -0.5, n_time_samp - 0.5);
      for (int i_f = 0; i_f < nFlags; i_f++) {
        hOutFlag[i_f][i_e][i_b] = new TH1F(
            Form("hCharge_flag%i_chan%i_nb%i", flags[i_f], i_b, i_e),
            Form(";time sample; Q, flag %i, chan %i, ev %i [fC]", i_f, i_b, i_e),
            n_time_samp, -0.5,
            n_time_samp - 0.5);  // less confusing to name them upon actual use
        // hOutFlag[iF][iE][iB] = new TH1F("hCharge_flag",  "",
        // nTimeSamp,-0.5,nTimeSamp-0.5);
      }
    }
  }

  hTDCfireChanvsEvent =
      new TH2F("hTDCfireChanvsEvent", ";channel with TDC < 63;event number",
               nChannels, -0.5, nChannels - 0.5, nEv, 0, nEv);

  peFillNb = 0;
  ldmx_log(debug) << "done setting up histograms";

  return;
}

void QualityFlagAnalyzer::onProcessEnd() { return; }

}  // namespace trigscint

DECLARE_ANALYZER(trigscint::QualityFlagAnalyzer)
