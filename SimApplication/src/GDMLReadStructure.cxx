#include "SimApplication/GDMLReadStructure.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>

namespace ldmx {

    void GDMLReadStructure::removeModule(std::string moduleName) {
        removeModules_.push_back(moduleName);
    }

    void GDMLReadStructure::StructureRead(const xercesc::DOMElement* const structure) {

        // Remove modules that should not be loaded.
        if (removeModules_.size()) {
            removeModules(const_cast<xercesc::DOMElement*>(structure), removeModules_);
        }

        // Resolve module file relative paths using the detector data dir.
        std::string detectorName = readDetectorName(structure->getParentNode());
        std::string detectorDir = std::getenv("LDMXSW_DIR") + std::string("/data/detectors/") + detectorName;
        resolveModules(structure, detectorDir);
 
        // Resolve field map relative file path using the fieldmap data dir.
        std::string fieldMapDir = std::getenv("LDMXSW_DIR") + std::string("/data/fieldmap");
        resolveFieldMap(structure->getParentNode(), fieldMapDir);

        // Activate the parser to read the modified document.
        G4GDMLReadStructure::StructureRead(structure);
    }

    void GDMLReadStructure::removeModules(xercesc::DOMElement* structure, const std::vector<std::string> moduleNames) {
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

    void GDMLReadStructure::resolveModules(const xercesc::DOMElement* const structure, std::string detectorDir) {
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

    std::string GDMLReadStructure::readDetectorName(const xercesc::DOMNode* const gdml) {
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

    void GDMLReadStructure::resolveFieldMap(const xercesc::DOMNode* const gdml, const std::string& fieldMapDir) {
        for (xercesc::DOMNode* iter = gdml->getFirstChild(); iter != 0; iter = iter->getNextSibling()) {
            const xercesc::DOMElement* const elem = dynamic_cast<xercesc::DOMElement*>(iter);
            static std::string userinfo_tagname = "userinfo";
            if (elem != 0 && xercesc::XMLString::transcode(elem->getTagName()) == userinfo_tagname) {
                const xercesc::DOMElement* const userinfo = dynamic_cast<xercesc::DOMElement*>(iter);  
                for (xercesc::DOMNode* infoIter = userinfo->getFirstChild(); infoIter != 0; infoIter = infoIter->getNextSibling()) {
                    const xercesc::DOMElement* const aux1 = dynamic_cast<xercesc::DOMElement*>(infoIter);
                    static XMLCh* auxtype_attrib = xercesc::XMLString::transcode("auxtype");
                    if (aux1 && !xercesc::XMLString::compareString(aux1->getAttribute(auxtype_attrib), xercesc::XMLString::transcode("MagneticField"))) {
                        for (xercesc::DOMNode* auxIter = aux1->getFirstChild(); auxIter != 0; auxIter = auxIter->getNextSibling()) {
                            const xercesc::DOMElement* const aux2 = dynamic_cast<xercesc::DOMElement*>(auxIter);
                            static XMLCh* auxvalue_attrib = xercesc::XMLString::transcode("auxvalue");
                            if (aux2 != 0 && !xercesc::XMLString::compareString(aux2->getAttribute(auxtype_attrib), xercesc::XMLString::transcode("File"))) {
                                std::string filename = xercesc::XMLString::transcode(aux2->getAttribute(auxvalue_attrib));
                                if (!std::fstream(filename).good()) {
                                    std::string newFilename = fieldMapDir + "/" + filename;
                                    const_cast<xercesc::DOMElement*>(aux2)->setAttribute(auxvalue_attrib, xercesc::XMLString::transcode(newFilename.c_str()));
                                    std::cout << "GDMLReadStructure: Mag field file '" << filename << "' resolved to '" << newFilename << "'." << std::endl;
                                }
                            }
                        } 
                    }
                }
            }
        }
    }
}
