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
#include <typeinfo> 
#include <type_traits>
#include <vector>

/*~~~~~~~~~~~~~~~*/
/*   Exception   */
/*~~~~~~~~~~~~~~~*/
#include "Exception/Exception.h" 

namespace ldmx {

    /**
     * Class encapsulating parameters for configuring a processor. 
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
             * @param [in, out] parameters mapping between parameter names and
             *      the corresponding value.
             */
            void setParameters(std::map < std::string, std::any > parameters) { 
                parameters_ = parameters; 
            }

            /**
             * Retrieve the parameter of the given name.
             *
             * @tparam T the data type to cast the parameter to.
             * 
             * @param name the name of the parameter value to retrieve.
             *
             * @return The user specified parameter.
             */
            template <typename T> 
            T getParameter(const std::string& name) { 
                
                // Check if the variable exists in the map.  If it doesn't, 
                // warn the user and set a default.
                if (parameters_.count(name) == 0) { 
               
                    if constexpr (std::numeric_limits<T>::is_integer || std::is_floating_point<T>::value) {
                        return std::numeric_limits<T>::lowest(); 
                    } else return {};  
                }

                T parameter; 
                try { 
                    parameter = std::any_cast< T >(parameters_[name]);
                } catch(const std::bad_any_cast& e) {
                    EXCEPTION_RAISE("Parameters::getParameter", 
                                    "Parameter " + name + " is being cast to incorrect type ( " + typeid(T).name() + ")."); 
                }

               return parameter; 
            }                

        private:

            /// Parameters 
            std::map < std::string, std::any > parameters_; 
    
    }; // Parameters

} // ldmx

#endif // FRAMEWORK_PARAMETERS_H
