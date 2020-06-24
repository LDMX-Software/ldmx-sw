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
             * @throw Exception if parameter of the given name isn't found
             *
             * @throw Exception if parameter is found but not of the input type
             *
             * @param T the data type to cast the parameter to.
             * 
             * @param[in] name the name of the parameter value to retrieve.
             *
             * @return The user specified parameter of type T.
             */
            template <typename T> 
            T getParameter(const std::string& name) const { 
                
                // Check if the variable exists in the map.  If it doesn't, 
                // raise an exception.
                if (parameters_.count(name) == 0) {
                    EXCEPTION_RAISE( "NonExistParam",
                            "Parameter '"+name+"' does not exist in list of parameters."
                            );
                }

                T parameter; 
                try { 
                    parameter = std::any_cast< T >(parameters_.at(name));
                } catch(const std::bad_any_cast& e) {
                    EXCEPTION_RAISE( "BadTypeParam",
                                    "Parameter '" + name + "' is being cast to incorrect type '" + typeid(T).name() + "'."); 
                }

                return parameter; 
            }

            /**
             * Retrieve a parameter with a default specified.
             *
             * Return the input default if a parameter is not found in map.
             *
             * @return the user parameter of type T
             */
            template <typename T>
            T getParameter(const std::string& name, const T& def ) const {

                if ( parameters_.count(name) == 0 ) return def;

                //get here knowing that name exists in parameters_
                return getParameter<T>(name);
            }

        private:

            /// Parameters 
            std::map < std::string, std::any > parameters_; 
    
    }; // Parameters

} // ldmx

#endif // FRAMEWORK_PARAMETERS_H
