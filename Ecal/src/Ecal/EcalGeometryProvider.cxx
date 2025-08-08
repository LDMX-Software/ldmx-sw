#include <regex.h>
#include <sys/types.h>

#include "DetDescr/EcalGeometry.h"
#include "Framework/ConditionsObjectProvider.h"
#include "Framework/EventHeader.h"
#include "Framework/RunHeader.h"

/**
 * @file EcalGeometryProvider.cxx
 * @brief Class that creates EcalGeometry object based on python specification
 * @author Jeremiah Mans, UMN
 */

namespace ecal {

class EcalGeometryProvider : public framework::ConditionsObjectProvider {
 public:
  /**
   * Class constructor
   * @param parameters -- uses the "EcalGeometry" section to configure the
   * EcalGeometry
   */
  EcalGeometryProvider(const std::string& name, const std::string& tagname,
                       const framework::config::Parameters& parameters,
                       framework::Process& process);

  /** Destructor */
  virtual ~EcalGeometryProvider();

  /**
   * Provides access to the EcalGeometry or EcalTriggerGeometry
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
    else if (ecal_geometry_ != nullptr &&
             detector_geometry_ != rh.getDetectorName()) {
      EXCEPTION_RAISE(
          "GeometryException",
          "Attempting to run a single job with multiple geometries " +
              detector_geometry_ + " and '" + rh.getDetectorName() + "'");
    }
    // make sure detector name has been set
    if (detector_geometry_.empty())
      EXCEPTION_RAISE("GeometryException",
                      "EcalGeometryProvider unable to get the name of the "
                      "detector from the RunHeader.");
  }

 private:
  /** parameters for the various geometry versions we can support */
  std::vector<framework::config::Parameters> geometries_;
  /** Geometry as last used */
  std::string detector_geometry_;
  ldmx::EcalGeometry* ecal_geometry_;
};

EcalGeometryProvider::EcalGeometryProvider(
    const std::string& name, const std::string& tagname,
    const framework::config::Parameters& parameters,
    framework::Process& process)
    : framework::ConditionsObjectProvider{
          ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME, tagname, parameters,
          process} {
  geometries_ =
      parameters.getParameter<std::vector<framework::config::Parameters>>(
          "geometries");
  ecal_geometry_ = 0;
}

EcalGeometryProvider::~EcalGeometryProvider() {
  if (ecal_geometry_) delete ecal_geometry_;
  ecal_geometry_ = 0;
}

std::pair<const framework::ConditionsObject*, framework::ConditionsIOV>
EcalGeometryProvider::getCondition(const ldmx::EventHeader& context) {
  static const std::string keyname("detectors_valid");

  if (!ecal_geometry_) {
    // search through the subtrees
    for (auto pver : geometries_) {
      if (!pver.exists(keyname)) {
        ldmx_log(warn) << "No parameter " << keyname
                       << " found one of the detector vesrsions.";
        // log strange situation and continue
        continue;
      }

      std::vector<std::string> dets_valid =
          pver.getParameter<std::vector<std::string>>(keyname);
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
          ecal_geometry_ = new ldmx::EcalGeometry(pver);
          break;
        }
      }
      if (ecal_geometry_) break;
    }
    if (!ecal_geometry_) {
      EXCEPTION_RAISE("GeometryException", "Unable to create EcalGeometry");
    }
  }

  return std::make_pair(
      ecal_geometry_,
      framework::ConditionsIOV(context.getRun(), context.getRun(), true, true));
}

}  // namespace ecal

DECLARE_CONDITIONS_PROVIDER(ecal::EcalGeometryProvider);
