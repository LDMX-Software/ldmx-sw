#include "Hcal/HcalDetectorMap.h"

#include <sstream>

#include "Framework/ConditionsObjectProvider.h"
#include "Framework/EventHeader.h"

namespace hcal {

class HcalDetectorMapLoader : public framework::ConditionsObjectProvider {
 public:
  HcalDetectorMapLoader(const std::string& name, const std::string& tagname,
                        const framework::config::Parameters& parameters,
                        framework::Process& process)
      : ConditionsObjectProvider(HcalDetectorMap::CONDITIONS_OBJECT_NAME,
                                 tagname, parameters, process),
        the_map_{nullptr} {
    want_d2e_ = parameters.getParameter<bool>("want_d2e");
    connections_table_ = parameters.getParameter<std::string>("connections_table");
  }

  virtual std::pair<const framework::ConditionsObject*,
                    framework::ConditionsIOV>
  getCondition(const ldmx::EventHeader& context) {
    if (!the_map_) {
      the_map_ = new HcalDetectorMap(connections_table_, want_d2e_);
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
  std::string connections_table_;
  bool want_d2e_;
};

HcalDetectorMap::HcalDetectorMap(const std::string& connections_table, bool want_d2e)
    : framework::ConditionsObject(CONDITIONS_OBJECT_NAME),
      ldmx::ElectronicsMap<ldmx::HcalElectronicsID, ldmx::HcalDigiID>(want_d2e) {
  
  this->clear();
  conditions::StreamCSVLoader csv(connections_table);
  while (csv.nextRow()) {
    /** Column Names
     * "HGCROC" "Channel" "CMB" "Quadbar" "Bar" "Plane"
     *
     * Each HGCROC has two links coming out of it,
     * one for the upper half of the channels and one for the lower half,
     * this means we need to modify the hgcroc/channel pair to be
     * actual link/channel pairs where the new link has range twice
     * as big as the hgcroc ID but the new channel has a range half
     * as big as the old one.
     */
    int hgcroc = csv.getInteger("HGCROC");
    int chan   = csv.getInteger("Channel");

    // each hgcroc has two links, one for the upper half of channels,
    // one for the lower half
    // for test beam (current conditions table) we only use 32 out of 38
    int link = 2*(hgcroc-1); // hgcroc count starts from 0
    if (chan >= 32) {
      link += 1;
      chan -= 32;
    }
    // one quadbar groups 4 strips and each quadbar is connected to 2 CMBs - one in each end
    int end = csv.getInteger("CMB")%2;
    if(end==0) end=1; // negative end (?)
    else end=0; // positive end (?)
 
    ldmx::HcalDigiID detid(0 /*section - only one section during test beam*/, 
        csv.getInteger("Plane") /*layer*/,
        csv.getInteger("Bar") /*strip*/,
        end /*end??*/);
    ldmx::HcalElectronicsID eleid(
        0 /*fpga - only one FPGA during test beam*/,
        link /*fiber*/,
        chan /*channel*/);
    
    if (this->exists(eleid)) {
      std::stringstream ss;
      ss << "Two different mappings for electronics channel " << eleid;
      EXCEPTION_RAISE("DuplicateMapping", ss.str());
    }
    this->addEntry(eleid, detid);
  }
}

}  // namespace hcal
DECLARE_CONDITIONS_PROVIDER_NS(hcal, HcalDetectorMapLoader);
