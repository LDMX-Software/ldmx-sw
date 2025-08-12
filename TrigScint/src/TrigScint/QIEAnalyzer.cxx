/**
 * @file QIEAnalyzer.cxx
 * @brief An analyzer drawing the most basic quanities of Trigger Scintillator
 * bars
 * @author Lene Kristian Bryngemark, Stanford University
 */

#include "TrigScint/QIEAnalyzer.h"

namespace trigscint {

QIEAnalyzer::QIEAnalyzer(const std::string& name, framework::Process& process)
    : Analyzer(name, process) {}

void QIEAnalyzer::configure(framework::config::Parameters& parameters) {
  inputCol_ = parameters.getParameter<std::string>("inputCollection");
  inputPassName_ = parameters.getParameter<std::string>("inputPassName");
  peds_ = parameters.getParameter<std::vector<double> >("pedestals");
  gain_ = parameters.getParameter<std::vector<double> >("gain");
  startSample_ = parameters.getParameter<int>("startSample");

  std::cout << " [ QIEAnalyzer ] In configure(), got parameters "
            << "\n\t inputCollection = " << inputCol_
            << "\n\t inputPassName = " << inputPassName_
            << "\n\t startSample = " << startSample_
            << "\n\t pedestals[0] = " << peds_[0]
            << "\n\t gain[0] = " << gain_[0] << "\t." << std::endl;

  return;
}

void QIEAnalyzer::analyze(const framework::Event& event) {
  const auto channels{
      event.getCollection<trigscint::EventReadout>(inputCol_, inputPassName_)};

  int ev_nb = event.getEventNumber();
  //	while (evNb < 0 ) {
  //  ldmx_log(debug) << "event number = " << evNb << " < 0; incrementing event
  //  number "; evNb++;
  //}
  int n_chan = channels.size();
  ldmx_log(debug) << "in event " << ev_nb << "; nChannels = " << n_chan;

  for (auto chan : channels) {
    std::vector<float> q = chan.getQ();
    std::vector<float> q_err = chan.getQError();
    std::vector<int> tdc = chan.getTDC();
    // int nTimeSamp = q.size();
    int bar = chan.getChanID();
    float q_tot = 0;
    float q_ped_subtracted_avg = 0;
    int first_t = startSample_ - 1;
    int n_samp_above = 0;
    int n_samp_above_event_ped = 0;
    float subtr_pe = 0;
    float subtr_q = 0;
    float ped = chan.getPedestal();
    for (int i_t = 0; i_t < q.size(); i_t++) {
      ldmx_log(debug) << "in event " << ev_nb << "; channel " << bar
                      << ", got charge[" << i_t << "] = " << q.at(i_t);
      if (ev_nb < nEv &&
          bar < nChannels) {  // stick within the predefined histogram array
        //		  hOut[evNb][bar]->Fill(iT+startSample_, q.at(iT));
        hOut[ev_nb][bar]->SetBinContent(i_t + startSample_, q.at(i_t));
        hOut[ev_nb][bar]->SetBinError(i_t + startSample_, fabs(q_err.at(i_t)));
        if (tdc.at(i_t) < 63) {
          ldmx_log(info) << "Found fired TDC = " << tdc.at(i_t)
                         << " at time sample " << i_t << " in channel " << bar
                         << " and event " << ev_nb;
          // for some reason, the style settings are washed out later...
          hOut[ev_nb][bar]->SetLineColor(kRed + 1);
          hOut[ev_nb][bar]->SetMarkerColor(hOut[ev_nb][bar]->GetLineColor());
          hOut[ev_nb][bar]->SetMarkerSize(0.2);

          if (i_t + startSample_ > 0)
            hTDCfireChanvsEvent->Fill(bar, ev_nb, i_t + startSample_);
          else
            hTDCfireChanvsEvent->Fill(bar, ev_nb);
        }
      }  // if within the number of events to plot individually
      if (q.at(i_t) >
          2 * fabs(peds_[bar])) {  // integrate all charge well above
                                   // ped to convert to a PE count
        q_tot += q.at(i_t);
        q_ped_subtracted_avg +=
            q.at(i_t) - chan.getPedestal();  // peds_[ bar ];
        n_samp_above++;
        ldmx_log(debug) << " above channel overall pedestal: " << q.at(i_t)
                        << " > " << 2 * fabs(peds_[bar]);

        // keep track of first time sample above threshold
        if (first_t == startSample_ - 1) first_t = startSample_ + i_t;
      }  // if above threshold
      if (q.at(i_t) > ped) {
        subtr_q += q.at(i_t) - peds_[bar];
        n_samp_above_event_ped++;
        ldmx_log(debug) << " above channel event pedestal: " << q.at(i_t)
                        << " > " << ped;
      }  // if above channel event pedestal
    }  // over time samples
    float pe = q_tot * 6250. / gain_[bar];
    subtr_pe = subtr_q * 6250. / gain_[bar];
    hTotQvsPed[bar]->Fill(ped, q_tot);
    hPE[bar]->Fill(pe);
    hPEvsT[bar]->Fill(first_t, pe);
    if (n_samp_above > 0) {
      q_ped_subtracted_avg /= n_samp_above;
      hPedSubtractedAvgQvsT[bar]->Fill(first_t, q_ped_subtracted_avg);
      hAvgQvsT[bar]->Fill(first_t, q_tot / n_samp_above);
    }
    // if (chan.getPedestal() < 40. ) {
    // subtrQ = subtrPE/(6250./4.e6); //undo conversion
    ldmx_log(debug) << "filling qTot histograms";
    hPedSubtractedTotQvsPed[bar]->Fill(ped, subtr_q);
    if (ped < 40)  // avoid case where we have saturation and a plateau as much
                   // as possible
      hPedSubtractedTotQvsN[bar]->Fill(n_samp_above_event_ped, subtr_q);
    hPedSubtractedPEvsN[bar]->Fill(n_samp_above_event_ped, subtr_pe);
    hPedSubtractedPEvsT[bar]->Fill(first_t, subtr_pe);
    ldmx_log(debug) << " done filling qTot histograms";
    // }
  }  // over channels

  return;
}

void QIEAnalyzer::onProcessStart() {
  std::cout << "\n\n Process starts! My analyzer should do something -- like "
               "print this \n\n"
            << std::endl;
  getHistoDirectory();

  int n_time_samp = 68;  // 40
  int p_emax = 100;
  int n_p_ebins = 5 * p_emax;
  float qmax = p_emax / (6250. / 4.e6);
  float qmin = -10;
  int n_qbins = (qmax - qmin) / 4;

  ldmx_log(debug) << "Setting up histograms... ";

  for (int i_b = 0; i_b < nChannels; i_b++) {
    hPE[i_b] = new TH1F(Form("hPE_chan%i", i_b), Form(";PE, chan%i", i_b),
                        n_p_ebins, 0, p_emax);
    hPEvsT[i_b] = new TH2F(
        Form("hPEvsT_chan%i", i_b),
        Form(";First time sample above summing threshold;PE, chan%i", i_b),
        n_time_samp + 1, -1.5, n_time_samp - 0.5, n_p_ebins, 0, p_emax);
    hPedSubtractedAvgQvsT[i_b] = new TH2F(
        Form("hPedSubtrAvgQvsT_chan%i", i_b),
        Form(";First time sample above threshold;Pedestal subtracted average "
             "Q, chan%i [fC]",
             i_b),
        n_time_samp + 1, -1.5, n_time_samp - 0.5, n_qbins / 10, qmin,
        qmax / 10.);
    hPedSubtractedTotQvsPed[i_b] =
        new TH2F(Form("hPedSubtrTotQvsPed_chan%i", i_b),
                 Form(";Channel event pedestal [fC];Event pedestal subtracted "
                      "total Q, chan%i [fC]",
                      i_b),
                 1010, qmin, 1000, 10010, -10,
                 10000);  // nQbins/2,Qmin,Qmax/5., nQbins,Qmin,2*Qmax);
    hPedSubtractedTotQvsN[i_b] =
        new TH2F(Form("hPedSubtrTotQvsN_chan%i", i_b),
                 Form(";Number of time samples added; Event pedestal "
                      "subtracted total Q, chan%i [fC]",
                      i_b),
                 n_time_samp + 1, -1.5, n_time_samp - 0.5, 10010, -10, 10000);
    hTotQvsPed[i_b] = new TH2F(
        Form("hTotQvsPed_chan%i", i_b),
        Form(";Channel event pedestal [fC];Event total Q, chan%i [fC]", i_b),
        1010, qmin, 1000, 10010, -10,
        10000);  // nQbins/2,Qmin,Qmax/5., nQbins,Qmin,2*Qmax);
    hPedSubtractedPEvsN[i_b] = new TH2F(
        Form("hPedSubtrPEvsN_chan%i", i_b),
        Form(";Number of time samples above threshold;Pedestal "
             "subtracted PE, chan%i [fC]",
             i_b),
        n_time_samp + 1, -1.5, n_time_samp - 0.5, n_p_ebins, 0, p_emax);
    hPedSubtractedPEvsT[i_b] = new TH2F(
        Form("hPedSubtrPEvsT_chan%i", i_b),
        Form(";First time sample above threshold;Pedestal subtracted "
             "PE, chan%i [fC]",
             i_b),
        n_time_samp + 1, -1.5, n_time_samp - 0.5, n_p_ebins, 0, p_emax);
    hAvgQvsT[i_b] = new TH2F(
        Form("hAvgQvsT_chan%i", i_b),
        Form(";First time sample above threshold;Average Q, chan%i [fC]", i_b),
        n_time_samp + 1, -1.5, n_time_samp - 0.5, n_qbins / 10, qmin,
        qmax / 10);
  }

  for (int i_e = 0; i_e < nEv; i_e++) {
    for (int i_b = 0; i_b < nChannels; i_b++) {
      hOut[i_e][i_b] =
          new TH1F(Form("hCharge_chan%i_ev%i", i_b, i_e),
                   Form(";time sample; Q, channel %i, event %i [fC]", i_b, i_e),
                   n_time_samp, -0.5, n_time_samp - 0.5);
    }
  }

  hTDCfireChanvsEvent =
      new TH2F("hTDCfireChanvsEvent", ";channel with TDC < 63;event number",
               nChannels, -0.5, nChannels - 0.5, nEv, 0, nEv);

  ldmx_log(debug) << "done setting up histograms";

  return;
}

void QIEAnalyzer::onProcessEnd() { return; }

}  // namespace trigscint

DECLARE_ANALYZER(trigscint::QIEAnalyzer)
