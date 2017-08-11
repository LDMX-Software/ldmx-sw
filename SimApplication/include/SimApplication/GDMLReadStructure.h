/*
 * GDMLReadStructure.h
 * @brief
 * @author JeremyMcCormick, SLAC
 */

#ifndef SIMAPPLICATION_GDMLREADSTRUCTURE_H_
#define SIMAPPLICATION_GDMLREADSTRUCTURE_H_

#include "G4GDMLReadStructure.hh"

#include <xercesc/dom/DOMElement.hpp>

#include <algorithm>

namespace ldmx {

    class GDMLReadStructure : public G4GDMLReadStructure {

        public:

            GDMLReadStructure() {
            }

            virtual ~GDMLReadStructure() {
            }

            void removeModule(std::string moduleName) {
                removeModules_.push_back(moduleName);
            }

            virtual void StructureRead(const xercesc::DOMElement* const structure) {

                if (removeModules_.size()) {
                    removeModules(structure, removeModules_);
                }

                G4GDMLReadStructure::StructureRead(structure);
            }

            void removeModules(const xercesc::DOMElement* const structure, const std::vector<std::string> moduleNames) {
                static XMLCh* name_attr = xercesc::XMLString::transcode("name");
                for (xercesc::DOMNode* iterStructure = structure->getFirstChild();
                        iterStructure != 0; iterStructure = iterStructure->getNextSibling()) {
                    const xercesc::DOMElement* const topVol = dynamic_cast<xercesc::DOMElement*>(iterStructure);
                    if (topVol != 0) {
                        char* volName = xercesc::XMLString::transcode(topVol->getAttribute(name_attr));
                        if (std::string(volName) == "World") {
                            for (xercesc::DOMNode* iterWorld = topVol->getFirstChild();
                                    iterWorld != 0; iterWorld = iterWorld->getNextSibling()) {
                                const xercesc::DOMElement* const physvol = dynamic_cast<xercesc::DOMElement*>(iterWorld);
                                if (physvol != 0) {
                                    for (xercesc::DOMNode* iterVol = physvol->getFirstChild();
                                            iterVol != 0; iterVol = iterVol->getNextSibling()) {
                                        const xercesc::DOMElement* const elem = dynamic_cast<xercesc::DOMElement*>(iterVol);
                                        if (elem != 0) {
                                            char* tagName = xercesc::XMLString::transcode(elem->getTagName());
                                            if (std::string(tagName) == "file") {
                                                char* filename = xercesc::XMLString::transcode(elem->getAttribute(name_attr));
                                                if (std::find(moduleNames.begin(), moduleNames.end(), filename) != moduleNames.end()) {
                                                    try {
                                                        iterVol->getParentNode()->removeChild(iterVol);
                                                    } catch (const xercesc::DOMException& err) {
                                                        throw new std::runtime_error(xercesc::XMLString::transcode(err.msg));
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

        private:

            std::vector<std::string> removeModules_;
    };
}

#endif /* SIMAPPLICATION_GDMLREADSTRUCTURE_H_ */


