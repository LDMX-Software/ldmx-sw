#include "Ecal/EcalDetectorMap.h"
#include "Framework/EventHeader.h"
#include "Framework/ConditionsObjectProvider.h"
#include <sstream>

namespace ecal {

class EcalDetectorMapLoader : public framework::ConditionsObjectProvider {
 public:
  EcalDetectorMapLoader(const std::string& name,
                        const std::string& tagname,
                        const framework::config::Parameters& parameters,
                        framework::Process& process)
      : ConditionsObjectProvider(EcalDetectorMap::CONDITIONS_OBJECT_NAME,
                                 tagname, parameters, process), theMap_{0} {
    cellMap_=parameters.getParameter<std::string>("CellMap");
    motherboardMap_=parameters.getParameter<std::string>("MotherboardMap");
    layerMap_=parameters.getParameter<std::string>("LayerMap");
  }

  virtual std::pair<const framework::ConditionsObject*,
                    framework::ConditionsIOV>
  getCondition(const ldmx::EventHeader& context) {

    if (!theMap_) {
      theMap_=new EcalDetectorMap();
      conditions::StreamCSVLoader scell(cellMap_);
      theMap_->loadCellMap(scell);
      conditions::StreamCSVLoader smb(motherboardMap_);
      theMap_->loadMotherboardMap(smb);
      conditions::StreamCSVLoader slayer(layerMap_);
      theMap_->loadLayerMap(slayer);

      theMap_->buildElectronicsMap();
    }
    
    return std::make_pair(theMap_,
                          framework::ConditionsIOV(
                              context.getRun(), context.getRun(), true, true));
  }
  
  /**
   * Take no action on release, as the object is permanently owned by the
   * Provider
   */
  virtual void releaseConditionsObject(const framework::ConditionsObject* co) {}

 private:
  EcalDetectorMap* theMap_;
  std::string cellMap_;
  std::string motherboardMap_;
  std::string layerMap_;
};

EcalDetectorMap::EcalDetectorMap() : framework::ConditionsObject(CONDITIONS_OBJECT_NAME) {
}

void EcalDetectorMap::loadCellMap(conditions::GeneralCSVLoader& loader) {
  cells_.clear();
  while (loader.nextRow()) {
    CellInformation ci;
    ci.module_cellid=loader.getInteger("CELLID");
    ci.rocid=loader.getInteger("ROCID");
    ci.roc_elink_number=loader.getInteger("ROC_ELINK_NUMBER");
    ci.roc_elink_channel=loader.getInteger("ROC_ELINK_CHANNEL");
    cells_.push_back(ci);
  }
}


void EcalDetectorMap::loadMotherboardMap(conditions::GeneralCSVLoader& loader) {
  elinks_.clear();
  while (loader.nextRow()) {
    MotherboardLinksInformation mli;
    mli.motherboard_type=loader.getInteger("MOTHERBOARD_TYPE");
    mli.module=loader.getInteger("MODULE");
    mli.rocid=loader.getInteger("ROCID");
    mli.roc_elink_number=loader.getInteger("ROC_ELINK_NUMBER");
    mli.polarfire_elink=loader.getInteger("POLARFIRE_ELINK");
    elinks_.push_back(mli);
  }
}

void EcalDetectorMap::loadLayerMap(conditions::GeneralCSVLoader& loader) {
  layers_.clear();
  while (loader.nextRow()) {
    MotherboardsPerLayer mpl;
    mpl.motherboard_type=loader.getInteger("MOTHERBOARD_TYPE");
    mpl.layer=loader.getInteger("LAYER");
    mpl.daq_opticallink=loader.getInteger("OLINK");
    layers_.push_back(mpl);
  }
}



void EcalDetectorMap::buildElectronicsMap() {
  using namespace ldmx;
  emap_.clear(); // empty the electronics map
  // loop over optical links
  for (auto olink : layers_) {
    for (auto elink : elinks_) {
      // select only matching motherboard types
      if (elink.motherboard_type!=olink.motherboard_type) continue;

      for (auto cell : cells_) {
        // select only cells which are associated with the appropriate elink
        if (elink.rocid != cell.rocid || elink.roc_elink_number != cell.roc_elink_number) continue;

        // now, we have only cells which are relevant
        EcalID precisionId(olink.layer, elink.module, cell.module_cellid);
        EcalElectronicsID elecId(olink.daq_opticallink, elink.polarfire_elink, cell.roc_elink_channel);

        if (emap_.exists(elecId)) {
          std::stringstream ss;
          ss << "Two different mappings for electronics channel " << elecId;
          EXCEPTION_RAISE(
          "DuplicateMapping",
          ss.str());
        }
        emap_.addEntry(elecId,precisionId);        
      }
    }
  }
}

}
DECLARE_CONDITIONS_PROVIDER_NS(ecal, EcalDetectorMapLoader);
