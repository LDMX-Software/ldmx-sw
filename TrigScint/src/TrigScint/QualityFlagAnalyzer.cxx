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
  input_event_col_ = parameters.get<std::string>("input_event_collection");
  input_event_pass_name_ = parameters.get<std::string>("input_event_pass_name");
  input_hit_col_ = parameters.get<std::string>("input_hit_collection");
  input_hit_pass_name_ = parameters.get<std::string>("input_hit_pass_name");
  peds_ = parameters.get<std::vector<double> >("pedestals");
  gain_ = parameters.get<std::vector<double> >("gain");
  start_sample_ = parameters.get<int>("start_sample");

  std::cout << " [ QualityFlagAnalyzer ] In configure(), got parameters "
            << "\n\t inputEventCollection = " << input_event_col_
            << "\n\t inputEventPassName = " << input_event_pass_name_
            << "\n\t inputHitCollection = " << input_hit_col_
            << "\n\t inputHitPassName = " << input_hit_pass_name_
            << "\n\t startSample = " << start_sample_
            << "\n\t pedestals[0] = " << peds_[0]
            << "\n\t gain[0] = " << gain_[0] << "\t." << std::endl;

  return;
}

void QualityFlagAnalyzer::analyze(const framework::Event& event) {
  const auto channels{event.getCollection<trigscint::EventReadout>(
      input_event_col_, input_event_pass_name_)};
  const auto hits{event.getCollection<trigscint::TestBeamHit>(
      input_hit_col_, input_hit_pass_name_)};

  int ev_nb = event.getEventNumber();
  //	while (evNb < 0 ) {
  //  ldmx_log(debug) << "event number = " << evNb << " < 0; incrementing event
  //  number "; evNb++;
  //}
  int n_chan = channels.size();
  ldmx_log(debug) << "in event " << ev_nb << "; n_channels_ = " << n_chan;

  bool exists_intermediate_pe = false;
  float hit_pes[N_CHANNELS] = {0.};

  // ok. get each channel, and find the associated hit, using bar nb.

  for (auto chan : channels) {
    std::vector<float> q = chan.getQ();
    std::vector<float> q_err = chan.getQError();
    std::vector<int> tdc = chan.getTDC();
    // int nTimeSamp = q.size();
    int bar = chan.getChanID();

    int flag = chan.getQualityFlag();

    // check if this is messing up flag
    for (auto hit : hits) {  // we will be ok even if there is no match
      if (hit.getBarID() == bar) {
        flag = hit.getQualityFlag();
        hit_pes[bar] = hit.getPE();
        if (flag == 0 && bar < 12 && 15 < hit.getPE() && hit.getPE() < 40)
          exists_intermediate_pe = true;
      }
    }

    ldmx_log(debug) << "Got event flag " << flag;

    // if flag = 0, fill for clean versions of the usual event displays
    if (flag == 0 && ev_nb < n_ev_ &&
        bar < N_CHANNELS) {  // stick within the predefined histogram array
      for (int i_t = 0; i_t < q.size(); i_t++) {
        ldmx_log(debug) << "in event " << ev_nb << "; channel " << bar
                        << ", got charge[" << i_t << "] = " << q.at(i_t);
        h_out_[ev_nb][bar]->SetBinContent(i_t + start_sample_, q.at(i_t));
        h_out_[ev_nb][bar]->SetBinError(i_t + start_sample_,
                                        fabs(q_err.at(i_t)));
      }  // if within the number of events to plot individually
    }  // over time samples

    // now select on flags_
    // special case: flag = 0 (this will happen to all eventually, so catch it
    // now
    if (flag == 0 && n_ev_drawn_[n_flags_ - 1] < n_ev_) {
      // then draw if we haven't collected enough
      int fill_nb = n_ev_drawn_[n_flags_ - 1];
      for (int i_t = 0; i_t < q.size(); i_t++) {  // fill this plot with all q
        h_out_flag_[n_flags_ - 1][fill_nb][bar]->SetBinContent(
            i_t + start_sample_, q.at(i_t));
        h_out_flag_[n_flags_ - 1][fill_nb][bar]->SetBinError(
            i_t + start_sample_, fabs(q_err.at(i_t)));
      }
      // keep track of actual event number
      h_out_flag_[n_flags_ - 1][fill_nb][bar]->GetYaxis()->SetTitle(
          Form("Q, flag 0, chan %i, ev %i [fC]", bar, ev_nb));
      n_ev_drawn_[n_flags_ -
                  1]++;  // update filled event counter for this flag (0)
    }  // if nothing was flagged
    else {                                            // hit was flagged somehow
      for (int i_f = 0; i_f < n_flags_ - 1; i_f++) {  // do all but the last
        int fill_nb = n_ev_drawn_[i_f];
        ldmx_log(debug) << "Checking flag " << flags_[i_f];
        // we're starting from the high numbers and iteratively subtracting
        if (flag >= flags_[i_f]) {
          ldmx_log(debug) << "Checking flag " << flags_[i_f];
          if (fill_nb < n_ev_) {  // then 1. this flag must be raised 2. draw if
                                  // we haven't collected enough
            for (int i_t = 0; i_t < q.size();
                 i_t++) {  // fill this plot with all
                           // q
              h_out_flag_[i_f][fill_nb][bar]->SetBinContent(i_t + start_sample_,
                                                            q.at(i_t));
              h_out_flag_[i_f][fill_nb][bar]->SetBinError(i_t + start_sample_,
                                                          fabs(q_err.at(i_t)));
            }  // over time samples
            h_out_flag_[i_f][fill_nb][bar]->GetYaxis()->SetTitle(Form(
                "Q, flag %i, chan %i, ev %i [fC]", flags_[i_f], bar, ev_nb));
            n_ev_drawn_[i_f]++;  // update filled event counter for this flag
          }
          flag -= flags_[i_f];  // subtract that flag from sum
        }  // if this flag
      }  // over flags_
    }  // if any non-zero flag
  }  // over channels

  // select 15 < PE < 40 events
  if (exists_intermediate_pe && pe_fill_nb_ < n_ev_) {
    // then 1. this flag must be raised 2. draw if we
    // haven't collected enough
    ldmx_log(debug) << "Got at least one intermediate PE channel";
    for (auto chan : channels) {
      std::vector<float> q = chan.getQ();
      std::vector<float> q_err = chan.getQError();
      std::vector<int> tdc = chan.getTDC();
      // int nTimeSamp = q.size();
      int bar = chan.getChanID();
      for (int i_t = 0; i_t < q.size(); i_t++) {  // fill this plot with all q
        h_out_pe_[pe_fill_nb_][bar]->SetBinContent(i_t + start_sample_,
                                                   q.at(i_t));
        h_out_pe_[pe_fill_nb_][bar]->SetBinError(i_t + start_sample_,
                                                 fabs(q_err.at(i_t)));
      }  // over time samples
      h_out_pe_[pe_fill_nb_][bar]->GetYaxis()->SetTitle(
          Form("Q, chan %i, ev %i, PE %.2f", bar, ev_nb, hit_pes[bar]));
    }  // over channels
    pe_fill_nb_++;  // update filled event counter for this flag
  }  // if fill

  return;
}

