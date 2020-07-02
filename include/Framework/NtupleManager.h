
#ifndef _NTUPLE_MANAGER_H_
#define _NTUPLE_MANAGER_H_

/*~~~~~~~~~~~~*/
/*   StdLib   */
/*~~~~~~~~~~~~*/
#include <limits> 
#include <iostream> 
#include <map> 
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <variant>

/*~~~~~~~~~~*/
/*   ROOT   */
/*~~~~~~~~~~*/
#include "TTree.h" 

/*~~~~~~~~~~*/
/*   Core   */
/*~~~~~~~~~~*/
#include "Exception/Exception.h" 
#include "Framework/Logger.h"

namespace ldmx {

    /**
     * @class NtupleManager
     * @brief Singleton class used to manage the creation and pooling of 
     *        ntuples.
     */
    class NtupleManager { 

        public:

            /// @return The NtupleManager instance 
            static NtupleManager& getInstance(); 

            /**
             * Create a ROOT tree to hold the ntuple variables (ROOT leaves). 
             *
             * @param name Name of the tree.
             */
            void create(const std::string& tname);

            /**
             *  Add a variable of type T to the ROOT tree with name 'tname'.  
             *  If the variable already exists in any tree or the requested 
             *  tree does not exists, an exception is thrown.
             *
             *  @param tname Name of the tree to add the variable to.
             *  @param vname Name of the variable to add to the tree 
             *  @throws exception
             */
            template <typename T>
            void addVar(const std::string& tname, const std::string& vname) {
                
                // Check if the variable exists in the map. If it does, throw 
                // an exception.
                if (variables_.count(vname) != 0) { 
                    EXCEPTION_RAISE(
                            "NtupleManager", 
                            "A variable with name " + vname + " has already been defined."); 
                } 
        
                // Check if a tree named 'tname' has already been created.  If
                // not, throw an exception.
                if (trees_.count(tname) == 0) 
                    EXCEPTION_RAISE(
                        "NtupleManager", 
                        "A tree with name " + tname + " has already been created." 
                    ); 
              
                // Initialize the variable to the minimum limit of that type. 
                variables_[vname] = T(std::numeric_limits<T>::min()); 

                // Get the type name
                std::string typeName = typeid(std::get<T>(variables_[vname])).name();

                //  The type name in C++ for basic types are lowercase:
                //      s, i, f, d, l
                //  while in ROOT it is uppercase:
                //      S, I, F, D, L
                //  so we need the call to toupper
                for ( char& c : typeName ) c = toupper(c);

                // Add the variable to the ROOT tree
                trees_[tname]->Branch(vname.c_str(), &std::get<T>(variables_[vname]), 
                        (vname + "/" + typeName).c_str()); 
            } 

            /**
             *  Set the value of the variable named 'vname'.  If the variable
             *  value is not set, the default value will be used when filling
             *  the tree.  If the requested variable has not been created, 
             *  a warning will be printed.  This allows a user to choose to 
             *  make a subset of an ntuple by simply not adding the variable 
             *  to the tree.
             *
             *  @param vname Name of the variable 
             *  @param value The value of the variable 
             */
            template <typename T>
            void setVar(const std::string& vname, const T value) {
                
                // Check if the variable already exists in the map.  If it
                // doesn't, warn the user and don't try to set the variable 
                // value. 
                if (variables_.count(vname) == 0) { 
                    ldmx_log(warn) << "The variable " 
                                   << vname << " does not exist in the tree.";
                    return; 
                }
                
                // Set the value of the variable
                variables_[vname] = value;      
            }

            // Fill all of the ROOT trees.
            void fill();

            /// Reset all of the variables to their limits.
            void clear(); 

            /// Hide Copy Constructor
            NtupleManager(const NtupleManager&) = delete;

            /// Hide Assignment Operator
            void operator=(const NtupleManager&) = delete;

        private: 

            /// The variables these trees can hold
            typedef std::variant<
                short,
                int,
                float,
                double,
                long
                > variable_type;

            /// Container for ROOT trees 
            std::unordered_map< std::string, TTree* > trees_; 

            /// Container for tree leaves 
            std::unordered_map< std::string, variable_type > variables_; 

            /// Private constructor to prevent instantiation 
            NtupleManager(); 

            /// Enable logging for this singleton
            enableLogging("NtupleManager")

    };  // NtupleManager 

} // ldmx

#endif // _NTUPLE_MANAGER_H_
