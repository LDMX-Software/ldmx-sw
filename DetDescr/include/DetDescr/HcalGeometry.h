/**
 * @file HcalGeometry.h
 * @brief Class that translates HCal ID into positions of bar hits
 */

#ifndef DETDESCR_HCALGEOMETRY_H_
#define DETDESCR_HCALGEOMETRY_H_

// LDMX
#include "Framework/Exception/Exception.h"
#include "DetDescr/HcalID.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/ConditionsObject.h"

// STL
#include <map>

namespace hcal {
class HcalGeometryProvider;
}

namespace ldmx {

    class HcalGeometryProvider;
  
    /**
     * @class HcalGeometry
     * @brief Implementation of HCal bar readout
     *
     */
    class HcalGeometry : public framework::ConditionsObject {

        public:
            static constexpr const char* CONDITIONS_OBJECT_NAME{"HcalGeometry"};

            /**
             * Class destructor.
             *
             * Does nothing because the stl containers clean up automatically.
             */
            virtual ~HcalGeometry() { }

            /**
             * Get entire real space position for the strip with the input raw ID
             *
             * Inputs x,y,z will be set to the calculated position
             *
             * @sa getStripCenterAbsolute and getZPosition
             *
             * @param[in] id HcalID for the strip we want the position of
             * @param[out] xy set to x/y-coordinate of strip center
             * @param[out] z set to z-coordinate of strip center
             */
            void getStripAbsolutePosition( ldmx::HcalID id, double &x, double &y, double &z ) const {
	        std::tuple<double,double,double> xyz = this->getStripCenterAbsolute( id );
	        x = std::get<0>(xyz);
		y = std::get<1>(xyz);
                z = std::get<2>(xyz);
                return;
            }

            /**
             * Get a strip center X, Y and Z position relative to hcal center from a combined hcal ID
             *
             * @throw std::out_of_range if HcalID isn't created with valid bar or bar IDs.
             *
             * @param HcalID 
             * @return The X, Y and Z position of the center of the bar.
             */
            std::tuple<double,double,double> getStripCenterAbsolute(ldmx::HcalID id) const {
	      return stripPositionMap_.at(id);
            }

            /**
	     * Get half total width for back Hcal
	     */
            double halfTotalWidth() const {
	      return hcalHalfTotalWidthBack_;
	    }

      static HcalGeometry* debugMake(const framework::config::Parameters& p) { return new HcalGeometry(p); }
    
        private:

            /**
             * Class constructor, for use only by the provider
             *
             * @param ps Parameters to configure the HcalGeometry
             */
            HcalGeometry(const framework::config::Parameters &ps);
            friend class hcal::HcalGeometryProvider;

            void buildStripPositionMap();

        private:

            /// verbosity, not configurable but helpful if developing
            int verbose_{2};

	    /// thickness of scintillator 
	    double hcalThicknessScint_{0.};

	    /// Width of Scintillator Strip [mm]
            double hcalWidthScint_{0.};

            /// Front of HCal relative to world geometry for each section [mm]
	    std::vector<double> hcalZeroLayer_;

            /// The plane of the zero'th strip of each section [mm] 
	    std::vector<double> hcalZeroStrip_;

	    /// Thickness of the layers in each section [mm] 
	    std::vector<double> hcalLayerThickness_;

	    /// Number of layers in each section 
	    std::vector<int> hcalNLayers_;

            /// Number of strips per layer in each section
	    std::vector<int> hcalNStrips_;

            /// Position of bar centers relative to world geometry (uses ID with real bar and section and layer as zero for key)
            std::map<ldmx::HcalID, std::tuple<double,double,double>> stripPositionMap_;

            /// Half Total Width for bars at back Hcal
            double hcalHalfTotalWidthBack_{0.};
    };
    
}

#endif
