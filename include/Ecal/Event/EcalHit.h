#ifndef ECAL_EVENT_ECALHIT_H_
#define ECAL_EVENT_ECALHIT_H_

//----------//
//   LDMX   //
//----------//
#include "Recon/Event/CalorimeterHit.h"

namespace ecal {
namespace event {

/**
 * @class EcalHit
 * @brief Stores reconstructed hit information from the ECAL
 *
 * @note This class represents the reconstructed hit information
 * from the ECAL, providing particular information for the ECAL,
 * above and beyond what is available in the CalorimeterHit.
 */
class EcalHit : public recon::event::CalorimeterHit {
 public:
  /** Constructor. */
  EcalHit() {}

  /** Destructor. */
  virtual ~EcalHit() {}

  /** Clear the data in the object. */
  void Clear();

  /** Print a text representation of this object. */
  void Print() const;

 private:
  /** The ROOT class definition. */
  ClassDef(EcalHit, 3);
};
}
}  // namespace ecal

#endif /* EVENT_ECALHIT_H_ */
