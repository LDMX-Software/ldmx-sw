
#include "TrigScint/TrigScintFirmwareTracker.h"

#include <iterator>
#include <map>

#include "TrigScint/Firmware/clusterproducer.h"
#include "TrigScint/Firmware/objdef.h"
#include "TrigScint/Firmware/trackproducer.h"

namespace trigscint {

void TrigScintFirmwareTracker::configure(framework::config::Parameters &ps) {
  minThr_ = ps.getParameter<double>("clustering_threshold");
  digis1_collection_ = ps.getParameter<std::string>("digis1_collection");
  digis2_collection_ = ps.getParameter<std::string>("digis2_collection");
  digis3_collection_ = ps.getParameter<std::string>("digis3_collection");
  passName_ = ps.getParameter<std::string>("input_pass_name");
  output_collection_ = ps.getParameter<std::string>("output_collection");
  verbose_ = ps.getParameter<int>("verbosity");
  timeTolerance_ = ps.getParameter<double>("time_tolerance");
  padTime_ = ps.getParameter<double>("pad_time");

  if (verbose_) {
    ldmx_log(info) << "In TrigScintFirmwareTracker: configure done!";
    ldmx_log(info) << "\nClustering threshold: " << minThr_
                   << "\nExpected pad hit time: " << padTime_
                   << "\nMax hit time delay: " << timeTolerance_
                   << "\ndigis1 collection:     " << digis1_collection_
                   << "\ndigis2 collection:     " << digis2_collection_
                   << "\ndigis3 collection:     " << digis3_collection_
                   << "\nInput pass name:     " << passName_
                   << "\nOutput collection:    " << output_collection_
                   << "\nVerbosity: " << verbose_;
  }

  return;
}

void TrigScintFirmwareTracker::produce(framework::Event &event) {
  // This processor takes in TS digis and outputs a track collection. It does so
  // using clusterproducer_sw and trackproducer_hw, which are validated pieces
  // of HLS code (though clusterproducer_sw has had its instances of pragmas
  // excluded. I will comment on how clusterproducer and trackproducer work more
  // thouroughly in them respectively, but generally the clusterproducer makes
  // only two hit clusters (as ready that was all that was made from the
  // original sw) and does so by making a digi map and running along channels
  // numerically and pairing if possible. The trackproducer takes a LOOKUP array
  // as a LUT and does track pattern mathcing. This depends on alignment through
  // the A vector below.

  if (verbose_) {
    ldmx_log(debug)
        << "TrigScintFirmwareTracker: produce() starts! Event number: "
        << event.getEventHeader().getEventNumber();
  }

  // A is the mis-alignment vector
  ap_int<12> a[3] = {0, 0, 0};
  ap_int<12> lookup[NCENT][COMBO][2];

  // Initialize the LOOKUP table to zero
  for (int i = 0; i < NCENT; ++i) {
    for (int j = 0; j < COMBO; ++j) {
      for (int k = 0; k < 2; ++k) {
        lookup[i][j][k] = ap_int<12>(-1);
      }
    }
  }

  // This line fills in the LOOKUP table used for patter matching latter. The
  // array takes in as its first argument the centroid of a first pad cluster,
  // then the next two take on which track pattern (of ~9) we are matching to
  // and the last if we are matching to a cluster with two hits
  for (int i = 0; i < NCENT; i++) {
    for (int j = 0; j < COMBO; j++) {
      lookup[i][j][0] = (i - a[1] + a[0]);
      lookup[i][j][1] = (i - a[2] + a[0]);
      if (j / 3 == 0) {
        lookup[i][j][0] -= 1;
      } else if (j / 3 == 2) {
        lookup[i][j][0] += 1;
      }
      if (j % 3 == 0) {
        lookup[i][j][1] -= 1;
      } else if (j % 3 == 2) {
        lookup[i][j][1] += 1;
      }
      if (not((lookup[i][j][0] >= 0) and (lookup[i][j][1] >= 0) and
              (lookup[i][j][0] < NCENT) and (lookup[i][j][1] < NCENT))) {
        lookup[i][j][0] = -1;
        lookup[i][j][1] = -1;
      }
    }
  }
  // Here we instantiate arrays necessary to do the rest of it.
  Hit h_pad1[NHITS];
  Hit h_pad2[NHITS];
  Hit h_pad3[NHITS];

  // Pad1 goes with NTRK bc of firmware bandwidth constraints
  // It is also expected on Pad1 to have 1 cluster per track
  Cluster pad1[NTRK];
  Cluster pad2[NCLUS];
  Cluster pad3[NCLUS];
  Track out_trk[NTRK];

  for (int j = 0; j < NHITS; j++) {
    clearHit(h_pad1[j]);
    clearHit(h_pad2[j]);
    clearHit(h_pad3[j]);
  }
  for (int j = 0; j < NCLUS; j++) {
    if (j < NTRK) {
      clearClus(pad1[j]);
    }
    clearClus(pad2[j]);
    clearClus(pad3[j]);
  }
  for (int j = 0; j < NTRK; j++) {
    clearTrack(out_trk[j]);
  }
  // I am reading in the three digi collections
  const auto &digis1{
      event.getCollection<ldmx::TrigScintHit>(digis1_collection_, passName_)};
  const auto &digis2{
      event.getCollection<ldmx::TrigScintHit>(digis2_collection_, passName_)};
  const auto &digis3{
      event.getCollection<ldmx::TrigScintHit>(digis3_collection_, passName_)};

  if (verbose_) {
    ldmx_log(debug) << "Got digi collection " << digis1_collection_ << "_"
                    << passName_ << " with " << digis1.size() << " entries ";
  }

  // The next collection of things fill in the firmware hit objects from reading
  // in the digi collections the necessary information. The firmware hit objects
  // only keep bID,mID,Time, and PE count.
  int occupied[NCHAN];
  for (int i = 0; i < NCHAN; i++) {
    occupied[i] = -1;
  }
  int count = 0;
  for (const auto &digi : digis1) {
    if ((digi.getPE() > minThr_) and (digi.getBarID() <= NCHAN) and
        (digi.getBarID() >= 0)) {
      ap_int<12> b_id = (ap_int<12>)(digi.getBarID());
      ap_int<12> amp = (ap_int<12>)(digi.getPE());
      if (occupied[digi.getBarID()] >= 0) {
        if (h_pad1[occupied[digi.getBarID()]].Amp < digi.getPE()) {
          h_pad1[occupied[digi.getBarID()]].bID = (ap_int<12>)(digi.getBarID());
          h_pad1[occupied[digi.getBarID()]].mID =
              (ap_int<12>)(digi.getModuleID());
          h_pad1[occupied[digi.getBarID()]].Amp = (ap_int<12>)(digi.getPE());
          h_pad1[occupied[digi.getBarID()]].Time = (ap_int<12>)(digi.getTime());
        }
      } else {
        h_pad1[count].bID = (ap_int<12>)(digi.getBarID());
        h_pad1[count].mID = (ap_int<12>)(digi.getModuleID());
        h_pad1[count].Amp = (ap_int<12>)(digi.getPE());
        h_pad1[count].Time = (ap_int<12>)(digi.getTime());
        occupied[digi.getBarID()] = count;
        count++;
      }
    }
  }

  for (int i = 0; i < NCHAN; i++) {
    occupied[i] = -1;
  }
  count = 0;
  for (const auto &digi : digis2) {
    if ((digi.getPE() > minThr_) and (digi.getBarID() <= NCHAN) and
        (digi.getBarID() >= 0)) {
      ap_int<12> b_id = (ap_int<12>)(digi.getBarID());
      ap_int<12> amp = (ap_int<12>)(digi.getPE());
      if (occupied[digi.getBarID()] >= 0) {
        if (h_pad2[occupied[digi.getBarID()]].Amp < digi.getPE()) {
          h_pad2[occupied[digi.getBarID()]].bID = (ap_int<12>)(digi.getBarID());
          h_pad2[occupied[digi.getBarID()]].mID =
              (ap_int<12>)(digi.getModuleID());
          h_pad2[occupied[digi.getBarID()]].Amp = (ap_int<12>)(digi.getPE());
          h_pad2[occupied[digi.getBarID()]].Time = (ap_int<12>)(digi.getTime());
        }
      } else {
        h_pad2[count].bID = (ap_int<12>)(digi.getBarID());
        h_pad2[count].mID = (ap_int<12>)(digi.getModuleID());
        h_pad2[count].Amp = (ap_int<12>)(digi.getPE());
        h_pad2[count].Time = (ap_int<12>)(digi.getTime());
        occupied[digi.getBarID()] = count;
        count++;
      }
    }
  }
  for (int i = 0; i < NCHAN; i++) {
    occupied[i] = -1;
  }
  count = 0;
  for (const auto &digi : digis3) {
    if ((digi.getPE() > minThr_) and (digi.getBarID() <= NCHAN) and
        (digi.getBarID() >= 0)) {
      ap_int<12> b_id = (ap_int<12>)(digi.getBarID());
      ap_int<12> amp = (ap_int<12>)(digi.getPE());
      if (occupied[digi.getBarID()] >= 0) {
        if (h_pad3[occupied[digi.getBarID()]].Amp < digi.getPE()) {
          h_pad3[occupied[digi.getBarID()]].bID = (ap_int<12>)(digi.getBarID());
          h_pad3[occupied[digi.getBarID()]].mID =
              (ap_int<12>)(digi.getModuleID());
          h_pad3[occupied[digi.getBarID()]].Amp = (ap_int<12>)(digi.getPE());
          h_pad3[occupied[digi.getBarID()]].Time = (ap_int<12>)(digi.getTime());
        }
      } else {
        h_pad3[count].bID = (ap_int<12>)(digi.getBarID());
        h_pad3[count].mID = (ap_int<12>)(digi.getModuleID());
        h_pad3[count].Amp = (ap_int<12>)(digi.getPE());
        h_pad3[count].Time = (ap_int<12>)(digi.getTime());
        occupied[digi.getBarID()] = count;
        count++;
      }
    }
  }
  // These next lines here calls clusterproducer_sw(HPad1), which is just the
  // validated firmware module. Since ap_* class is messy, I had to do some
  // post-call cleanup before looping over the clusters and putting them into
  // Point i which is feed into track producer
  int counter_n = 0;
  std::array<Cluster, NCLUS> point1 = clusterproducer_sw(h_pad1);
  int top_seed = 0;
  for (int i = 0; i < NCLUS; i++) {
    if ((point1[i].Seed.Amp < 450) and (point1[i].Seed.Amp > 30) and
        (point1[i].Seed.bID < (NCHAN + 1)) and (point1[i].Seed.bID >= 0) and
        (point1[i].Sec.Amp < 450) and (counter_n < NTRK)) {
      if (point1[i].Seed.bID >= top_seed) {
        cpyHit(pad1[counter_n].Seed, point1[i].Seed);
        cpyHit(pad1[counter_n].Sec, point1[i].Sec);
        calcCent(pad1[counter_n]);
        counter_n++;
        top_seed = point1[i].Seed.bID;
      }
    }
  }
  std::array<Cluster, NCLUS> point2 = clusterproducer_sw(h_pad2);
  top_seed = 0;
  for (int i = 0; i < NCLUS; i++) {
    if ((point2[i].Seed.Amp < 450) and (point2[i].Seed.Amp > 30) and
        (point2[i].Seed.bID < (NCHAN + 1)) and (point2[i].Seed.bID >= 0) and
        (point2[i].Sec.Amp < 450)) {
      if (point2[i].Seed.bID >= top_seed) {
        cpyHit(pad2[i].Seed, point2[i].Seed);
        cpyHit(pad2[i].Sec, point2[i].Sec);
        calcCent(pad2[i]);
        top_seed = point2[i].Seed.bID;
      }
    }
  }
  std::array<Cluster, NCLUS> point3 = clusterproducer_sw(h_pad3);
  top_seed = 0;
  for (int i = 0; i < NCLUS; i++) {
    if ((point3[i].Seed.Amp < 450) and (point3[i].Seed.Amp > 30) and
        (point3[i].Seed.bID < (NCHAN + 1)) and (point3[i].Seed.bID >= 0) and
        (point3[i].Sec.Amp < 450)) {
      if (point3[i].Seed.bID >= top_seed) {
        cpyHit(pad3[i].Seed, point3[i].Seed);
        cpyHit(pad3[i].Sec, point3[i].Sec);
        calcCent(pad3[i]);
        top_seed = point3[i].Seed.bID;
      }
    }
  }
  // I have stagged the digis into firmware digi objects and paired them into
  // firmware cluster objects, so at this point I can insert them and the LUT
  // into the trackproducer_hw to create the track collection I use makeTrack to
  // revert the firmware track object back into a regular track object for
  // analysis purposes
  //
  // NOTE: Pad1 has NTRK instead of NCLUS clusters for a reason: the firmware
  // cannot facilitate NCLUS many tracks within its alloted bandwidth , we have
  // to put a cut on them which is facilitated by a cut on the number of
  // clusters in Pad1. Do not change this.
  trackproducer_hw(pad1, pad2, pad3, out_trk, lookup);
  for (int i = 0; i < NTRK; i++) {
    if (out_trk[i].Pad1.Seed.Amp > 0. && out_trk[i].Pad1.Sec.Amp >= 0. &&
        out_trk[i].Pad2.Seed.Amp > 0. && out_trk[i].Pad2.Sec.Amp >= 0. &&
        out_trk[i].Pad3.Seed.Amp > 0. && out_trk[i].Pad3.Sec.Amp >= 0.) {
      ldmx::TrigScintTrack trk = makeTrack(out_trk[i]);
      tracks_.push_back(trk);
    }
  }
  event.add(output_collection_, tracks_);
  tracks_.resize(0);

  return;
}

ldmx::TrigScintTrack TrigScintFirmwareTracker::makeTrack(Track outTrk) {
  // This takes a firmware track object and reverts it into an ldmx track
  // object, unfortunately only retaining that information of the track that is
  // retained in the firmware track.
  ldmx::TrigScintTrack tr;
  float pe{0.};
  pe += static_cast<float>(outTrk.Pad1.Seed.Amp) +
        static_cast<float>(outTrk.Pad1.Sec.Amp);
  pe += static_cast<float>(outTrk.Pad2.Seed.Amp) +
        static_cast<float>(outTrk.Pad2.Sec.Amp);
  pe += static_cast<float>(outTrk.Pad3.Seed.Amp) +
        static_cast<float>(outTrk.Pad3.Sec.Amp);
  tr.setCentroid(calcTCent(outTrk));
  calcResid(outTrk);
  tr.setPE(pe);
  return tr;
}

}  // namespace trigscint

DECLARE_PRODUCER(trigscint::TrigScintFirmwareTracker);
