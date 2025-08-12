#include "TrigScint/EventReadoutProducer.h"

#include <iostream>

#include "Framework/Exception/Exception.h"

namespace trigscint {

EventReadoutProducer::EventReadoutProducer(const std::string &name,
                                           framework::Process &process)
    : Producer(name, process) {}

void EventReadoutProducer::configure(
    framework::config::Parameters &parameters) {
  // Configure this instance of the producer
  inputCollection_ = parameters.getParameter<std::string>("input_collection");
  inputPassName_ = parameters.getParameter<std::string>("input_pass_name");
  outputCollection_ = parameters.getParameter<std::string>("output_collection");
  nPedSamples_ = parameters.getParameter<int>("number_pedestal_samples");
  timeShift_ = parameters.getParameter<int>("time_shift");
  fiberToShift_ = parameters.getParameter<int>("fiber_to_shift");
  verbose_ = parameters.getParameter<bool>("verbose");

  ldmx_log(debug) << "In configure, got parameters:" << "\noutput_collection = "
                  << outputCollection_
                  << "\ninput_collection = " << inputCollection_
                  << "\ninput_pass_name  = " << inputPassName_
                  << "\nnumber_pedestal_samples  = " << nPedSamples_
                  << "\ntime_shift = " << timeShift_
                  << "\nfiber_to_shift  = " << fiberToShift_
                  << "\nverbose          = " << verbose_;
}

void EventReadoutProducer::produce(framework::Event &event) {
  // initialize QIE object for linearizing ADCs
  SimQIE qie;

  const auto digis{event.getCollection<trigscint::TrigScintQIEDigis>(
      inputCollection_, inputPassName_)};

  std::vector<trigscint::EventReadout> channel_readout_events;
  for (const auto &digi : digis) {
    trigscint::EventReadout out_event;
    auto adc{digi.getADC()};
    auto tdc{digi.getTDC()};

    // copy over from qie digi for convenience
    out_event.setChanID(digi.getChanID());
    out_event.setElecID(digi.getElecID());
    out_event.setTimeSinceSpill(digi.getTimeSinceSpill());
    // elecID increases monotonically with 8 channels per fiber
    out_event.setFiberNb(digi.getElecID() / 8);
    if (out_event.getFiberNb() == fiberToShift_)
      out_event.setTimeOffset(timeShift_);

    out_event.setADC(adc);
    out_event.setTDC(tdc);
    std::vector<float> charge;
    std::vector<float> charge_err;

    float avg_q = 0;
    float tot_pos_q = 0;
    int i_s = 0;
    [[maybe_unused]] int n_pos = 0;
    float early_ped = 0;
    for (auto &val : adc) {
      float q = qie.ADC2Q(val);
      charge.push_back(q);
      charge_err.push_back(qie.QErr(q));
      avg_q += q;  // charge.back();
      if (q > 0) {
        tot_pos_q += q;
        n_pos++;
      }
      if (verbose_)
        ldmx_log(debug) << "got adc value " << val << " and charge "
                        << q;  // qie.ADC2Q(val);
      if (i_s < nPedSamples_) early_ped += q;
      i_s++;
    }
    out_event.setQ(charge);          // set in proper order before sorting
    out_event.setQError(charge_err);  // set in proper order before sorting
    early_ped /= nPedSamples_;
    out_event.setEarlyPedestal(early_ped);

    // oscillation check. for this the pulse charge needs to be in order, so set
    // this up now the period is 4. ansatz: an oscillation is a repeated shape.
    // normalize to maxq=1, in every interval of 4 samples if after
    // normalization the same numbers are repeated, it's an oscillation edge
    // case: multiple single PE peaks with that repetition. we probably don't
    // need to keep those anyway
    std::vector<float> charge_check = {NULL};
    float min_charge =
        10;  // no point in looking at oscillations just around the pedestal.
             // nearest edge is 10.35 fC  //ADC=0 corresponds to -16 fC.
    float ped = 0;
    int ped_length = (int)charge.size() /
                    5;  // actually pulse can be up to 12 samples = 2/5*30
    int ped_offset = 2;  // but still skip the lowest (and highest) few
                        //	for (int i = pedLength; i < 3*pedLength ; i++) {
                        ////use 2nd and third 5th
    if (charge.size() > 8) {
      if (verbose_) ldmx_log(debug) << "going into oscillations check ";
      for (int i = 3; i < charge.size() - 4; i++) {
        float max_samp = min_charge;
        for (int i_q = 0; i_q < 4; i_q++) {  // find the local max in the 4 samples
          //		ldmx_log(debug) << "got charge " << charge[i+iQ];
          if (charge[i + i_q] > max_samp) max_samp = charge[i + i_q];
        }
        if (verbose_) ldmx_log(debug) << "got max charge " << max_samp;
        for (int i_q = 0; i_q < 4; i_q++)  // store the locally normalized numbers.
                                        // even if the period is
          // 5 this should work for a while
          charge_check.push_back(charge[i + i_q] / max_samp);
        i += 3;  // to increment by 4, do 3 here and 1 in the loop
      }
    }
    if (ped_length > 4) {
      // now calculate the pedestal as the average of the middle half of the
      // sorted vector
      std::sort(charge.begin(), charge.end());
      for (int i = ped_offset; i < 2 * ped_length + ped_offset;
           i++) {  // use 1st and 2nd 5th
        ped += charge[i];
      }
      ped /= 2 * ped_length;
    }
    // median: technically only true for odd number of elements but good enough
    float med_q = charge[(int)charge.size() / 2];
    float min_q = charge[0];
    float max_q = charge[charge.size() - 1];

    out_event.setTotQ(tot_pos_q);  //-nPos*ped); //store (event) ped subtracted
                                // total charge, before dividing by N  -->
                                // actually, ped subtraction makes it confusing
    //	outEvent.setTotQ(totPosQ-adc.size()*ped); //store (event) ped subtracted
    // total charge, before dividing by N
    // outEvent.setTotQ(avgQ-adc.size()*ped);
    ////store (event) ped subtracted total charge, before dividing by N
    avg_q /= adc.size();
    out_event.setPedestal(ped);
    out_event.setAvgQ(avg_q);
    out_event.setMedQ(med_q);
    out_event.setMinQ(min_q);
    out_event.setMaxQ(max_q);

    // and the noise as the RMSE of that, same interval as pedestal
    float diff_sq = 0;
    //    for (int i = pedLength; i < 3*pedLength ; i++) {
    if (charge.size() > 8) {
      for (int i = ped_offset; i < 2 * ped_length + ped_offset; i++) {
        diff_sq += (charge[i] - ped) * (charge[i] - ped);
      }
      diff_sq /= 2 * ped_length;  // adc.size();
    }
    out_event.setNoise(sqrt(diff_sq));

    // oscillation check
    uint flag_oscillation = 0;
    if (charge.size() > 8) {
      // no need to run tedious oscillation check for all-neg channels
      if (max_q > min_charge) {
        int max_id = 0;
        // find the first occurence of a local max
        for (int i = 0; i < charge_check.size() - 4; i++) {
          if (charge_check[i] ==
              1) {  // an actual local max has been normalised by its own value
            max_id = i;
            //		  ldmx_log(debug) << "storing max index " <<maxID <<"
            // and size of vector is " << chargeCheck.size()-4;
            break;
          }
        }
        int last_match_sample = 0;
        // start from local max
        bool do_break = false;
        for (int i = max_id; i < charge_check.size() - 4; i++) {
          if (verbose_)
            ldmx_log(debug)
                << "Checking how many matching groups of four we can "
                   "find, starting at index "
                << i;

          for (int i_q = 0; i_q < 4; i_q++) {  // check if they are consistently
            // close
            if (verbose_)
              ldmx_log(debug)
                  << "Comparing " << charge_check[i + i_q] << " (sample "
                  << i + i_q << ") to " << charge_check[i + 4 + i_q] << " (sample "
                  << i + 4 + i_q << "), ratio is "
                  << charge_check[i + i_q] / charge_check[i + 4 + i_q];
            // we can be generous in these crietira since we will require an
            // unbroken suite of 8 matches to call it oscillation
            if (fabs(charge_check[i + i_q] / charge_check[i + 4 + i_q] - 1) <
                    0.5 ||  // need this tolerance to be kind of large, most
                // actual peaks won't pass it by far anyway.
                (charge_check[i + 4 + i_q] < 0.01 &&
                 fabs(charge_check[i + i_q] / charge_check[i + 4 + i_q]) <
                     5))  // for very small numbers, one ADC difference can be a
              // factor 3 so add some margin
              last_match_sample = i + i_q;
            else {
              if (verbose_)
                ldmx_log(debug)
                    << "Oscillation check for channel " << digi.getChanID()
                    << " breaking at time sample " << i + i_q;
              do_break = true;  // break outer loop
              break;           // break this loop
            }
          }
          if (do_break) {
            break;
          }
          if (verbose_)
            ldmx_log(debug) << "Current lastMatchSample " << last_match_sample;
          if (last_match_sample - max_id >=
              2 * 4) {  // we had at least a couple of oscillations (2nd period
            // was fully matched by third)
            flag_oscillation = 1;  // there is another check possible later too,
            // commented for now
            break;  // we've seen what we need to see
          }
          i += 3;
        }
      }  // if positive maxQ
    }
    // //use the top and bottom ends of the sorted q as another oscillation
    // catcher: we don't expect that the top values will be high and basically
    // identical unless they are from an oscillation
    int n_high = 0;
    int quart_length = (int)charge.size() / 4;
    for (int i = 4 * quart_length - 2; i >= 3 * quart_length;
         i--) {  // maxQ is already at last index
      if (charge[i] / max_q > 0.66) n_high++;
    }

    /* used to be: >0.9, > 0.25. But this really killed MC pulses which
       all end up in 0.90-0.94, and somewhere >0.05 (at least >0.15)
    */
    uint flag_spike = (max_q / out_event.getTotQ() > 0.95) ||
                     (charge[charge.size() - 2] / max_q <
                      0.05);  // skip "unnaturally" narrow hits (all charge in
                              // one sample or huge drop to second highest)
    uint flag_plateau =
        (ped > 15 || n_high >= 5);  //( fabs(ped) > 15 ); //threshold //   //skip
                                   // events that have strange plateaus
    uint flag_long_pulse =
        0;  // easier to deal with in hit reconstruction directly. copy channel
            // flags to hit flags and add this one there
    uint flag_noise =
        (out_event.getNoise() > 3.5 ||
         out_event.getNoise() == 0);  // =0 is typically from funky events but
                                     // could be too harsh maybe
                                     /* //let this wait for now
                                     //if we have many high counts, a small event pedestal (where they weren't
                                     included), and an avgQ ~ maxQ/nHigh, then this is an oscillation                                  if (
                                     (quartLength-nHigh<2 && ped<10) || fabs( maxQ/(avgQ*nHigh)-1 ) <0.1 )
                                       flagOscillation=1;
                             */

    /* //this seems to not catch what we want it to
if ( fabs(chan.getPedestal()) < 15 //threshold //   //skip events that have
strange plateaus
           //              && (chan.getAvgQ()/chan.getPedestal()<0.8)  //skip
events that have strange oscillations
           && 1 < nSampAboveThr && nSampAboveThr < 10 ) // skip one-time sample
flips and long weird pulses
    */
    uint flag = flag_spike + 2 * flag_plateau + 4 * flag_long_pulse +
                8 * flag_oscillation + 16 * flag_noise;
    ldmx_log(debug)
        << "Got quality flag " << flag
        << " made up of (spike/plateau/long pulse/oscillation/noise) "
        << flag_spike << "+" << flag_plateau << "+" << flag_long_pulse << "+"
        << flag_oscillation << "+" << flag_noise;
    out_event.setQualityFlag(flag);

    //	if (ped > 15 )
    //  continue;
    ldmx_log(debug) << "In event " << event.getEventHeader().getEventNumber()
                    << ", set pedestal = " << out_event.getPedestal()
                    <<  // ped <<
        " fC, noise = " << out_event.getNoise() << " fC for channel "
                    << out_event.getChanID();

    channel_readout_events.push_back(out_event);
  }
  // Create the container to hold the
  // digitized trigger scintillator hits.

  event.add(outputCollection_, channel_readout_events);
  ldmx_log(debug) << "\n";
}
}  // namespace trigscint

DECLARE_PRODUCER(trigscint::EventReadoutProducer);
