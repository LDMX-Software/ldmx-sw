/**
 * @file IntermediateCluster.h
 * @brief Type alias for Ecal cluster reconstruction
 */
#ifndef ECAL_INTERMEDIATECLUSTER_H_
#define ECAL_INTERMEDIATECLUSTER_H_

#include "Ecal/Event/EcalHit.h"
#include "Recon/WorkingCluster.h"

namespace ecal {

/**
 * @typedef IntermediateCluster
 * @brief Type alias for WorkingCluster specialized for EcalHit.
 */
using IntermediateCluster = recon::WorkingCluster<ldmx::EcalHit>;

}  // namespace ecal

#endif  // ECAL_INTERMEDIATECLUSTER_H_
