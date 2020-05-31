#ifndef BIASING_PHOTONUCLEARPRODUCTSFILTER_H
#define BIASING_PHOTONUCLEARPRODUCTSFILTER_H 

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimApplication/UserAction.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Parameters.h" 

// Forward declaration
class G4Step; 

namespace ldmx { 

    /**
     * User action used to filter out photo-nuclear events that don't see 
     * the photo-nuclear gamma produce specific products.
     *
     * This user action will only process steps whose associated track 
     * has been tagged as a "PN Gamma".  This tag is currently only set in 
     * ECalProcessFilter and needs to be placed in the UserAction pipeline
     * before this class. The desired products are passed to this class 
     * through the parameter vector<int> parameter "pdg_ids".  
     *
     */
    class PhotoNuclearProductsFilter : public UserAction { 
    
        public: 

            /**
             * Constructor
             *
             * @param[in] name The name of this class instance.
             * @param[in] parameters The parameters used to configure this class.
             */
            PhotoNuclearProductsFilter(const std::string& name, Parameters& parameters); 
            
            /// Destructor
            ~PhotoNuclearProductsFilter();

            /**
             * Callback that allows a user to take some actions at the end of
             * a step.  
             *
             * @param[in] step The Geant4 step containing transient information
             *      about the step taken by a track.  
             */
            void stepping(const G4Step* step) final override; 
            
            /// Retrieve the type of actions this class defines
            std::vector< TYPE > getTypes() final override { 
                return { TYPE::STEPPING }; 
            } 

        private:
    
            /// Container to hold the PDG IDs of products of interest
            std::vector< int > productsPdgID_; 

    }; // PhotoNuclearProductsFilter

} // ldmx 

#endif // BIASING_PROTONUCLEARPRODUCTSFILTER_H 
