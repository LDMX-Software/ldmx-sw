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
             *
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
