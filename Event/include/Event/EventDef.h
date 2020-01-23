/**
 * @file EventDef.h
 * @brief Headers to be processed when creating the ROOT dictionary
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Event/CalorimeterHit.h"
#include "Event/EcalHit.h"
#include "Event/EcalDigiCollection.h"
#include "Event/EcalVetoResult.h"
#include "Event/NonFidEcalVetoResult.h"
#include "Event/EcalCluster.h"
#include "Event/EventConstants.h"
#include "Event/EventHeader.h"
#include "Event/FindableTrackResult.h"
#include "Event/RunHeader.h"
#include "Event/SimCalorimeterHit.h"
#include "Event/SimTrackerHit.h"
#include "Event/SimParticle.h"
#include "Event/TriggerResult.h"
#include "Event/TrackerVetoResult.h"
#include "Event/ClusterAlgoResult.h"
#include "Event/HcalHit.h"
#include "Event/HcalVetoResult.h"
#include "Event/PnWeightResult.h"
#include "Event/ProductTag.h"
#include "Event/SiStripHit.h"
#include "Event/RawHit.h"
#include "Event/DigiCollection.h" 

#include <variant>

namespace ldmx {

    /**
     * @type EventBusPassenger
     * Allows all event bus passenger objects to be handled under one name (without an inheritance tree).
     * 
     * @note Any object that you want to be put into the event bus needs to be listed here as well as above.
     *
     * @note There are two types of EventBusPassengers:
     *      1) Those that exist inside of an STL collection (e.g. SimParticle or EcalHit).
     *          These must have Print and operator< (for sorting) methods defined.
     *      2) Those that exist outside an STL collection (e.g. EcalVetoResult)
     *          These must have Print and Clear methods defined.
     *
     * TODO: Figure out if there are a maximum number of types allowed.
     *
     * TODO: Dynamically (pre-processor) set the EventBusPassenger object to be the objects being used in the run.
     */
    typedef std::variant< 
        EcalVetoResult ,
        NonFidEcalVetoResult ,
        EventHeader ,
        TriggerResult ,
        TrackerVetoResult ,
        ClusterAlgoResult ,
        HcalVetoResult ,
        PnWeightResult ,
        DigiCollection ,
        EcalDigiCollection ,
        std::vector< FindableTrackResult > ,
        std::vector< SimCalorimeterHit > ,
        std::vector< SimTrackerHit > ,
        std::map< int , SimParticle > ,
        std::vector< CalorimeterHit > ,
        std::vector< EcalHit > ,
        std::vector< EcalCluster > ,
        std::vector< HcalHit > ,
        std::vector< SiStripHit > ,
        std::vector< RawHit >
        > EventBusPassenger;

}
