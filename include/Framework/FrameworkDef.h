/**
 * @file FrameworkDef.h 
 * @brief Class used to define commonly used variables.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef FRAMEWORK_FRAMEWORKDEF_H
#define FRAMEWORK_FRAMEWORKDEF_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <string> 

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Parameters.h" 

namespace ldmx { 

    /**
     * @struct HistogramInfo
     * @brief Encapsulates the information required to create a histogram 
     */
    struct HistogramInfo { 
     
        /// Name of the histogram 
        std::string name_;

        /// X axis label
        std::string xLabel_;  

        /// Bin Edges for X axis
        std::vector<double> xbins_; 

        /// Y axis label
        std::string yLabel_;  

        /// Bin Edges for Y axis (non-empty ==> 2D histogram)
        std::vector<double> ybins_; 

    };

    /**
     * @struct Class 
     * @brief Encapsualtes the information required to create a class 
     *        of type className_ 
     */
    struct Class { 

        /// Name of the class  
        std::string className_;

        /// Name of the instance of this class described by this object
        std::string instanceName_;

        /// The parameters associated with this class
        Parameters params_;

    };

} // ldmx

#endif // FRAMEWORK_FRAMEWORKDEF_H
