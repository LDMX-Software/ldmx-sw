#include <regex.h>
#include <sys/types.h>

#include "DetDescr/HcalGeometry.h"
#include "Framework/ConditionsObjectProvider.h"
#include "Framework/EventHeader.h"
#include "Framework/RunHeader.h"

/**
 * @file HcalGeometryProvider.cxx
 * @brief Class that creates HcalGeometry object based on python specification
 * @author Jeremiah Mans, UMN
 * @author Cristina Suarez, Fermi National Accelerator Laboratory
 */

namespace hcal {

class HcalGeometryProvider : public framework::ConditionsObjectProvider {
 public:
  /**
   * Class constructor
   * @param parameters -- uses the "hcal_geometry" section to configure the
   * HcalGeometry
   */
  HcalGeometryProvider(const std::string& name, const std::string& tagname,
                       const framework::config::Parameters& parameters,
                       framework::Process& process);

  /** Destructor */
  virtual ~HcalGeometryProvider();

  /**
   * Provides access to the HcalGeometry
   * @note Currently, these are assumed to be valid for all time, but this
   * behavior could be changed.  Users should not cache the pointer between
   * events
   */
  virtual std::pair<const framework::ConditionsObject*,
                    framework::ConditionsIOV>
  getCondition(const ldmx::EventHeader& context);

  /**
   * Take no action on release, as the object is permanently owned by the
   * Provider
   */
  virtual void releaseConditionsObject(const framework::ConditionsObject* co) {}

  virtual void onNewRun(ldmx::RunHeader& rh) {
    if (detector_geometry_.empty())
      detector_geometry_ = rh.getDetectorName();
    else if (hcal_geometry_ != nullptr &&
             detector_geometry_ != rh.getDetectorName()) {
      EXCEPTION_RAISE(
          "GeometryException",
          "Attempting to run a single job with multiple geometries " +
              detector_geometry_ + " and '" + rh.getDetectorName() + "'");
    }
    // make sure detector name has been set
    if (detector_geometry_.empty())
      EXCEPTION_RAISE("GeometryException",
                      "HcalGeometryProvider unable to get the name of the "
                      "detector from the RunHeader.");
  }

 private:
  /** Handle to the parameters, needed for future use during get condition */
  framework::config::Parameters params_;
  /** Geometry as last used */
  std::string detector_geometry_;
  ldmx::HcalGeometry* hcal_geometry_;
};

HcalGeometryProvider::HcalGeometryProvider(
    const std::string& name, const std::string& tagname,
    const framework::config::Parameters& parameters,
    framework::Process& process)
    : framework::
          ConditionsObjectProvider{ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME,
                                   tagname, parameters, process},
      params_{parameters} {
  hcal_geometry_ = 0;
}

HcalGeometryProvider::~HcalGeometryProvider() {
  if (hcal_geometry_) delete hcal_geometry_;
  hcal_geometry_ = 0;
}

std::pair<const framework::ConditionsObject*, framework::ConditionsIOV>
HcalGeometryProvider::getCondition(const ldmx::EventHeader& context) {
  static const std::string keyname("detectors_valid");

  if (!hcal_geometry_) {
    framework::config::Parameters phex =
        (params_.exists("hcal_geometry"))
            ? (params_.get<framework::config::Parameters>("hcal_geometry"))
            : (params_);

    // search through the subtrees
    for (auto key : phex.keys()) {
      framework::config::Parameters pver =
          phex.get<framework::config::Parameters>(key);

      if (!pver.exists(keyname)) {
        ldmx_log(warn) << "No parameter " << keyname << " found in " << key;
        // log strange situation and continue
        continue;
      }

      std::vector<std::string> dets_valid =
          pver.get<std::vector<std::string> >(keyname);
      for (auto detregex : dets_valid) {
        std::string regex(detregex);
        if (regex.empty()) continue;  // no empty regex allowed
        if (regex[0] != '^') regex.insert(0, 1, '^');
        if (regex.back() != '$') regex += '$';
        regex_t reg;

        int rv =
            regcomp(&reg, regex.c_str(), REG_EXTENDED | REG_ICASE | REG_NOSUB);
        if (rv) {
          char err[1024];
          regerror(rv, &reg, err, 1024);
          EXCEPTION_RAISE(
              "GeometryException",
              "Invalid detector regular expression : '" + regex + "' " + err);
        }
        int nmatch = regexec(&reg, detector_geometry_.c_str(), 0, 0, 0);
        regfree(&reg);
        if (!nmatch) {
          hcal_geometry_ = new ldmx::HcalGeometry(pver);
          break;
        }
      }
      if (hcal_geometry_) break;
    }
    if (!hcal_geometry_) {
      EXCEPTION_RAISE("GeometryException", "Unable to create HcalGeometry");
    }
  }

  return std::make_pair(
      hcal_geometry_,
      framework::ConditionsIOV(context.getRun(), context.getRun(), true, true));
}

}  // namespace hcal

DECLARE_CONDITIONS_PROVIDER(hcal::HcalGeometryProvider);