void QualityFlagAnalyzer::onProcessStart() {
  ldmx_log(trace)
      << "\n\n Process starts! My analyzer should do something -- like "
         "print this \n\n";
  getHistoDirectory();

  int n_time_samp = 40;
  int p_emax = 100;
  int n_p_ebins = 5 * p_emax;
  // float Qmax = PEmax / (6250. / 4.e6);
  // float Qmin = -10;
  // int nQbins = (Qmax - Qmin) / 4;

  ldmx_log(debug) << "Setting up histograms... ";

  for (int i_b = 0; i_b < N_CHANNELS; i_b++) {
    h_pe_[i_b] = new TH1F(Form("hPE_chan%i", i_b), Form(";PE, chan%i", i_b),
                          n_p_ebins, 0, p_emax);
  }

  for (int i_e = 0; i_e < n_ev_; i_e++) {
    for (int i_b = 0; i_b < N_CHANNELS; i_b++) {
      h_out_[i_e][i_b] =
          new TH1F(Form("hCharge_chan%i_ev%i", i_b, i_e),
                   Form(";time sample; Q, channel %i, event %i [fC]", i_b, i_e),
                   n_time_samp, -0.5, n_time_samp - 0.5);
      h_out_pe_[i_e][i_b] =
          new TH1F(Form("hCharge_PEcut_chan%i_nb%i", i_b, i_e),
                   Form(";time sample; Q, channel %i, event %i [fC]", i_b, i_e),
                   n_time_samp, -0.5, n_time_samp - 0.5);
      for (int i_f = 0; i_f < n_flags_; i_f++) {
        h_out_flag_[i_f][i_e][i_b] = new TH1F(
            Form("hCharge_flag%i_chan%i_nb%i", flags_[i_f], i_b, i_e),
            Form(";time sample; Q, flag %i, chan %i, ev %i [fC]", i_f, i_b,
                 i_e),
            n_time_samp, -0.5,
            n_time_samp - 0.5);  // less confusing to name them upon actual use
        // h_out_flag_[iF][iE][iB] = new TH1F("hCharge_flag",  "",
        // nTimeSamp,-0.5,nTimeSamp-0.5);
      }
    }
  }

  h_td_cfire_chan_vs_event_ = new TH2F(
      "h_td_cfire_chan_vs_event_", ";channel with TDC < 63;event number",
      N_CHANNELS, -0.5, N_CHANNELS - 0.5, n_ev_, 0, n_ev_);

  pe_fill_nb_ = 0;
  ldmx_log(debug) << "done setting up histograms";

  return;
}

void QualityFlagAnalyzer::onProcessEnd() { return; }

}  // namespace trigscint

DECLARE_ANALYZER(trigscint::QualityFlagAnalyzer)
