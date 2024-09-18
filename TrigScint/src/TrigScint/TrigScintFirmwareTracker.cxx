
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
  ap_int<12> A[3] = {0, 0, 0};
  ap_int<12> LOOKUP[NCENT][COMBO][2];

  // This line fills in the LOOKUP table used for patter matching latter. The
  // array takes in as its first argument the centroid of a first pad cluster,
  // then the next two take on which track pattern (of ~9) we are matching to
  // and the last if we are matching to a cluster with two hits
  for (int i = 0; i < NCENT; i++) {
    for (int j = 0; j < COMBO; j++) {
      LOOKUP[i][j][0] = (i - A[1] + A[0]);
      LOOKUP[i][j][1] = (i - A[2] + A[0]);
      if (j / 3 == 0) {
        LOOKUP[i][j][0] -= 1;
      } else if (j / 3 == 2) {
        LOOKUP[i][j][0] += 1;
      }
      if (j % 3 == 0) {
        LOOKUP[i][j][1] -= 1;
      } else if (j % 3 == 2) {
        LOOKUP[i][j][1] += 1;
      }
      if (not((LOOKUP[i][j][0] >= 0) and (LOOKUP[i][j][1] >= 0) and
              (LOOKUP[i][j][0] < NCENT) and (LOOKUP[i][j][1] < NCENT))) {
        LOOKUP[i][j][0] = -1;
        LOOKUP[i][j][1] = -1;
      }
    }
  }
  // Here we instantiate arrays necessary to do the rest of it.
  Hit HPad1[NHITS];
  Hit HPad2[NHITS];
  Hit HPad3[NHITS];

  Cluster Pad1[NCLUS];
  Cluster Pad2[NCLUS];
  Cluster Pad3[NCLUS];
  Track outTrk[NTRK];

  for (int j = 0; j < NHITS; j++) {
    clearHit(HPad1[j]);
    clearHit(HPad2[j]);
    clearHit(HPad3[j]);
  }
  for (int j = 0; j < NCLUS; j++) {
    if (j < NTRK) {
      clearClus(Pad1[j]);
    }
    clearClus(Pad2[j]);
    clearClus(Pad3[j]);
  }
  for (int j = 0; j < NTRK; j++) {
    clearTrack(outTrk[j]);
  }
  // I am reading in the three digi collections
  const auto digis1_{
      event.getCollection<ldmx::TrigScintHit>(digis1_collection_, passName_)};
  const auto digis3_{
      event.getCollection<ldmx::TrigScintHit>(digis2_collection_, passName_)};
  const auto digis2_{
      event.getCollection<ldmx::TrigScintHit>(digis3_collection_, passName_)};

  if (verbose_) {
    ldmx_log(debug) << "Got digi collection " << digis1_collection_ << "_"
                    << passName_ << " with " << digis1_.size() << " entries ";
  }

  // The next collection of things fill in the firmware hit objects from reading
  // in the digi collections the necessary information. The firmware hit objects
  // only keep bID,mID,Time, and PE count.
  int occupied[NCHAN];
  for (int i = 0; i < NCHAN; i++) {
    occupied[i] = -1;
  }
  int count = 0;
  for (const auto &digi : digis1_) {
    if ((digi.getPE() > minThr_) and (digi.getBarID() <= NCHAN) and
        (digi.getBarID() >= 0)) {
      ap_int<12> bID = (ap_int<12>)(digi.getBarID());
      ap_int<12> Amp = (ap_int<12>)(digi.getPE());
      int index = count;
      if (occupied[(int)digi.getBarID()] >= 0) {
        if (HPad1[(int)occupied[(int)digi.getBarID()]].Amp < digi.getPE()) {
          HPad1[(int)occupied[(int)digi.getBarID()]].bID =
              (ap_int<12>)(digi.getBarID());
          HPad1[(int)occupied[(int)digi.getBarID()]].mID =
              (ap_int<12>)(digi.getModuleID());
          HPad1[(int)occupied[(int)digi.getBarID()]].Amp =
              (ap_int<12>)(digi.getPE());
          HPad1[(int)occupied[(int)digi.getBarID()]].Time =
              (ap_int<12>)(digi.getTime());
        }
      } else {
        HPad1[count].bID = (ap_int<12>)(digi.getBarID());
        HPad1[count].mID = (ap_int<12>)(digi.getModuleID());
        HPad1[count].Amp = (ap_int<12>)(digi.getPE());
        HPad1[count].Time = (ap_int<12>)(digi.getTime());
        occupied[digi.getBarID()] = count;
        count++;
      }
    }
  }

  for (int i = 0; i < NCHAN; i++) {
    occupied[i] = -1;
  }
  count = 0;
  for (const auto &digi : digis2_) {
    if ((digi.getPE() > minThr_) and (digi.getBarID() <= NCHAN) and
        (digi.getBarID() >= 0)) {
      ap_int<12> bID = (ap_int<12>)(digi.getBarID());
      ap_int<12> Amp = (ap_int<12>)(digi.getPE());
      int index = count;
      if (occupied[(int)digi.getBarID()] >= 0) {
        if (HPad2[(int)occupied[(int)digi.getBarID()]].Amp < digi.getPE()) {
          HPad2[(int)occupied[(int)digi.getBarID()]].bID =
              (ap_int<12>)(digi.getBarID());
          HPad2[(int)occupied[(int)digi.getBarID()]].mID =
              (ap_int<12>)(digi.getModuleID());
          HPad2[(int)occupied[(int)digi.getBarID()]].Amp =
              (ap_int<12>)(digi.getPE());
          HPad2[(int)occupied[(int)digi.getBarID()]].Time =
              (ap_int<12>)(digi.getTime());
        }
      } else {
        HPad2[count].bID = (ap_int<12>)(digi.getBarID());
        HPad2[count].mID = (ap_int<12>)(digi.getModuleID());
        HPad2[count].Amp = (ap_int<12>)(digi.getPE());
        HPad2[count].Time = (ap_int<12>)(digi.getTime());
        occupied[digi.getBarID()] = count;
        count++;
      }
    }
  }
  for (int i = 0; i < NCHAN; i++) {
    occupied[i] = -1;
  }
  count = 0;
  for (const auto &digi : digis3_) {
    if ((digi.getPE() > minThr_) and (digi.getBarID() <= NCHAN) and
        (digi.getBarID() >= 0)) {
      ap_int<12> bID = (ap_int<12>)(digi.getBarID());
      ap_int<12> Amp = (ap_int<12>)(digi.getPE());
      int index = count;
      if (occupied[(int)digi.getBarID()] >= 0) {
        if (HPad3[(int)occupied[(int)digi.getBarID()]].Amp < digi.getPE()) {
          HPad3[(int)occupied[(int)digi.getBarID()]].bID =
              (ap_int<12>)(digi.getBarID());
          HPad3[(int)occupied[(int)digi.getBarID()]].mID =
              (ap_int<12>)(digi.getModuleID());
          HPad3[(int)occupied[(int)digi.getBarID()]].Amp =
              (ap_int<12>)(digi.getPE());
          HPad3[(int)occupied[(int)digi.getBarID()]].Time =
              (ap_int<12>)(digi.getTime());
        }
      } else {
        HPad3[count].bID = (ap_int<12>)(digi.getBarID());
        HPad3[count].mID = (ap_int<12>)(digi.getModuleID());
        HPad3[count].Amp = (ap_int<12>)(digi.getPE());
        HPad3[count].Time = (ap_int<12>)(digi.getTime());
        occupied[digi.getBarID()] = count;
        count++;
      }
    }
  }
  count = 0;
  // These next lines here calls clusterproducer_sw(HPad1), which is just the
  // validated firmware module. Since ap_* class is messy, I had to do some
  // post-call cleanup before looping over the clusters and putting them into
  // Point i which is feed into track producer
  int counterN = 0;
  std::array<Cluster,NCLUS> Point1 = clusterproducer_sw(HPad1);
  int topSeed = 0;
  for (int i = 0; i < NCLUS; i++) {
    if ((Point1[i].Seed.Amp < 450) and (Point1[i].Seed.Amp > 30) and
        (Point1[i].Seed.bID < (NCHAN + 1)) and (Point1[i].Seed.bID >= 0) and
        (Point1[i].Sec.Amp < 450) and (counterN < NTRK)) {
      if (Point1[i].Seed.bID >= topSeed) {
        cpyHit(Pad1[counterN].Seed, Point1[i].Seed);
        cpyHit(Pad1[counterN].Sec, Point1[i].Sec);
        calcCent(Pad1[counterN]);
        counterN++;
        topSeed = Point1[i].Seed.bID;
      }
    }
  }
  std::array<Cluster,NCLUS> Point2 = clusterproducer_sw(HPad2);
  topSeed = 0;
  for (int i = 0; i < NCLUS; i++) {
    if ((Point2[i].Seed.Amp < 450) and (Point2[i].Seed.Amp > 30) and
        (Point2[i].Seed.bID < (NCHAN + 1)) and (Point2[i].Seed.bID >= 0) and
        (Point2[i].Sec.Amp < 450)) {
      if (Point2[i].Seed.bID >= topSeed) {
        cpyHit(Pad2[i].Seed, Point2[i].Seed);
        cpyHit(Pad2[i].Sec, Point2[i].Sec);
        calcCent(Pad2[i]);
        topSeed = Point2[i].Seed.bID;
      }
    }
  }
  std::array<Cluster,NCLUS> Point3 = clusterproducer_sw(HPad3);
  topSeed = 0;
  for (int i = 0; i < NCLUS; i++) {
    if ((Point3[i].Seed.Amp < 450) and (Point3[i].Seed.Amp > 30) and
        (Point3[i].Seed.bID < (NCHAN + 1)) and (Point3[i].Seed.bID >= 0) and
        (Point3[i].Sec.Amp < 450)) {
      if (Point3[i].Seed.bID >= topSeed) {
        cpyHit(Pad3[i].Seed, Point3[i].Seed);
        cpyHit(Pad3[i].Sec, Point3[i].Sec);
        calcCent(Pad3[i]);
        topSeed = Point3[i].Seed.bID;
      }
    }
  }
  // I have stagged the digis into firmware digi objects and paired them into
  // firmware cluster objects, so at this point I can insert them and the LUT
  // into the trackproducer_hw to create the track collection I use makeTrack to
  // revert the firmware track object back into a regular track object for
  // analysis purposes
  trackproducer_hw(Pad1, Pad2, Pad3, outTrk, LOOKUP);
  for (int I = 0; I < NTRK; I++) {
    if (outTrk[I].Pad1.Seed.Amp > 0) {
      ldmx::TrigScintTrack trk = makeTrack(outTrk[I]);
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
  float pe = outTrk.Pad1.Seed.Amp + outTrk.Pad1.Sec.Amp;
  pe += outTrk.Pad2.Seed.Amp + outTrk.Pad2.Sec.Amp;
  pe += outTrk.Pad3.Seed.Amp + outTrk.Pad3.Sec.Amp;
  tr.setCentroid(calcTCent(outTrk));
  calcResid(outTrk);
  tr.setPE(pe);
  return tr;
}

}  // namespace trigscint

DECLARE_PRODUCER_NS(trigscint, TrigScintFirmwareTracker);
