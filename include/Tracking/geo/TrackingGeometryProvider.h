
#include <string>

#include "Framework/ConditionsObjectProvider.h"


// Foward decs
namespace framework { 
class ConditionsIOV;
class ConditionsObject;
class Process;
}

namespace framework::config { 
class Parameters;
}

namespace ldmx { 
class EventHeader;
}

namespace tracking::geo {

class TrackingGeometryProvider : public framework::ConditionsObjectProvider {
 public:
  /**
   * Constructor
   *
   * @param name Name of ths instance of the class.
   * @param tag_name The tag for the database entry (should not include
   *   whitespace)
   * @param parameters
   * @param process The Process class associated with ConditionsObjectProvider,
   *   provided by the framework.
   */
  TrackingGeometryProvider(const std::string& name, const std::string& tag_name,
                           const framework::config::Parameters& parameters,
                           framework::Process& process);

  /// Destructor
  ~TrackingGeometryProvider() = default;

  /**
   */
  std::pair<const framework::ConditionsObject*, framework::ConditionsIOV>
  getCondition(const ldmx::EventHeader& context);

 private: 
};


}  // namespace tracking::geo
