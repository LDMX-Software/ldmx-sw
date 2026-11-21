#include <regex.h>

#include <array>
#include <map>
#include <memory>

#include "Conditions/SimpleTableCondition.h"
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
   * @note Without an alignment table the geometry is valid for all time.
   * With one, the ECal is shifted by the table's offset and the geometry is
   * valid for the table's interval of validity. Users should not cache the
   * pointer between events
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
    // the configured detector wins (real data RunHeaders carry no detector)
    const std::string rh_det = rh.getDetectorName();
    if (detector_geometry_.empty())
      detector_geometry_ = rh_det;
    else if ((ecal_geometry_ != nullptr || !aligned_.empty()) &&
             !rh_det.empty() && detector_geometry_ != rh_det) {
      EXCEPTION_RAISE(
          "GeometryException",
          "Attempting to run a single job with multiple geometries " +
              detector_geometry_ + " and '" + rh_det + "'");
    }
    // make sure detector name has been set
    if (detector_geometry_.empty())
      EXCEPTION_RAISE("GeometryException",
                      "EcalGeometryProvider unable to get the name of the "
                      "detector from the RunHeader; set the 'detector' "
                      "parameter.");
  }

 private:
  /** Geometry parameters matching the detector name */
  const framework::config::Parameters& findParameters();

  /** parameters for the various geometry versions we can support */
  std::vector<framework::config::Parameters> geometries_;
  /** Alignment table (DX, DY, DZ [mm] in the EcalID(0, 0, 0) row), empty: none
   */
  std::string alignment_;
  /** Aligned geometries, one per offset */
  std::map<std::array<double, 3>, std::unique_ptr<ldmx::EcalGeometry>> aligned_;
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
      parameters.get<std::vector<framework::config::Parameters>>("geometries");
  // optional explicit detector name (real data), empty: use the RunHeader
  detector_geometry_ = parameters.get<std::string>("detector", "");
  alignment_ = parameters.get<std::string>("alignment", "");
  ecal_geometry_ = 0;
}

EcalGeometryProvider::~EcalGeometryProvider() {
  if (ecal_geometry_) delete ecal_geometry_;
  ecal_geometry_ = 0;
}

const framework::config::Parameters& EcalGeometryProvider::findParameters() {
  static const std::string keyname("detectors_valid");
  // search through the subtrees
  for (const auto& pver : geometries_) {
    if (!pver.exists(keyname)) {
      ldmx_log(warn) << "No parameter " << keyname
                     << " found one of the detector vesrsions.";
      // log strange situation and continue
      continue;
    }

    std::vector<std::string> dets_valid =
        pver.get<std::vector<std::string>>(keyname);
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
      if (!nmatch) return pver;
    }
  }
  EXCEPTION_RAISE("GeometryException", "Unable to create EcalGeometry");
}

std::pair<const framework::ConditionsObject*, framework::ConditionsIOV>
EcalGeometryProvider::getCondition(const ldmx::EventHeader& context) {
  if (alignment_.empty()) {
    if (!ecal_geometry_)
      ecal_geometry_ = new ldmx::EcalGeometry(findParameters());
    return std::make_pair(
        ecal_geometry_, framework::ConditionsIOV(context.getRun(),
                                                 context.getRun(), true, true));
  }

  // whole-ECal shift from the alignment table, valid as long as the table
  auto [obj, iov] = requestParentCondition(alignment_, context);
  auto table = dynamic_cast<const conditions::DoubleTableCondition*>(obj);
  if (!table) {
    EXCEPTION_RAISE("GeometryException", "ECal alignment '" + alignment_ +
                                             "' is not a table of doubles");
  }
  const unsigned int id{ldmx::EcalID(0, 0, 0).raw()};
  const std::array<double, 3> offset{table->getByName(id, "DX"),
                                     table->getByName(id, "DY"),
                                     table->getByName(id, "DZ")};
  auto& geo = aligned_[offset];
  if (!geo) {
    ldmx_log(info) << "ECal shifted by (" << offset[0] << ", " << offset[1]
                   << ", " << offset[2] << ") mm for run " << context.getRun();
    geo.reset(new ldmx::EcalGeometry(findParameters(), offset));
  }
  return std::make_pair(geo.get(), iov);
}

}  // namespace ecal

DECLARE_CONDITIONS_PROVIDER(ecal::EcalGeometryProvider);
