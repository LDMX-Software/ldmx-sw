/**
 * @file EcalDetectorMap.h
 * @brief Class which contains logic for how the detector items connect to and
 * relate with the reconstruction cells
 * @author Jeremiah Mans, University of Minnesota
 */

#ifndef ECAL_ECALDETECTORMAP_H_
#define ECAL_ECALDETECTORMAP_H_

#include <vector>

#include "Conditions/GeneralCSVLoader.h"
#include "DetDescr/EcalElectronicsID.h"
#include "DetDescr/EcalID.h"
#include "Framework/ConditionsObject.h"
#include "Tools/ElectronicsMap.h"

namespace ecal {

/**
 * \brief Class which provides various information about how the parts of the
 * Ecal connect to each other.
 *
 * The class is loaded from three tables, currently in the form of CSV objects,
 * using the EcalDetectorMapLoader declared and defined in the source.
 *
 * CellMap gives the information for a single Ecal module and has the following
 * columns CELLID -- EcalDetectorID cell id ROCID -- which ROC on the module
 *   ROC_ELINK_NUMBER -- which elink on the ROC (0/1)
 *   ROC_ELINK_CHANNEL -- which channel on the elink (0-35)
 *
 * MotherboardMap gives information about how the modules plug into a given type
 * of motherboard ROCID -- which ROC on the module ROC_ELINK_NUMBER -- which
 * elink on the ROC (0/1) MODULE -- which module on the layer POLARFIRE_ELINK --
 * elink input number on the Polarfire MOTHERBOARD_TYPE -- what type of
 * motherboard is this
 *
 * LayerMap gives information about how the motherboards are used on the various
 * layers LAYER -- layer number MOTHERBOARD_TYPE OLINK -- DAQ optical link
 * number
 *
 * We inherit from two classes: (1) the ConditionsObject because we are a
 * conditions object and (2) from the ElectronicsMap template because we
 * are a electronics map.
 */
class EcalDetectorMap
    : public framework::ConditionsObject,
      public ldmx::ElectronicsMap<ldmx::EcalElectronicsID, ldmx::EcalID> {
 public:
  /// The name of the EID <-> DetID map for the ECal
  static constexpr const char* CONDITIONS_OBJECT_NAME{"EcalDetectorMap"};

  /**
   * Default constructor which builds the necessary maps.
   *
   * @param[in] want_d2e true if we want to build a reverse mapping
   * Building a reverse (detector->electronics) map takes extra time
   * and memory so it should be off by default.
   */
  EcalDetectorMap(bool want_d2e);

  /// Provider which loads the map
  friend class EcalDetectorMapLoader;

 private:
  /// import cell map from the provided CSV loader
  void loadCellMap(conditions::GeneralCSVLoader& loader);
  /// import motherboard map from the provided CSV loader
  void loadMotherboardMap(conditions::GeneralCSVLoader& loader);
  /// import layer map from the provided CSV loader
  void loadLayerMap(conditions::GeneralCSVLoader& loader);

  /// build the electronics map from loaded maps
  void buildElectronicsMap();

  /// the full electronics map
  ldmx::ElectronicsMap<ldmx::EcalElectronicsID, ldmx::EcalID> emap_;

  /**
   * Table of per-module cell information
   */
  struct CellInformation {
    /// precision information
    /** cellid */
    int module_cellid;
    /** hgcroc id on the module (which of the six) */
    int rocid;
    /** roc elink number */
    int roc_elink_number;
    /** roc elink channel */
    int roc_elink_channel;
  };
  std::vector<CellInformation> cells_;

  /**
   * Table of per-motherboard connections information
   */
  struct MotherboardLinksInformation {
    /** motherboard type */
    int motherboard_type;
    /** elink id */
    int polarfire_elink;
    /** module number */
    int module;
    /** hgcroc id on the module (which of the six) */
    int rocid;
    /** hgcroc link id on the module (which of the two) */
    int roc_elink_number;
  };
  std::vector<MotherboardLinksInformation> elinks_;

  /**
   * Table of per-layer motherboard layouts
   */
  struct MotherboardsPerLayer {
    /** layer number */
    int layer;
    /** mother board type */
    int motherboard_type;
    /** mother global optical link number */
    int daq_opticallink;
  };
  std::vector<MotherboardsPerLayer> layers_;
};

}  // namespace ecal

#endif  // ECAL_ECALDETECTORMAP_H_

