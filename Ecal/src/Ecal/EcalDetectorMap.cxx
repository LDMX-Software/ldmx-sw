#include "Ecal/EcalDetectorMap.h"

#include <sstream>

#include "Framework/ConditionsObjectProvider.h"
#include "Framework/EventHeader.h"

namespace ecal {

class EcalDetectorMapLoader : public framework::ConditionsObjectProvider {
 public:
  EcalDetectorMapLoader(const std::string& name, const std::string& tagname,
                        const framework::config::Parameters& parameters,
                        framework::Process& process)
      : ConditionsObjectProvider(EcalDetectorMap::CONDITIONS_OBJECT_NAME,
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
      the_map_ = new EcalDetectorMap(cell_map_, motherboard_map_, layer_map_,
                                     want_d2e_);
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
  EcalDetectorMap* the_map_;
  std::string cell_map_;
  std::string motherboard_map_;
  std::string layer_map_;
  bool want_d2e_;
};

EcalDetectorMap::EcalDetectorMap(const std::string& cell_map,
                                 const std::string& motherboard_map,
                                 const std::string& layer_map, bool want_d2e)
    : framework::ConditionsObject(CONDITIONS_OBJECT_NAME),
      ldmx::ElectronicsMap<ldmx::EcalElectronicsID, ldmx::EcalID>(want_d2e) {
  conditions::StreamCSVLoader scell(cell_map);
  this->loadCellMap(scell);
  conditions::StreamCSVLoader smb(motherboard_map);
  this->loadMotherboardMap(smb);
  conditions::StreamCSVLoader slayer(layer_map);
  this->loadLayerMap(slayer);
  this->buildElectronicsMap();
}

void EcalDetectorMap::loadCellMap(conditions::GeneralCSVLoader& loader) {
  cells_.clear();
  while (loader.nextRow()) {
    CellInformation ci;
    ci.module_cellid_ = loader.getInteger("CELLID");
    ci.rocid_ = loader.getInteger("ROCID");
    ci.roc_elink_number_ = loader.getInteger("ROC_ELINK_NUMBER");
    ci.roc_elink_channel_ = loader.getInteger("ROC_ELINK_CHANNEL");
    cells_.push_back(ci);
  }
}

void EcalDetectorMap::loadMotherboardMap(conditions::GeneralCSVLoader& loader) {
  elinks_.clear();
  while (loader.nextRow()) {
    MotherboardLinksInformation mli;
    mli.motherboard_type_ = loader.getInteger("MOTHERBOARD_TYPE");
    mli.module_ = loader.getInteger("MODULE");
    mli.rocid_ = loader.getInteger("ROCID");
    mli.roc_elink_number_ = loader.getInteger("ROC_ELINK_NUMBER");
    mli.polarfire_elink_ = loader.getInteger("POLARFIRE_ELINK");
    elinks_.push_back(mli);
  }
}

void EcalDetectorMap::loadLayerMap(conditions::GeneralCSVLoader& loader) {
  layers_.clear();
  while (loader.nextRow()) {
    MotherboardsPerLayer mpl;
    mpl.motherboard_type_ = loader.getInteger("MOTHERBOARD_TYPE");
    mpl.layer_ = loader.getInteger("LAYER");
    mpl.daq_opticallink_ = loader.getInteger("OLINK");
    layers_.push_back(mpl);
  }
}

void EcalDetectorMap::buildElectronicsMap() {
  this->clear();  // empty the electronics map
  // loop over optical links
  for (auto olink : layers_) {
    for (auto elink : elinks_) {
      // select only matching motherboard types
      if (elink.motherboard_type_ != olink.motherboard_type_) continue;

      for (auto cell : cells_) {
        // select only cells which are associated with the appropriate elink
        if (elink.rocid_ != cell.rocid_ ||
            elink.roc_elink_number_ != cell.roc_elink_number_)
          continue;

        // now, we have only cells which are relevant
        ldmx::EcalID precision_id(olink.layer_, elink.module_, cell.module_cellid_);
        ldmx::EcalElectronicsID elec_id(olink.daq_opticallink_,
                                       elink.polarfire_elink_,
                                       cell.roc_elink_channel_);

        if (this->exists(elec_id)) {
          std::stringstream ss;
          ss << "Two different mappings for electronics channel " << elec_id;
          EXCEPTION_RAISE("DuplicateMapping", ss.str());
        }
        this->addEntry(elec_id, precision_id);
      }
    }
  }
}

}  // namespace ecal
DECLARE_CONDITIONS_PROVIDER(ecal::EcalDetectorMapLoader);
