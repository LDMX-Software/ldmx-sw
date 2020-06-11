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
             * @throw Exception if the parameter isn't found
             * 
             * @throw Exception if parameter is being cast to the wrong type
             *
             * @param T the data type to cast the parameter to.
             * 
             * @param[in] name the name of the parameter value to retrieve.
             * @param[in] defaultParam the value the parameter should take on 
             *      if it's not found in the list of parameters.  
             *
             * @return The user specified parameter of type T.
             */
            template <typename T> 
            T getParameter(const std::string& name) { 
                
                // Check if the variable exists in the map.
                // If it doesn't, throw an exception.
                if (parameters_.count(name) == 0) {
                    std::cerr << "[ WARN ] [ ParamName ] : "
                        << "Parameter '" << name << "' was not passed in the python configuration. "
                        << "The default constructor will be used."
                        << std::endl;
                    return T();
                    /* Do we want to not run or just print a warning?
                    EXCEPTION_RAISE( "ParamName",
                            "Parameter '" + name + "' does not exist in python configuration parameters."
                            );
                    */
                }

                T parameter; 
                try { 
                    parameter = std::any_cast< T >(parameters_[name]);
                } catch(const std::bad_any_cast& e) {
                    EXCEPTION_RAISE("ParamType", 
                                    "Parameter " + name + " is being cast to incorrect type ( " + typeid(T).name() + ")."); 
                }

                return parameter; 
            }

            /**
             * Retrieve the parameter of the given name.  
             *
             * @note Setting a default in the C++ source for your processor
             * will mean that the parameter doesn't need to be set in the
             * python. Only the parameters set in the python are tracked
             * in the meta-data, so use the default with caution.
             *
             * If the parameter isn't found, the default is returned instead.
             *
             * @throw Exception if parameter is being cast to the wrong type
             *
             * @param T the data type to cast the parameter to.
             * 
             * @param[in] name the name of the parameter value to retrieve.
             * @param[in] defaultParam the value the parameter should take on 
             *      if it's not found in the list of parameters.  
             *
             * @return The user specified parameter of type T.
             */
            template <typename T> 
            T getParameter(const std::string& name, T defaultParam) { 
                
                // Check if the variable exists in the map.  If it doesn't, 
                // warn the user and set a default.
                if (parameters_.count(name) == 0) return defaultParam; 

                // we now know variable exists in map, so we can use
                // the other get method
                return getParameter<T>(name); 
            }               

        private:

            /// Parameters 
            std::map < std::string, std::any > parameters_; 
    
    }; // Parameters

} // ldmx

#endif // FRAMEWORK_PARAMETERS_H
