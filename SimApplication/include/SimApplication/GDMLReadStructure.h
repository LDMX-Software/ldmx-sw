#ifndef SIMAPPLICATION_GDMLREADSTRUCTURE_H_
#define SIMAPPLICATION_GDMLREADSTRUCTURE_H_

#include "G4GDMLReadStructure.hh"

#include <xercesc/dom/DOMElement.hpp>

#include <algorithm>
#include <cstdlib>
#include <fstream>

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

            void StructureRead(const xercesc::DOMElement* const structure) {

                if (removeModules_.size()) {
                    removeModules(const_cast<xercesc::DOMElement*>(structure), removeModules_);
                }

                std::string detectorName = readDetectorName(structure->getParentNode());
                std::string detectorDir = std::getenv("LDMXSW_DIR") + std::string("/data/detectors/") + detectorName;
                resolveModules(structure, detectorDir);

                G4GDMLReadStructure::StructureRead(structure);
            }

        private:

            void removeModules(xercesc::DOMElement* structure, const std::vector<std::string> moduleNames) {
                static XMLCh* name_attr = xercesc::XMLString::transcode("name");
                for (xercesc::DOMNode* iterStructure = structure->getFirstChild();
                        iterStructure != 0; iterStructure = iterStructure->getNextSibling()) {
                    xercesc::DOMElement* topVol = dynamic_cast<xercesc::DOMElement*>(iterStructure);
                    if (topVol != 0) {
                        char* volName = xercesc::XMLString::transcode(topVol->getAttribute(name_attr));
                        if (std::string(volName) == "World") {
                            for (xercesc::DOMNode* iterWorld = topVol->getFirstChild();
                                    iterWorld != 0; iterWorld = iterWorld->getNextSibling()) {
                                xercesc::DOMElement* physvol = dynamic_cast<xercesc::DOMElement*>(iterWorld);
                                if (physvol != 0) {
                                    for (xercesc::DOMNode* iterVol = physvol->getFirstChild();
                                            iterVol != 0; iterVol = iterVol->getNextSibling()) {
                                        xercesc::DOMElement* elem = dynamic_cast<xercesc::DOMElement*>(iterVol);
                                        if (elem != 0) {
                                            char* tagName = xercesc::XMLString::transcode(elem->getTagName());
                                            if (std::string(tagName) == "file") {
                                                char* filename = xercesc::XMLString::transcode(elem->getAttribute(name_attr));
                                                if (std::find(moduleNames.begin(), moduleNames.end(), filename) != moduleNames.end()) {
                                                    try {
                                                        iterVol->getParentNode()->removeChild(iterVol);
                                                        std::cout << "GDMLReadStructure: Module '" << filename << "' was removed!" << std::endl;
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

            void resolveModules(const xercesc::DOMElement* const structure, std::string detectorDir) {
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
                                            static const XMLCh* tagName = elem->getTagName();
                                            const XMLCh* file_attr = xercesc::XMLString::transcode("file");
                                            if (!xercesc::XMLString::compareString(tagName, file_attr)) {
                                                std::string filename = xercesc::XMLString::transcode(elem->getAttribute(name_attr));
                                                if (filename.size()) {
                                                    if (!std::ifstream(filename).good()) {
                                                        std::string newFilename = detectorDir + "/" + filename;
                                                        if (std::ifstream(newFilename).good()) {
                                                            std::cout << "GDMLReadStructure: Module file '" << filename 
                                                                    << "' resolved to '" << newFilename << "'." << std::endl;
                                                            const XMLCh* file_val = xercesc::XMLString::transcode(newFilename.c_str()); 
                                                            const_cast<xercesc::DOMElement*>(elem)->setAttribute(xercesc::XMLString::transcode("name"), file_val);
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
            }

            std::string readDetectorName(const xercesc::DOMNode* const gdml) {
                std::string detectorName("");
                for (xercesc::DOMNode* iter = gdml->getFirstChild();
                        iter != 0; iter = iter->getNextSibling()) {
                    const xercesc::DOMElement* const elem = dynamic_cast<xercesc::DOMElement*>(iter);
                    static std::string userinfo_tagname = "userinfo";
                    if (elem != 0 && xercesc::XMLString::transcode(elem->getTagName()) == userinfo_tagname) {
                        for (xercesc::DOMNode* infoIter = elem->getFirstChild(); infoIter != 0; 
                                infoIter = infoIter->getNextSibling()) {
                            const xercesc::DOMElement* const userinfo = dynamic_cast<xercesc::DOMElement*>(infoIter);
                            static std::string detVersion = "DetectorVersion";
                            static XMLCh* auxtype_attrib = xercesc::XMLString::transcode("auxtype");
                            if (userinfo != 0 && xercesc::XMLString::transcode(userinfo->getAttribute(auxtype_attrib)) == detVersion) {
                                for (xercesc::DOMNode* auxIter = infoIter->getFirstChild(); auxIter != 0;
                                        auxIter = auxIter->getNextSibling()) {
                                    const xercesc::DOMElement* const aux = dynamic_cast<xercesc::DOMElement*>(auxIter);
                                    static std::string detname = "DetectorName";
                                    static XMLCh* auxvalue_attrib = xercesc::XMLString::transcode("auxvalue");
                                    if (aux != 0 && xercesc::XMLString::transcode(aux->getAttribute(auxtype_attrib)) == detname) {
                                         detectorName = xercesc::XMLString::transcode(aux->getAttribute(auxvalue_attrib));
                                         break;
                                    }
                                } 
                            }
                        }
                    }
                }
                return detectorName;
            }

            // TODO: entity resolving
            // - for mag field use $LDMXSW_DIR/data/fieldmap/$FILENAME 

        private:

            std::vector<std::string> removeModules_;
    };
}

#endif /* SIMAPPLICATION_GDMLREADSTRUCTURE_H_ */


