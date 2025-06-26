#ifndef RECON_EVENT_HGCROCPULSETRUTH_H_
#define RECON_EVENT_HGCROCPULSETRUTH_H_

#include "TObject.h"  //for ClassDef
#include "Recon/Event/CompositePulse.h"

#include <vector>
#include <iostream>
#include <optional>

namespace ldmx {

class HgcrocPulseTruth;
typedef std::vector<HgcrocPulseTruth> HgcrocPulseTruthCollection;

class HgcrocPulseTruth{
public:
  HgcrocPulseTruth(unsigned int id, ldmx::CompositePulse p) : id_(id), compositePulse_(p) {};

  HgcrocPulseTruth() {}; // Default constructor needed, because ROOT will call std::vector<HgcrocPulseTruth>::resize

  virtual ~HgcrocPulseTruth() {};

  void Clear();

  double getMax() const;

  unsigned int getID() const {
    return id_;
  }

private:
  unsigned int id_;
  ldmx::CompositePulse compositePulse_;

  ClassDef(HgcrocPulseTruth, 1);
};

} // namespace ldmx

#endif

