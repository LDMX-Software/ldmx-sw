/**
 * @file TargetDarkBremFilter.h
 * @class TargetDarkBremFilter
 * @brief Class defining a UserActionPlugin that allows a user to filter out 
 *        events that don't result in a dark brem inside a given volume
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef BIASING_ECALDARKBREMFILTER_H_
#define BIASING_ECALDARKBREMFILTER_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <algorithm>

//------------//
//   Geant4   //
//------------//
#include "G4RunManager.hh"
#include "G4VPhysicalVolume.hh"

//------------//
//    LDMX    //
//------------//
#include "SimCore/UserAction.h"

namespace ldmx {

    /**
     * @class TargetDarkBremFilter
     *
     * This class is meant to filter for events that produce a dark brem matching
     * originating in the target and matching the following parameters.
     *
     *      threshold: minimum energy [MeV] A' needs to have
     *
     * @see TargetBremFilter
     * This filter is designed similar to the target brem filter where we check the
     * secondaries of the primary electron if it is stopping within the target or
     * if it is leaving the target region.
     */
    class TargetDarkBremFilter : public UserAction {

        public:

            /**
             * Class constructor.
             *
             * Retrieve the necessary configuration parameters
             */
            TargetDarkBremFilter(const std::string& name, Parameters& parameters);

            /**
             * Class destructor.
             */
            ~TargetDarkBremFilter() { }

            /**
             * Get the types of actions this class can do
             *
             * @return list of action types this class does
             */
            std::vector< TYPE > getTypes() final override {
                return { TYPE::STEPPING };
            }

            /**
             * Looking for A' while primary is stepping.
             *
             * We make sure that the current track
             * is the primary electron that is within
             * the target region. Then if the track
             * is either stopped or leaving the target region,
             * we look through its secondaries for a good A'.
             *
             * @param[in] step current G4Step
             */
            void stepping(const G4Step* step);

        private:

            /**
             * Check if the volume is in the target region
             *
             * @note will return false if vol is nullptr
             *
             * @param[in] vol G4VPhysicalVolume to check region
             * @returns true if vol is in target region
             */
            inline bool isInTargetRegion(const G4VPhysicalVolume* vol) const {
                return vol ? isInTargetRegion(vol->GetLogicalVolume()) : false;
            }

            /**
             * Check if the volume is in the target region
             *
             * @note will return false if vol or region is nullptr.
             *
             * @param[in] vol G4LogicalVolume to check region
             * @returns true if vol is in target region
             */
            inline bool isInTargetRegion(const G4LogicalVolume* vol) const {
                if (not vol) return false;
                auto region{vol->GetRegion()};
                return region ? (region->GetName().compareTo("target") == 0) : false;
            }

            /**
             * Helper to abort an event with a message
             *
             * Tells the RunManger to abort the current event
             * after displaying the input message.
             *
             * @param[in] reason reason for aborting the event
             */
            void AbortEvent(const std::string& reason) const;

        private:

            /**
             * Minimum energy [MeV] that the A' should have to keep the event.
             *
             * Also used by PartialEnergySorter to determine
             * which tracks should be processed first.
             * 
             * Parameter Name: 'threshold'
             */
            double threshold_;

    }; // TargetDarkBremFilter
}

#endif // BIASING_ECALDARKBREMFILTER_H__
