#ifndef OBJDEF_H
#define OBJDEF_H

#include "ap_int.h"
#define NTIMES 5
#define NHITS 25
#define NCLUS 25
#define NCHAN 50
#define NTRK 10
#define W = 10

#define NCENT 99

#define NDIGIS 14
#define COMBO 9

// 2*NCHAN*NTIMES are the number of bytes per event plus 4+4+4+3+1 bytes for the
// header

#define NSAMPLES 6

// NSAMPLES/8 is the number of 64 bit words

#define NWORDS 72

struct Digi {
  int m_id_{}, b_id_{};
  int adc0_{}, adc1_{}, adc2_{}, adc3_{}, adc4_{}, adc5_{};
  int tdc0_{}, tdc1_{}, tdc2_{}, tdc3_{}, tdc4_{}, tdc5_{};
};

inline void clearDigi(Digi& c) {
  c.m_id_ = 0;
  c.b_id_ = 0;
  c.adc0_ = 0;
  c.adc1_ = 0;
  c.adc2_ = 0;
  c.adc3_ = 0;
  c.adc4_ = 0;
  c.adc5_ = 0;
  c.tdc0_ = 0;
  c.tdc1_ = 0;
  c.tdc2_ = 0;
  c.tdc3_ = 0;
  c.tdc4_ = 0;
  c.tdc5_ = 0;
}

struct Hit {
  ap_int<12> m_id_{}, b_id_{};
  ap_int<12> amp_{}, time_{};  // TrigTime;
};

inline void clearHit(Hit& c) {
  c.m_id_ = 0;
  c.b_id_ = -1;
  c.amp_ = 0;
  c.time_ = 0;  // c.TrigTime=0.0;
}

inline void cpyHit(Hit& c1, Hit& c2) {
  c1.m_id_ = c2.m_id_;
  c1.b_id_ = c2.b_id_;
  c1.amp_ = c2.amp_;
  c1.time_ = c2.time_;
}

struct Cluster {
  Hit seed_{};
  Hit sec_{};
  ap_int<12> cent_{};
  // int nhits, mID, SeedID;
  // float CentX, CentY, CentZ, Amp, Time, TrigTime;
};

inline void clearClus(Cluster& c) {
  clearHit(c.seed_);
  clearHit(c.sec_);
  c.cent_ = (ap_int<12>)(0);  // clearHit(c.For);
}

inline void calcCent(Cluster& c) {
  // Check if Seed and Sec amplitudes are valid
  if (c.seed_.amp_ <= 0 || c.sec_.amp_ <= 0) {
    c.cent_ = (ap_int<12>)(0);
    return;
  }

  if (c.seed_.b_id_ < 0 || c.sec_.b_id_ < 0) {
    c.cent_ = (ap_int<12>)(0);
    return;
  }

  // Perform the centroid calculation if all checks passed
  c.cent_ =
      (ap_int<12>)(10.0f *
                   ((float)(c.seed_.amp_ * c.seed_.b_id_ + c.sec_.amp_ * c.sec_.b_id_)) /
                   ((float)(c.seed_.amp_ + c.sec_.amp_)));
}

inline void cpyCluster(Cluster& c1, Cluster& c2) {
  cpyHit(c1.seed_, c2.seed_);
  cpyHit(c1.sec_, c2.sec_);
}

struct Track {
  Cluster pad1_{};
  Cluster pad2_{};
  Cluster pad3_{};
  ap_int<12> resid_{};
};

inline void clearTrack(Track& c) {
  clearClus(c.pad1_);
  clearClus(c.pad2_);
  clearClus(c.pad3_);
  c.resid_ = 5000;
}

inline ap_int<12> calcTCent(Track& c) {
  calcCent(c.pad1_);
  calcCent(c.pad2_);
  calcCent(c.pad3_);

  float one = (float)c.pad1_.cent_;
  float two = (float)c.pad2_.cent_;
  float three = (float)c.pad3_.cent_;
  float mean = (one + two + three) / 3.0;
  ap_int<12> cent = (ap_int<12>)((int)(mean));
  return cent;
}

inline void calcResid(Track& c) {
  calcCent(c.pad1_);
  calcCent(c.pad2_);
  calcCent(c.pad3_);
  float one = (float)c.pad1_.cent_;
  float two = (float)c.pad2_.cent_;
  float three = (float)c.pad3_.cent_;
  float mean = (one + two + three) / 3.0;
  c.resid_ = (ap_int<12>)((int)(((one - mean) * (one - mean) +
                                (two - mean) * (two - mean) +
                                (three - mean) * (three - mean)) /
                               3.0));
}

inline void cpyTrack(Track& c1, Track& c2) {
  cpyCluster(c1.pad1_, c2.pad1_);
  cpyCluster(c1.pad2_, c2.pad2_);
  cpyCluster(c1.pad3_, c2.pad3_);
  c1.resid_ = c2.resid_;
}

#endif
