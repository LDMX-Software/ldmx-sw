/**
 * @file EcalHit.h
 * @brief Class that stores reconstructed hit information from the ECAL
 * @author Jeremy Mans, University of Minnesota
 */

#ifndef EVENT_ECALHIT_H_
#define EVENT_ECALHIT_H_

//----------//
//   LDMX   //
//----------//
#include "Recon/Event/CalorimeterHit.h"

namespace ldmx {

/**
 * @class EcalHit
 * @brief Stores reconstructed hit information from the ECAL
 *
 * @note This class represents the reconstructed hit information
 * from the ECAL, providing particular information for the ECAL,
 * above and beyond what is available in the CalorimeterHit.
 */
class EcalHit : public CalorimeterHit {
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
}  // namespace ldmx

#endif /* EVENT_ECALHIT_H_ */
