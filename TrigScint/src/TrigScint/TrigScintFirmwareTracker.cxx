
#include "TrigScint/TrigScintFirmwareTracker.h"

#include <iterator>
#include <map>

#include "TrigScint/Firmware/clusterproducer.h"
#include "TrigScint/Firmware/objdef.h"
#include "TrigScint/Firmware/trackproducer.h"

namespace trigscint {

void TrigScintFirmwareTracker::configure(framework::config::Parameters &ps) {
  min_thr_ = ps.get<double>("clustering_threshold");
  digis1_collection_ = ps.get<std::string>("digis1_collection");
  digis2_collection_ = ps.get<std::string>("digis2_collection");
  digis3_collection_ = ps.get<std::string>("digis3_collection");
  pass_name_ = ps.get<std::string>("input_pass_name");
  output_collection_ = ps.get<std::string>("output_collection");
  verbose_ = ps.get<int>("verbosity");
  time_tolerance_ = ps.get<double>("time_tolerance");
  pad_time_ = ps.get<double>("pad_time");

  if (verbose_) {
    ldmx_log(info) << "In TrigScintFirmwareTracker: configure done!";
    ldmx_log(info) << "\nClustering threshold: " << min_thr_
                   << "\nExpected pad hit time: " << pad_time_
                   << "\nMax hit time delay: " << time_tolerance_
                   << "\ndigis1 collection:     " << digis1_collection_
                   << "\ndigis2 collection:     " << digis2_collection_
                   << "\ndigis3 collection:     " << digis3_collection_
                   << "\nInput pass name:     " << pass_name_
                   << "\nOutput collection:    " << output_collection_
                   << "\nVerbosity: " << verbose_;
  }

  return;
}

void TrigScintFirmwareTracker::produce(framework::Event &event) {
  // This processor takes in TS digis and outputs a track collection. It does so
  // using clusterproducer_sw and trackproducerHw, which are validated pieces
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
      event.getCollection<ldmx::TrigScintHit>(digis1_collection_, pass_name_)};
  const auto &digis2{
      event.getCollection<ldmx::TrigScintHit>(digis2_collection_, pass_name_)};
  const auto &digis3{
      event.getCollection<ldmx::TrigScintHit>(digis3_collection_, pass_name_)};

