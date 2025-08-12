/**
 * @file TestBeamHitProducer.cxx
 * @brief An producer drawing the most basic quanities of Trigger Scintillator
 * bars
 * @author Lene Kristian Bryngemark, Stanford University
 */

#include "TrigScint/TestBeamHitProducer.h"

namespace trigscint {

TestBeamHitProducer::TestBeamHitProducer(const std::string& name,
                                         framework::Process& process)
    : Producer(name, process) {}

void TestBeamHitProducer::configure(framework::config::Parameters& parameters) {
  inputCol_ = parameters.getParameter<std::string>("inputCollection");
  outputCollection_ = parameters.getParameter<std::string>("outputCollection");
  inputPassName_ = parameters.getParameter<std::string>("inputPassName");
  MIPresponse_ = parameters.getParameter<std::vector<double> >("MIPresponse");
  peds_ = parameters.getParameter<std::vector<double> >("pedestals");
  gain_ =
      parameters.getParameter<std::vector<double> >("gain");  // to do: vector
  startSample_ = parameters.getParameter<int>("startSample");
  pulseWidth_ = parameters.getParameter<int>("pulseWidth");
  pulseWidthLYSO_ = parameters.getParameter<int>("pulseWidthLYSO");
  nInstrumentedChannels_ =
      parameters.getParameter<int>("nInstrumentedChannels");
  doCleanHits_ = parameters.getParameter<bool>("doCleanHits");

  std::cout << " [ TestBeamHitProducer ] In configure(), got parameters "
            << "\n\t inputCollection = " << inputCol_
            << "\n\t inputPassName = " << inputPassName_
            << "\n\t outputCollection = " << outputCollection_
            << "\n\t startSample = " << startSample_
            << "\n\t pulseWidth = " << pulseWidth_
            << "\n\t pulseWidthLYSO = " << pulseWidthLYSO_
            << "\n\t gain[0] = " << gain_[0]
            << "\n\t nInstrumentedChannels = " << nInstrumentedChannels_
            << "\n\t doCleanHits = " << doCleanHits_
            << "\n\t pedestals[0] = " << peds_[0]
            << "\n\t MIPresponse[0] = " << MIPresponse_[0] << "\t."
            << std::endl;

  return;
}

void TestBeamHitProducer::produce(framework::Event& event) {
  /*
    hit producer.
    sum up all charge within a certain time window. could work two ways:
        - find a time sample above a threshold or where TDC is 0-50 (0-2)
                 * this could allow for finding several pulses per event, just
    keep sliding along in time
                     -- in that case, record nPulses, startsample, pulsewidth
    for each hit
            - use a fixed start sample, and keep summing from there until
    nSamples is reached or all time samples used

    specifics:
            - each channel has its own pedestal to subtract. also try using the
    first 5 samples to establish a threshold in the event. store both
            - store hit Q and hit PE conversion
            - for now use the same reconstruction paradigm for LYSO and plastic.
    two notes though
              1. LYSO pulse looks wider, and might need wider window. plastic is
    fine with 5. start by 8 for LYSO (even if for large pulses, sometimes 10
    seem to be needed). could probe this by correlating plastic and LYSO, and
    checking if at some amplitude (in plastic), we start cutting off charge in
    LYSO
                  2. there is a time offset between fibers. need to have a
    parameter fiber2offset to use for elecID >= 8
                  -- added this as a variable in EventReadout, along with fiber
    number. so this class doesn't need any detailed knowledge

    variables to write out:
        - start sample
            - pulse width
            - n samples above threshold
            - nPulses
            - early pedestal (from first 5)
            - assumed/average channel pedestal
            - total Q
            - ped subtracted total Q
            - PE value
            - max amplitude in pulse
            - store material assumption? isLYSO?
   */

  float mev_per_mip = 0.3;
  float pe_per_mip = 100;

  const auto channels{
      event.getCollection<trigscint::EventReadout>(inputCol_, inputPassName_)};

  int ev_nb = event.getEventNumber();
  std::vector<trigscint::TestBeamHit> hits;
  for (auto chan : channels) {
    trigscint::TestBeamHit hit;
    int bar = chan.getChanID();
    if (bar >=
        nInstrumentedChannels_)  // don't run hit reconstruction on junk signal
      continue;
    int width = pulseWidth_;
    if (bar % 2 == 0) {         // LYSO channel: allow for wider pulses
      width = pulseWidthLYSO_;  // avoid hardwiring
    }
    hit.setPulseWidth(width);
    hit.setStartSample(startSample_);
    float ped = peds_.at(bar);  // chan.getPedestal() ;
    float early_ped = chan.getEarlyPedestal();
    hit.setPedestal(ped);
    hit.setEarlyPedestal(early_ped);
    int is_clean = 0;             // false;
    float threshold = fabs(ped);  // 2*fabs(peds_[ bar ]); // or sth
    if (doCleanHits_) threshold = 7 * fabs(ped);  // stricter cut

    int start_t = startSample_ + chan.getTimeOffset();
    float max_q = -999.;
    int n_samp_above_ped = 0;
    int n_samp_above_thr = 0;
    float tot_subtr_q = 0;
    std::vector<float> q = chan.getQ();
    // go to the start sample defined for this channel.
    for (int i_t = start_t; i_t < q.size(); i_t++) {
      ldmx_log(debug) << "in event " << ev_nb << "; channel " << bar
                      << ", got charge[" << i_t << "] = " << q.at(i_t);
      // for the defined number of samples, subtract pedestal. if > 0, increment
      // sample counter.
      float sub_q = q.at(i_t) -
                    ped;  // this might be addition, if ped is negative. should
                          // see this as channel dependence in nSampAbove
      // once beyond nSamples, want to see how long positive threshold
      // subtracted tail is --> increment sample counter in any case.
      if (sub_q > 0) n_samp_above_ped++;
      if (sub_q > threshold) n_samp_above_thr++;
      if (i_t - start_t < width) {  // we're in the pulse integration window
        if (sub_q > max_q)  // keep track of max Q. this is the pulse amplitude
          max_q = sub_q;    // q.at(iT);
        if (sub_q > 0)
          tot_subtr_q +=
              sub_q;  // add positive subtracted Q to total pulse charge.
      } else if (sub_q < 0 ||
                 q.at(i_t) < 0)  // if after the full pulse width, subQ <
                                 // pedestal, break. special case to break at
                                 // q=0 for cases where ped < 0
        break;
      // done
    }  // over time samples

    // first check that hit passes any quality cuts
    uint flag = chan.getQualityFlag();
    if (doCleanHits_) {
      //		int isLongPulse=(nSampAboveThr < 2 || nSampAboveThr >
      // width + 2);
      int is_long_pulse =
          (n_samp_above_thr >
           width);  // the short ones we'll have to single out otherwise (like
                    // spike flag or low Q) or live with
      flag += 4 * is_long_pulse;
      if (max_q > 2e5 && n_samp_above_thr < 3 &&
          flag % 2 == 0)  // this was not flagged as a spike but is eerily
                          // narrow and with high Q; flag it as a spike.
        flag += 1;
      if (flag == 0) is_clean = 1;
    }

    float pe = tot_subtr_q * 6250. / gain_[bar];
    if (pe > 20)  // dont't want to intercalibrate the shot noise
      pe *= MIPresponse_[bar];

    // set pulse properties like PE and amplitude
    hit.setSampAbovePed(n_samp_above_ped);
    hit.setSampAboveThr(n_samp_above_thr);
    hit.setQ(tot_subtr_q);
    hit.setAmplitude(max_q);
    hit.setPE(pe);
    hit.setHitQuality(is_clean);
    hit.setQualityFlag(flag);
    // set bar id. set moduleNB = 0
    hit.setBarID(bar);
    hit.setModuleID(0);  // test beam
    // the rest are a little ill-defined for now (PE-energy conversion not
    // known/different between LYSO and plastic)
    hit.setTime(-999);  // maybe?
    hit.setBeamEfrac(-1.);
    hit.setEnergy(hit.getPE() * mev_per_mip / pe_per_mip);

    // add hit
    hits.push_back(hit);
  }  // over channels

  // at end of event, write the collection of trigger scintillator hits.
  event.add(outputCollection_, hits);

  return;
}

}  // namespace trigscint

DECLARE_PRODUCER(trigscint::TestBeamHitProducer)
