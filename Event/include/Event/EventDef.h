/**
 * @file EventDef.h
 * @brief Headers to be processed when creating the ROOT dictionary
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Event/CalorimeterHit.h"
#include "Event/ClusterAlgoResult.h"
#include "Event/EcalCluster.h"
#include "Event/EcalHit.h"
#include "Event/EcalDigiCollection.h"
#include "Event/EcalVetoResult.h"
#include "Event/EventConstants.h"
#include "Event/EventHeader.h"
#include "Event/FindableTrackResult.h"
#include "Event/HcalHit.h"
#include "Event/HcalVetoResult.h"
#include "Event/NonFidEcalVetoResult.h"
#include "Event/PnWeightResult.h"
#include "Event/ProductTag.h"
#include "Event/RawHit.h"
#include "Event/RunHeader.h"
#include "Event/SimCalorimeterHit.h"
#include "Event/SimParticle.h"
#include "Event/SiStripHit.h"
#include "Event/SimTrackerHit.h"
#include "Event/TrackerVetoResult.h"
#include "Event/TriggerResult.h"
#include "Event/TrigScintHit.h" 
#include "Event/TrigScintQIEDigis.h" 

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
        ClusterAlgoResult,
        EcalDigiCollection,
        EcalVetoResult,
        EventHeader,
        HcalVetoResult,
        NonFidEcalVetoResult,
        PnWeightResult,
        TrackerVetoResult,
        TriggerResult,
        TrigScintHit,
        TrigScintQIEDigis,
        std::vector < CalorimeterHit >,
        std::vector < EcalCluster >,
        std::vector < EcalHit >,
        std::vector < FindableTrackResult >,
        std::vector < HcalHit >,
        std::vector < RawHit >, 
        std::vector < SimCalorimeterHit >,
        std::vector < SiStripHit >,
        std::vector < SimTrackerHit >,
        std::vector < TrigScintHit >,
        std::vector < TrigScintQIEDigis >,
        std::map< int , SimParticle >
    > EventBusPassenger;

}
