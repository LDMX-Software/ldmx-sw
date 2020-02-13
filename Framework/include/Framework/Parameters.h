/**
 * @file Parameters.h
 * @brief Class encapsulating parameters for configuring a processor.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef FRAMEWORK_PARAMETERS_H
#define FRAMEWORK_PARAMETERS_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <any>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <type_traits>
#include <vector> 

namespace ldmx {

    /**
     *
     */
    class Parameters {
        
        public: 

            /// Constructor
            Parameters() {};

            /// Destructor
            ~Parameters() {}; 

            /**
             * Set the mapping of parameter names to value.
             *
             * @param mapping between parameter names and the corresponding value.
             */
            void setParameters(std::map < std::string, std::any > parameters) { 
                parameters_ = parameters; 
            }

            /**
             *
             */
            template <typename T> 
            T getParameter(const std::string& name) { 
                
                // Check if the variable exists in the map.  If it doesn't, 
                // warn the user and set a default.
                if (parameters_.count(name) == 0) { 
                    std::cout << "[ Parameters ]: Parameter " << name
                              << " hasn't been set. Using default value." << std::endl; 
               
                    if constexpr (std::is_integral<T>::value || std::is_floating_point<T>::value) {
                        return std::numeric_limits<T>::lowest(); 
                    } else return {};  
                }

                return std::any_cast< T >(parameters_[name]); 
            }                

        private:

            /// Parameters 
            std::map < std::string, std::any > parameters_; 
    
    }; // Parameters

} // ldmx

#endif // FRAMEWORK_PARAMETERS_H
