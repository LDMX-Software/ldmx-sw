/**
 * @file GDMLReadStructure.h 
 * @brief Class implementing custom behavior when reading GDML files
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */
#ifndef SIMAPPLICATION_GDMLREADSTRUCTURE_H_
#define SIMAPPLICATION_GDMLREADSTRUCTURE_H_

#include "G4GDMLReadStructure.hh"

#include <xercesc/dom/DOMElement.hpp>

#include <algorithm>
#include <cstdlib>
#include <fstream>

namespace ldmx {

    /**
     * @class GDMLReadStructure
     * @brief Implements custom behavior when reading GDML files
     */
    class GDMLReadStructure : public G4GDMLReadStructure {

        public:

            GDMLReadStructure() {
            }

            virtual ~GDMLReadStructure() {
            }

            /**
             * Remove a GDML module element before the structure is processed.
             */
            void removeModule(std::string moduleName);

            /**
             * Override method for reading the GDML structure to implement custom behavior.
             */
            void StructureRead(const xercesc::DOMElement* const structure);

        private:

            /**
             * Remove modules from the GDML structure before it is processed.
             */ 
            void removeModules(xercesc::DOMElement* structure, const std::vector<std::string> moduleNames);

            /**
             * Resolve GDML module files to the detector data directory if they are not found in the current
             * dir.
             */  
            void resolveModules(const xercesc::DOMElement* const structure, std::string detectorDir);

            /**
             * Read the detector name from the custom header information in a GDML file.
             */  
            std::string readDetectorName(const xercesc::DOMNode* const gdml);

            /**
             * Resolve field map files to the fieldmap data dir if it is not found in the current dir.
             */  
            void resolveFieldMap(const xercesc::DOMNode* const gdml, const std::string& fieldMapDir);

        private:

            /** List of modules to remove from the GDML file in preprocessing step. */
            std::vector<std::string> removeModules_;
    };
}

#endif /* SIMAPPLICATION_GDMLREADSTRUCTURE_H_ */


