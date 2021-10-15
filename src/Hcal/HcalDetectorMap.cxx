#include "Hcal/HcalDetectorMap.h"

#include <sstream>

#include "Framework/ConditionsObjectProvider.h"
#include "Framework/EventHeader.h"

namespace ecal {

class HcalDetectorMapLoader : public framework::ConditionsObjectProvider {
 public:
  HcalDetectorMapLoader(const std::string& name, const std::string& tagname,
                        const framework::config::Parameters& parameters,
                        framework::Process& process)
      : ConditionsObjectProvider(HcalDetectorMap::CONDITIONS_OBJECT_NAME,
                                 tagname, parameters, process),
        the_map_{nullptr} {
    want_d2e_ = parameters.getParameter<bool>("want_d2e");
    cell_map_ = parameters.getParameter<std::string>("cell_map");
    motherboard_map_ = parameters.getParameter<std::string>("motherboard_map");
    layer_map_ = parameters.getParameter<std::string>("layer_map");
  }

  virtual std::pair<const framework::ConditionsObject*,
                    framework::ConditionsIOV>
  getCondition(const ldmx::EventHeader& context) {
    if (!the_map_) {
      the_map_ = new HcalDetectorMap(cell_map_, motherboard_map_, layer_map_, want_d2e_);
    }

    return std::make_pair(
        the_map_, framework::ConditionsIOV(context.getRun(), context.getRun(),
                                          true, true));
  }

  /**
   * Take no action on release, as the object is permanently owned by the
   * Provider
   */
  virtual void releaseConditionsObject(const framework::ConditionsObject* co) {}

 private:
  HcalDetectorMap* the_map_;
  std::string cell_map_;
  std::string motherboard_map_;
  std::string layer_map_;
  bool want_d2e_;
};

HcalDetectorMap::HcalDetectorMap(const std::string& cell_map, const std::string& motherboard_map, const std::string& layer_map, bool want_d2e)
    : framework::ConditionsObject(CONDITIONS_OBJECT_NAME),
      ldmx::ElectronicsMap<ldmx::HcalElectronicsID, ldmx::HcalDigiID>(want_d2e) {

      conditions::StreamCSVLoader scell(cell_map);
      this->loadCellMap(scell);
      conditions::StreamCSVLoader smb(motherboard_map);
      this->loadMotherboardMap(smb);
      conditions::StreamCSVLoader slayer(layer_map);
      this->loadLayerMap(slayer);
      this->buildElectronicsMap();
}

void HcalDetectorMap::loadCellMap(conditions::GeneralCSVLoader& loader) {
  cells_.clear();
  while (loader.nextRow()) {
    CellInformation ci;
    ci.module_cellid = loader.getInteger("CELLID");
    ci.rocid = loader.getInteger("ROCID");
    ci.roc_elink_number = loader.getInteger("ROC_ELINK_NUMBER");
    ci.roc_elink_channel = loader.getInteger("ROC_ELINK_CHANNEL");
    cells_.push_back(ci);
  }
}

void HcalDetectorMap::loadMotherboardMap(conditions::GeneralCSVLoader& loader) {
  elinks_.clear();
  while (loader.nextRow()) {
    MotherboardLinksInformation mli;
    mli.motherboard_type = loader.getInteger("MOTHERBOARD_TYPE");
    mli.module = loader.getInteger("MODULE");
    mli.rocid = loader.getInteger("ROCID");
    mli.roc_elink_number = loader.getInteger("ROC_ELINK_NUMBER");
    mli.polarfire_elink = loader.getInteger("POLARFIRE_ELINK");
    elinks_.push_back(mli);
  }
}

void HcalDetectorMap::loadLayerMap(conditions::GeneralCSVLoader& loader) {
  layers_.clear();
  while (loader.nextRow()) {
    MotherboardsPerLayer mpl;
    mpl.motherboard_type = loader.getInteger("MOTHERBOARD_TYPE");
    mpl.layer = loader.getInteger("LAYER");
    mpl.daq_opticallink = loader.getInteger("OLINK");
    layers_.push_back(mpl);
  }
}

void HcalDetectorMap::buildElectronicsMap() {
  this->clear();  // empty the electronics map
  // loop over optical links
  for (auto olink : layers_) {
    for (auto elink : elinks_) {
      // select only matching motherboard types
      if (elink.motherboard_type != olink.motherboard_type) continue;

      for (auto cell : cells_) {
        // select only cells which are associated with the appropriate elink
        if (elink.rocid != cell.rocid ||
            elink.roc_elink_number != cell.roc_elink_number)
          continue;

        // now, we have only cells which are relevant
        ldmx::HcalDigiID precisionId(olink.layer, elink.module, cell.module_cellid);
        ldmx::HcalElectronicsID elecId(olink.daq_opticallink,
                                       elink.polarfire_elink,
                                       cell.roc_elink_channel);

        if (this->exists(elecId)) {
          std::stringstream ss;
          ss << "Two different mappings for electronics channel " << elecId;
          EXCEPTION_RAISE("DuplicateMapping", ss.str());
        }
        this->addEntry(elecId, precisionId);
      }
    }
  }
}

}  // namespace ecal
DECLARE_CONDITIONS_PROVIDER_NS(ecal, HcalDetectorMapLoader);
