
#ifndef _NTUPLE_MANAGER_H_
#define _NTUPLE_MANAGER_H_

#include <limits> 
#include <iostream> 
#include <map> 
#include <string>
#include <typeinfo>
#include <unordered_map>

#include "boost/assign/list_of.hpp"
#include "boost/variant.hpp"
#include "boost/unordered_map.hpp"

#include "TTree.h" 

typedef boost::variant< short, int, float, double, long > vtype; 

namespace ldmx {

    struct cmpStr {
        bool operator()(char const *a, char const *b) const { return std::strcmp(a, b) < 0; }
    };

    /**
     * @class NtupleManager
     * @brief Singleton class used to manage the creation and pooling of 
     *        ntuples.
     */
    class NtupleManager { 

        public:

            static NtupleManager* getInstance(); 

            /**
             * Create a ROOT tree to hold the ntuple variables (ROOT leaves). 
             *
             * @param name Name of the tree.
             */
            void create(const std::string& tname);

            /**
             *
             */
            template <typename T>
            void addVar(const std::string& tname, const std::string& vname) {
                
                // Check if the variable exists in the map. If it does, throw 
                // an exception.
                if (variables_.count(vname) != 0) { 
                    return;
                } 
               
                // Initialize the variable to the minimum limit of that type. 
                variables_[vname] = T(std::numeric_limits<T>::min()); 

                // Get the type name
                auto typeName = typeid(boost::get<T>(variables_[vname])).name(); 

                // Add the variable to the ROOT tree
                trees_[tname]->Branch(vname.c_str(), &boost::get<T>(variables_[vname]), 
                        (vname + "/" + rtype_.at(typeName)).c_str()); 
            } 

            /**
             *
             */
            template <typename T>
            void setVar(const std::string& vname, T value) {
                
                // Check if the variable already exists in the map.  If so, 
                // throw an exception. 
                if (variables_.count(vname) == 0) { 
                    // throw an exception
                }
                
                // Set the value of the variable
                variables_[vname] = value;      
            }

            /**
             *
             */
            void fill();

            /**
             *
             */
            void clear(); 

        private: 

            /// NtupleManager instance
            static NtupleManager* instance_;  

            /// Container for ROOT trees 
            std::unordered_map< std::string, TTree* > trees_; 

            /// Container for tree leaves 
            std::unordered_map< std::string, vtype > variables_; 

            /// Map from variable type to string representation used by ROOT.
            const std::map< const char*, std::string, cmpStr> rtype_ = 
            { 
                {"s", "S"}, 
                {"i", "I"},
                {"f", "F"},
                {"d", "D"},
                {"l", "L"},
            };

            /// Private constructor to prevent instantiation 
            NtupleManager(); 

    };  // NtupleManager 

} // ldmx

#endif // _NTUPLE_MANAGER_H_
