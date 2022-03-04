#ifndef BIASING_UTILITY_DECAYCHILDRENKEEPER_H
#define BIASING_UTILITY_DECAYCHILDRENKEEPER_H

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
 * Filter to keep tracks that are the products of certain
 * particle's decays.
 *
 * @note It is important to emphasize that this filter
 * is only able to persist products of particles that
 * are themselves kept. i.e. Another action needs to
 * keep the parent particle and then this action will
 * save that particle's children.
 */
class DecayChildrenKeeper : public simcore::UserAction {
 public:
  /**
   * Constructor.
   *
   * @param[in] name the name of the instance of this UserAction.
   * @param[in] parameters the parameters used to configure this
   *      UserAction.
   */
  DecayChildrenKeeper(const std::string& name, framework::config::Parameters& parameters);

  /// Destructor
  ~DecayChildrenKeeper();

  /**
   * Method called when a track is done being processed.
   *
   * We get the stored particle map from the TrackMap object
   * and look for the current track's parent in it. If the
   * current track's parent is in it THEN we check if that
   * parent has a PDG matching ANY of the configured PDGs.
   *
   * @note Techinically, we can check for children of any
   * particle that has finished processing (and is therefore
   * in the stored particle map), but I focus here on "decays"
   * because that is the only one where the parent is
   * _definitely_ at the end of processing.
   *
   * @param[in] track Geant4 track associated with a particle.
   */
  void PostUserTrackingAction(const G4Track* track) final override;

  /// Retrieve the type of actions this class defines.
  std::vector<simcore::TYPE> getTypes() final override {
    return {simcore::TYPE::TRACKING};
  }

 private:
  /// The PDG IDs for which to keep decay children
  std::vector<int> parents_;

};  // DecayChildrenKeeper

}  // namespace utility
}  // namespace biasing

#endif  // BIASING_UTILITY_TRACKPROCESSFILTER_H
