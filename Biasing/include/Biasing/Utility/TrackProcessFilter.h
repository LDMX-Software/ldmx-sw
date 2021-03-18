#ifndef BIASING_UTILITY_TRACKPROCESSFILTER_H
#define BIASING_UTILITY_TRACKPROCESSFILTER_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <string>

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserAction.h"

namespace biasing {
namespace utility {

/**
 * Filter used to tag tracks for persistence based on the process they were
 * created from.
 *
 * All tracks are checked during the post user tracking action stage and
 * are tagged to be persisted if the process used to create them match the
 * user specified process. The process name specified by the user needs to
 * match the names assigned by Geant4.
 *
 * @note Any track created through a process *not* the same as the configured
 * process will *not* be saved.
 *
 */
class TrackProcessFilter : public simcore::UserAction {
 public:
  /**
   * Constructor.
   *
   * @param[in] name the name of the instance of this UserAction.
   * @param[in] parameters the parameters used to configure this
   *      UserAction.
   */
  TrackProcessFilter(const std::string& name, framework::config::Parameters& parameters);

  /// Destructor
  ~TrackProcessFilter();

  /**
   * Method called when a track is done being processed.
   *
   * @param[in] track Geant4 track associated with a particle.
   */
  void PostUserTrackingAction(const G4Track* track) final override;

  /// Retrieve the type of actions this class defines.
  std::vector<simcore::TYPE> getTypes() final override {
    return {simcore::TYPE::TRACKING};
  }

 private:
  /// The process to filter on.
  std::string process_{""};

};  // TrackProcessFilter

}  // namespace utility
}  // namespace biasing

#endif  // BIASING_UTILITY_TRACKPROCESSFILTER_H
