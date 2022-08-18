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
    if (detectorGeometry_.empty())
      detectorGeometry_ = rh.getDetectorName();
    else if (ecalGeometry_ != nullptr &&
             detectorGeometry_ != rh.getDetectorName()) {
      EXCEPTION_RAISE(
          "GeometryException",
          "Attempting to run a single job with multiple geometries " +
              detectorGeometry_ + " and '" + rh.getDetectorName() + "'");
    }
    // make sure detector name has been set
    if (detectorGeometry_.empty())
      EXCEPTION_RAISE("GeometryException",
                      "EcalGeometryProvider unable to get the name of the "
                      "detector from the RunHeader.");
  }

 private:
  /** Handle to the parameters, needed for future use during get condition */
  framework::config::Parameters params_;
  /** Geometry as last used */
  std::string detectorGeometry_;
  ldmx::EcalGeometry* ecalGeometry_;
};

EcalGeometryProvider::EcalGeometryProvider(
    const std::string& name, const std::string& tagname,
    const framework::config::Parameters& parameters,
    framework::Process& process)
    : framework::
          ConditionsObjectProvider{ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME,
                                   tagname, parameters, process},
      params_{parameters} {
  ecalGeometry_ = 0;
}

EcalGeometryProvider::~EcalGeometryProvider() {
  if (ecalGeometry_) delete ecalGeometry_;
  ecalGeometry_ = 0;
}

std::pair<const framework::ConditionsObject*, framework::ConditionsIOV>
EcalGeometryProvider::getCondition(const ldmx::EventHeader& context) {
  static const std::string KEYNAME("detectors_valid");

  if (!ecalGeometry_) {
    framework::config::Parameters phex =
        (params_.exists("EcalGeometry"))
            ? (params_.getParameter<framework::config::Parameters>(
                  "EcalGeometry"))
            : (params_);

    // search through the subtrees
    for (auto key : phex.keys()) {
      framework::config::Parameters pver =
          phex.getParameter<framework::config::Parameters>(key);

      if (!pver.exists(KEYNAME)) {
        ldmx_log(warn) << "No parameter " << KEYNAME << " found in " << key;
        // log strange situation and continue
        continue;
      }

      std::vector<std::string> dets_valid =
          pver.getParameter<std::vector<std::string> >(KEYNAME);
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
        int nmatch = regexec(&reg, detectorGeometry_.c_str(), 0, 0, 0);
        regfree(&reg);
        if (!nmatch) {
          ecalGeometry_ = new ldmx::EcalGeometry(pver);
          break;
        }
      }
      if (ecalGeometry_) break;
    }
    if (!ecalGeometry_) {
      EXCEPTION_RAISE("GeometryException", "Unable to create EcalGeometry");
    }
  }

  return std::make_pair(
      ecalGeometry_,
      framework::ConditionsIOV(context.getRun(), context.getRun(), true, true));
}

}  // namespace ecal

DECLARE_CONDITIONS_PROVIDER_NS(ecal, EcalGeometryProvider);
