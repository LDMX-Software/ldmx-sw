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

#include <boost/variant.hpp>
namespace ldmx {

    /**
     * @type EventBusPassenger
     * Allows all event bus passenger objects to be handled under one name (without an inheritance tree).
     * 
     * @note Any object that you want to be put into the event bus needs to be listed here as well as above.
     * TODO: Modify to allow for more than 20 event bus types
     */
    using EventBusPassengerList = boost::mpl::list< 
        EcalVetoResult ,
        NonFidEcalVetoResult ,
        EventHeader ,
        TriggerResult ,
        TrackerVetoResult ,
        ClusterAlgoResult ,
        HcalVetoResult ,
        PnWeightResult ,
//        DigiCollection ,
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
        >;

    typedef boost::make_variant_over< EventBusPassengerList >::type EventBusPassenger;

}
