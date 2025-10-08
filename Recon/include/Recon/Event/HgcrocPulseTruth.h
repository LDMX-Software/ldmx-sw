#ifndef RECON_EVENT_HGCROCPULSETRUTH_H_
#define RECON_EVENT_HGCROCPULSETRUTH_H_

#include <iostream>
#include <optional>
#include <vector>

#include "Recon/Event/CompositePulse.h"
#include "TObject.h"  //for ClassDef

namespace ldmx {

class HgcrocPulseTruth;
typedef std::vector<HgcrocPulseTruth> HgcrocPulseTruthCollection;

class HgcrocPulseTruth {
 public:
  HgcrocPulseTruth(unsigned int id, ldmx::CompositePulse p)
      : id_(id), composite_pulse_(p){};

  /// default constructor needed for std::vector::resize
  HgcrocPulseTruth() = default;

  virtual ~HgcrocPulseTruth() = default;

  void clear();

  double getMax() const;

  unsigned int getID() const { return id_; }

 private:
  unsigned int id_;
  ldmx::CompositePulse composite_pulse_;

  ClassDef(HgcrocPulseTruth, 1);
};

}  // namespace ldmx

#endif
