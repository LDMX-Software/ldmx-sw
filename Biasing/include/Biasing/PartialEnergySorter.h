#ifndef BIASING_PARTIALENERGYSORTER_H
#define BIASING_PARTIALENERGYSORTER_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <string>

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserAction.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Parameters.h" 

// Forward declarations
class G4Step;

namespace ldmx { 

    /**
     */
    class PartialEnergySorter : public UserAction { 
    
        public: 
            
            /**
             * Constructor.
             *
             * @param[in] name the name of the instance of this UserAction.
             * @param[in] parameters the parameters used to configure this UserAction.
             */
            PartialEnergySorter(const std::string& name, Parameters& parameters); 

            /// Destructor
            ~PartialEnergySorter();

            /**
             * Classify a "new" track
             *
             * This is called when a new track is created *or*
             * when a current track is suspended. We can use
             * this functionality to move any tracks below
             * the energy threshold to the waiting stack.
             *
             * @param aTrack The Geant4 track.
             * @param currentTrackClass The current track classification.
             * @returns the updated classification
             */
            G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* aTrack, 
                    const G4ClassificationOfNewTrack& currentTrackClass) final override; 

            /**
             * @param[in] step Geant4 step
             */
            void stepping(const G4Step* step) final override;  

            /// Retrieve the type of actions this class defines
            std::vector< TYPE > getTypes() final override { 
                return { TYPE::STEPPING }; 
            } 
            
        private: 

            /// Minimum Energy [MeV] we want to simulate first
            double threshold_; 

            /// Number of particles above the threshold
            int num_particles_above_threshold_;

    };  // PartialEnergySorter

} // ldmx

#endif // BIASING_TAGGERVETOFILTER_H 