  if (verbose_) {
    ldmx_log(debug) << "Got digi collection " << digis1_collection_ << "_"
                    << pass_name_ << " with " << digis1.size() << " entries ";
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
    if ((digi.getPE() > min_thr_) and (digi.getBarID() <= NCHAN) and
        (digi.getBarID() >= 0)) {
      ap_int<12> b_id = (ap_int<12>)(digi.getBarID());
      ap_int<12> amp = (ap_int<12>)(digi.getPE());
      if (occupied[digi.getBarID()] >= 0) {
        if (h_pad1[occupied[digi.getBarID()]].amp_ < digi.getPE()) {
          h_pad1[occupied[digi.getBarID()]].b_id_ =
              (ap_int<12>)(digi.getBarID());
          h_pad1[occupied[digi.getBarID()]].m_id_ =
              (ap_int<12>)(digi.getModuleID());
          h_pad1[occupied[digi.getBarID()]].amp_ = (ap_int<12>)(digi.getPE());
          h_pad1[occupied[digi.getBarID()]].time_ =
              (ap_int<12>)(digi.getTime());
        }
      } else {
        h_pad1[count].b_id_ = (ap_int<12>)(digi.getBarID());
        h_pad1[count].m_id_ = (ap_int<12>)(digi.getModuleID());
        h_pad1[count].amp_ = (ap_int<12>)(digi.getPE());
        h_pad1[count].time_ = (ap_int<12>)(digi.getTime());
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
    if ((digi.getPE() > min_thr_) and (digi.getBarID() <= NCHAN) and
        (digi.getBarID() >= 0)) {
      ap_int<12> b_id = (ap_int<12>)(digi.getBarID());
      ap_int<12> amp = (ap_int<12>)(digi.getPE());
      if (occupied[digi.getBarID()] >= 0) {
        if (h_pad2[occupied[digi.getBarID()]].amp_ < digi.getPE()) {
          h_pad2[occupied[digi.getBarID()]].b_id_ =
              (ap_int<12>)(digi.getBarID());
          h_pad2[occupied[digi.getBarID()]].m_id_ =
              (ap_int<12>)(digi.getModuleID());
          h_pad2[occupied[digi.getBarID()]].amp_ = (ap_int<12>)(digi.getPE());
          h_pad2[occupied[digi.getBarID()]].time_ =
              (ap_int<12>)(digi.getTime());
        }
      } else {
        h_pad2[count].b_id_ = (ap_int<12>)(digi.getBarID());
        h_pad2[count].m_id_ = (ap_int<12>)(digi.getModuleID());
        h_pad2[count].amp_ = (ap_int<12>)(digi.getPE());
        h_pad2[count].time_ = (ap_int<12>)(digi.getTime());
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
    if ((digi.getPE() > min_thr_) and (digi.getBarID() <= NCHAN) and
        (digi.getBarID() >= 0)) {
      ap_int<12> b_id = (ap_int<12>)(digi.getBarID());
      ap_int<12> amp = (ap_int<12>)(digi.getPE());
      if (occupied[digi.getBarID()] >= 0) {
        if (h_pad3[occupied[digi.getBarID()]].amp_ < digi.getPE()) {
          h_pad3[occupied[digi.getBarID()]].b_id_ =
              (ap_int<12>)(digi.getBarID());
          h_pad3[occupied[digi.getBarID()]].m_id_ =
              (ap_int<12>)(digi.getModuleID());
          h_pad3[occupied[digi.getBarID()]].amp_ = (ap_int<12>)(digi.getPE());
          h_pad3[occupied[digi.getBarID()]].time_ =
              (ap_int<12>)(digi.getTime());
        }
      } else {
        h_pad3[count].b_id_ = (ap_int<12>)(digi.getBarID());
        h_pad3[count].m_id_ = (ap_int<12>)(digi.getModuleID());
        h_pad3[count].amp_ = (ap_int<12>)(digi.getPE());
        h_pad3[count].time_ = (ap_int<12>)(digi.getTime());
        occupied[digi.getBarID()] = count;
        count++;
      }
    }
  }
  // These next lines here calls clusterproducerSw(HPad1), which is just the
  // validated firmware module. Since ap_* class is messy, I had to do some
  // post-call cleanup before looping over the clusters and putting them into
  // Point i which is feed into track producer
  int counter_n = 0;
  std::array<Cluster, NCLUS> point1 = clusterproducerSw(h_pad1);
  int top_seed = 0;
  for (int i = 0; i < NCLUS; i++) {
    if ((point1[i].seed_.amp_ < 450) and (point1[i].seed_.amp_ > 30) and
        (point1[i].seed_.b_id_ < (NCHAN + 1)) and
        (point1[i].seed_.b_id_ >= 0) and (point1[i].sec_.amp_ < 450) and
        (counter_n < NTRK)) {
      if (point1[i].seed_.b_id_ >= top_seed) {
        cpyHit(pad1[counter_n].seed_, point1[i].seed_);
        cpyHit(pad1[counter_n].sec_, point1[i].sec_);
        calcCent(pad1[counter_n]);
        counter_n++;
        top_seed = point1[i].seed_.b_id_;
      }
    }
  }
  std::array<Cluster, NCLUS> point2 = clusterproducerSw(h_pad2);
  top_seed = 0;
  for (int i = 0; i < NCLUS; i++) {
    if ((point2[i].seed_.amp_ < 450) and (point2[i].seed_.amp_ > 30) and
        (point2[i].seed_.b_id_ < (NCHAN + 1)) and
        (point2[i].seed_.b_id_ >= 0) and (point2[i].sec_.amp_ < 450)) {
      if (point2[i].seed_.b_id_ >= top_seed) {
        cpyHit(pad2[i].seed_, point2[i].seed_);
        cpyHit(pad2[i].sec_, point2[i].sec_);
        calcCent(pad2[i]);
        top_seed = point2[i].seed_.b_id_;
      }
    }
  }
  std::array<Cluster, NCLUS> point3 = clusterproducerSw(h_pad3);
  top_seed = 0;
  for (int i = 0; i < NCLUS; i++) {
    if ((point3[i].seed_.amp_ < 450) and (point3[i].seed_.amp_ > 30) and
        (point3[i].seed_.b_id_ < (NCHAN + 1)) and
        (point3[i].seed_.b_id_ >= 0) and (point3[i].sec_.amp_ < 450)) {
      if (point3[i].seed_.b_id_ >= top_seed) {
        cpyHit(pad3[i].seed_, point3[i].seed_);
        cpyHit(pad3[i].sec_, point3[i].sec_);
        calcCent(pad3[i]);
        top_seed = point3[i].seed_.b_id_;
      }
    }
  }
  // I have stagged the digis into firmware digi objects and paired them into
  // firmware cluster objects, so at this point I can insert them and the LUT
  // into the trackproducerHw to create the track collection I use makeTrack to
  // revert the firmware track object back into a regular track object for
  // analysis purposes
  //
  // NOTE: Pad1 has NTRK instead of NCLUS clusters for a reason: the firmware
  // cannot facilitate NCLUS many tracks within its alloted bandwidth , we have
  // to put a cut on them which is facilitated by a cut on the number of
  // clusters in Pad1. Do not change this.
  trackproducerHw(pad1, pad2, pad3, out_trk, lookup);
  for (int i = 0; i < NTRK; i++) {
    if (out_trk[i].pad1_.seed_.amp_ > 0. && out_trk[i].pad1_.sec_.amp_ >= 0. &&
        out_trk[i].pad2_.seed_.amp_ > 0. && out_trk[i].pad2_.sec_.amp_ >= 0. &&
        out_trk[i].pad3_.seed_.amp_ > 0. && out_trk[i].pad3_.sec_.amp_ >= 0.) {
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
  pe += static_cast<float>(outTrk.pad1_.seed_.amp_) +
        static_cast<float>(outTrk.pad1_.sec_.amp_);
  pe += static_cast<float>(outTrk.pad2_.seed_.amp_) +
        static_cast<float>(outTrk.pad2_.sec_.amp_);
  pe += static_cast<float>(outTrk.pad3_.seed_.amp_) +
        static_cast<float>(outTrk.pad3_.sec_.amp_);
  tr.setCentroid(calcTCent(outTrk));
  calcResid(outTrk);
  tr.setPE(pe);
  return tr;
}

}  // namespace trigscint

DECLARE_PRODUCER(trigscint::TrigScintFirmwareTracker);
