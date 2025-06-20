//
// Created by Wesley Ketchum on 4/26/24.
//
// This is just a simple extension of the HepMC3::GenEvent
// for use on the ldmx Event Bus.
//

#ifndef SIMCORE_HEPMC3GENEVENT_H
#define SIMCORE_HEPMC3GENEVENT_H

#include "HepMC3/Data/GenEventData.h"
#include "HepMC3/GenEvent.h"
#include "TObject.h"

namespace ldmx {

class HepMC3GenEvent : public HepMC3::GenEventData {
 public:
  virtual ~HepMC3GenEvent() = default;

  void Clear();
  void Print() const;

  HepMC3::GenEvent getHepMCGenEvent() const;

  std::string get_as_string() const;

 public:
  ClassDef(HepMC3GenEvent, 1);
};

}  // namespace ldmx

#endif  // SIMCORE_HEPMC3GENEVENT_H
