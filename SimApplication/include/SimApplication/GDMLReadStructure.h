/*
 * GDMLReadStructure.h
 * @brief
 * @author JeremyMcCormick, SLAC
 */

#ifndef SIMAPPLICATION_GDMLREADSTRUCTURE_H_
#define SIMAPPLICATION_GDMLREADSTRUCTURE_H_

#include "G4GDMLReadStructure.hh"

#include <xercesc/dom/DOMElement.hpp>

namespace ldmx {

    /*
    void G4GDMLRead::Read(const G4String& fileName,
                                G4bool validation,
                                G4bool isModule,
                                G4bool strip)
    */

    class GDMLReadStructure : public G4GDMLReadStructure {

        public:

            GDMLReadStructure() {
            }

            virtual ~GDMLReadStructure() {
            }

            virtual void StructureRead(const xercesc::DOMElement* const structure) {

                removeModule(structure);

                // Now call super-class method.
                G4GDMLReadStructure::StructureRead(structure);
            }

            void removeModule(const xercesc::DOMElement* const structure, std::string moduleName = "magnet.gdml") {

                static XMLCh* name_attr = xercesc::XMLString::transcode("name");

                //std::vector<xercesc::DOMNode*> removeNodes;
                xercesc::DOMNode* worldNode;

                // loop over structure
                for (xercesc::DOMNode* iterStructure = structure->getFirstChild();
                        iterStructure != 0; iterStructure = iterStructure->getNextSibling()) {
                    const xercesc::DOMElement* const topVol = dynamic_cast<xercesc::DOMElement*>(iterStructure);
                    if (topVol != 0) {
                        char* volName = xercesc::XMLString::transcode(topVol->getAttribute(name_attr));

                        std::cout << "GDMLReadStructure: read vol <" << volName << ">" << std::endl;

                        if (std::string(volName) == "World") {

                            std::cout << "GDMLReadStructure: found World vol" << std::endl;

                            //worldNode = iterStructure;

                            // loop over children of world vol
                            for (xercesc::DOMNode* iterWorld = topVol->getFirstChild();
                                    iterWorld != 0; iterWorld = iterWorld->getNextSibling()) {

                                const xercesc::DOMElement* const physvol = dynamic_cast<xercesc::DOMElement*>(iterWorld);

                                if (physvol != 0) {
                                    std::cout << "GDMLReadStructure: read physvol under World" << std::endl;

                                    // loop over physvol child elements
                                    for (xercesc::DOMNode* iterVol = physvol->getFirstChild();
                                            iterVol != 0; iterVol = iterVol->getNextSibling()) {

                                        const xercesc::DOMElement* const elem = dynamic_cast<xercesc::DOMElement*>(iterVol);

                                        if (elem != 0) {
                                            std::cout << "GDMLReadStructure: got child elem <" << xercesc::XMLString::transcode(elem->getTagName()) << ">" << std::endl;

                                            char* tagName = xercesc::XMLString::transcode(elem->getTagName());
                                            if (std::string(tagName) == "file") {
                                                char* filename = xercesc::XMLString::transcode(elem->getAttribute(name_attr));
                                                std::cout << "GDMLReadStructure: file <" << filename << ">" << std::endl;

                                                // FIXME: make this string parameter in the GDML messenger
                                                static std::string removeMe(moduleName);
                                                if (std::string(filename) == removeMe) {
                                                    std::cout << "GDMLReadStructure: module <" << filename << "> is being removed" << std::endl;
                                                    //removeNodes.push_back(iterVol)
                                                    try {
                                                        iterVol->getParentNode()->removeChild(iterVol);
                                                    } catch (const xercesc::DOMException& err) {
                                                        std::cerr << "DOMException: " << xercesc::XMLString::transcode(err.msg) << std::endl;
                                                        throw new std::runtime_error(xercesc::XMLString::transcode(err.msg));
                                                    }
                                                    //worldNode->removeChild(iterVol);
                                                    //return;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    /*
                    if (worldNode && removeNodes.size() != 0) {
                        for (std::vector<xercesc::DOMNode*>::iterator it = removeNodes.begin();
                                it != removeNodes.end(); it++) {
                            xercesc::DOMNode* node = (*it);
                            try {
                                worldNode->removeChild(node);
                            } catch (const xercesc::DOMException& err) {
                                std::cerr << "DOMException: " << xercesc::XMLString::transcode(err.msg) << std::endl;
                                throw new std::runtime_error(xercesc::XMLString::transcode(err.msg));
                            }
                        }
                    }
                    */
                }
            }

    };
}

#endif /* SIMAPPLICATION_GDMLREADSTRUCTURE_H_ */


