/**
 * @file GammaToMuPairXsecBiasingPlugin.h
 * @brief Geant4 Biasing Operator used to bias the occurence of muon pair
 *        conversions by modifying the cross-section.
 * @author Omar Moreno
 *         SLAC National Accelerator Laboratory
 */

#ifndef BIASING_GAMMATOMUPAIRXSECBIASINGOPERATOR_H_
#define BIASING_GAMMATOMUPAIRXSECBIASINGOPERATOR_H_

#include "SimCore/PhotoNuclearXsecBiasingOperator.h"

namespace simcore {

class GammaToMuPairXsecBiasingOperator
    : public PhotoNuclearXsecBiasingOperator {
 public:
  /** Constructor */
  GammaToMuPairXsecBiasingOperator(std::string name);

  /** Destructor */
  ~GammaToMuPairXsecBiasingOperator();

 protected:
  virtual std::string getProcessToBias() { return GAMMATOMUPAIR_PROCESS; }

 private:
  /** Geant4 gamma->mu+mu- process name. */
  static const std::string GAMMATOMUPAIR_PROCESS;
};
}  // namespace simcore

#endif  // BIASING_GAMMATOMUPAIRXSECBIASINGOPERATOR_H_
