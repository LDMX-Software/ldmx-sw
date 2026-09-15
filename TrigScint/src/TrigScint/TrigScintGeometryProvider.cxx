#include <regex.h>
#include <sys/types.h>

#include "DetDescr/TrigScintGeometry.h"
#include "Framework/ConditionsObjectProvider.h"
#include "Framework/EventHeader.h"
#include "Framework/RunHeader.h"

/**
 * @file TrigScintGeometryProvider.cxx
 * @brief Creates a TrigScintGeometry object based on the python specification
 *        (mirrors HcalGeometryProvider).
 */

namespace trigscint {

class TrigScintGeometryProvider : public framework::ConditionsObjectProvider {
 public:
  TrigScintGeometryProvider(const std::string& name, const std::string& tagname,
                            const framework::config::Parameters& parameters,
                            framework::Process& process);

  virtual ~TrigScintGeometryProvider();

  virtual std::pair<const framework::ConditionsObject*,
                    framework::ConditionsIOV>
  getCondition(const ldmx::EventHeader& context);

  virtual void releaseConditionsObject(const framework::ConditionsObject* co) {}

  virtual void onNewRun(ldmx::RunHeader& rh) {
    // Prefer the explicitly-configured detector (set via the python
    // 'detector' parameter, robust for real data whose RunHeader may carry no
    // detector name); otherwise fall back to the RunHeader (sim).
    const std::string rh_det = rh.getDetectorName();
    if (detector_geometry_.empty())
      detector_geometry_ = rh_det;
    else if (ts_geometry_ != nullptr && !rh_det.empty() &&
             detector_geometry_ != rh_det) {
      EXCEPTION_RAISE(
          "GeometryException",
          "Attempting to run a single job with multiple geometries " +
              detector_geometry_ + " and '" + rh_det + "'");
    }
    if (detector_geometry_.empty())
      EXCEPTION_RAISE("GeometryException",
                      "TrigScintGeometryProvider unable to get the detector "
                      "name from the RunHeader; set the 'detector' parameter.");
  }

 private:
  framework::config::Parameters params_;
  std::string detector_geometry_;
  ldmx::TrigScintGeometry* ts_geometry_;
};

TrigScintGeometryProvider::TrigScintGeometryProvider(
    const std::string& name, const std::string& tagname,
    const framework::config::Parameters& parameters,
    framework::Process& process)
    : framework::ConditionsObjectProvider{
          ldmx::TrigScintGeometry::CONDITIONS_OBJECT_NAME, tagname, parameters,
          process},
      params_{parameters} {
  ts_geometry_ = 0;
  // optional explicit detector name (real data); empty => use RunHeader
  detector_geometry_ = parameters.get<std::string>("detector", "");
}

TrigScintGeometryProvider::~TrigScintGeometryProvider() {
  if (ts_geometry_) delete ts_geometry_;
  ts_geometry_ = 0;
}

std::pair<const framework::ConditionsObject*, framework::ConditionsIOV>
TrigScintGeometryProvider::getCondition(const ldmx::EventHeader& context) {
  static const std::string keyname("detectors_valid");

  if (!ts_geometry_) {
    framework::config::Parameters pts =
        (params_.exists("trig_scint_geometry"))
            ? (params_.get<framework::config::Parameters>(
                  "trig_scint_geometry"))
            : (params_);

    for (auto key : pts.keys()) {
      framework::config::Parameters pver =
          pts.get<framework::config::Parameters>(key);

      if (!pver.exists(keyname)) {
        ldmx_log(warn) << "No parameter " << keyname << " found in " << key;
        continue;
      }

      std::vector<std::string> dets_valid =
          pver.get<std::vector<std::string> >(keyname);
      for (auto detregex : dets_valid) {
        std::string regex(detregex);
        if (regex.empty()) continue;
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
          ts_geometry_ = new ldmx::TrigScintGeometry(pver);
          break;
        }
      }
      if (ts_geometry_) break;
    }
    if (!ts_geometry_) {
      EXCEPTION_RAISE("GeometryException", "Unable to create TrigScintGeometry");
    }
  }

  return std::make_pair(
      ts_geometry_,
      framework::ConditionsIOV(context.getRun(), context.getRun(), true, true));
}

}  // namespace trigscint

DECLARE_CONDITIONS_PROVIDER(trigscint::TrigScintGeometryProvider);
