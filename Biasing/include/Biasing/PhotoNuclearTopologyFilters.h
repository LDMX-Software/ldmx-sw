#ifndef PHOTONUCLEARTOPOLOGYFILTERS_H
#define PHOTONUCLEARTOPOLOGYFILTERS_H

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserAction.h"
#include "SimCore/UserTrackInformation.h"
/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include <G4RunManager.hh>
#include <G4Step.hh>

#include "Framework/Configure/Parameters.h"
// Forward declaration

namespace biasing {

/**
 * Abstract base class for a user action used to filter out photo-nuclear events
 * that don't match the topology the user is interested in studying. Derived
 * classes implement the event rejection
 *
 * Similar to the PhotoNuclearProductsFilter, this user action will only process
 * steps whose associated track has been tagged as a "PN Gamma". This tag is
 * currently only set in ECalProcessFilter and needs to be placed in the
 * UserAction pipeline before this class.
 *
 */
class PhotoNuclearTopologyFilter : public simcore::UserAction {
 public:
  /**
   * Constructor
   *
   * @param[in] name The name of this class instance.
   * @param[in] parameters The parameters used to configure this class.
   */
  PhotoNuclearTopologyFilter(const std::string& name,
                             framework::config::Parameters& parameters);

  /// Destructor
  ~PhotoNuclearTopologyFilter() = default;

  /**
   * Callback that allows a user to take some actions at the end of
   * a step.
   *
   * @param[in] step The Geant4 step containing transient information
   *      about the step taken by a track.
   */
  void stepping(const G4Step* step) override;

  virtual bool rejectEvent(const std::vector<G4Track*>& secondaries) const = 0;

  /// Retrieve the type of actions this class defines
  std::vector<simcore::TYPE> getTypes() override {
    return {simcore::TYPE::STEPPING};
  }

 protected:
  /**
   *  Check if the PDG code corresponds to a light ion nucleus.
   *
   *  Nuclear PDG codes are given by ±10LZZZAAAI So to find the atomic
   *  number, we first divide by 10 (to lose the I-component) and then
   *  take the modulo with 1000.
   *
   *  TODO: Repeated code from SimCore, could probably live elsewhere.
   *
   */
  constexpr bool isLightIon(const int pdgCode) const {
    if (pdgCode > 1000000000) {
      // Check if the atomic number is less than or equal to 4
      return ((pdgCode / 10) % 1000) <= 4;
    }
    return false;
  }

  /**
   * Whether or not to include a particular particle type in any counting.
   * Unless \ref count_light_ions_ is set, we don't count anything with a
   * nuclear PDG code. This is consistent with the counting behaviour used in
   * the PhotoNuclearDQM.
   *
   * If \ref count_light_ions_ is set, we also match PDG codes for nuclei with
   * atomic number < 4. 
   *
     TODO: Repeated code from SimCore, could probably live elsewhere.
   * @see isLightIon
   *
   */
  constexpr bool skipCountingParticle(const int pdgcode) const {
    return !(pdgcode < 10000 || (count_light_ions_ && isLightIon(pdgcode)));
  }

  constexpr bool isNeutron(const int pdgID) const { return pdgID == 2112; }

  bool count_light_ions_;
  double hard_particle_threshold_;
};  // PhotoNuclearTopologyFilter

class SingleNeutronFilter : public PhotoNuclearTopologyFilter {
 public:
  SingleNeutronFilter(const std::string& name,
                      framework::config::Parameters& parameters)
      : PhotoNuclearTopologyFilter{name, parameters} {}

  bool rejectEvent(const std::vector<G4Track*>& secondaries) const override;
};

class NothingHardFilter : public PhotoNuclearTopologyFilter {
 public:
  NothingHardFilter(const std::string& name,
                    framework::config::Parameters& parameters)
      : PhotoNuclearTopologyFilter{name, parameters} {}

  bool rejectEvent(const std::vector<G4Track*>& secondaries) const override;
};
}  // namespace biasing

#endif /* PHOTONUCLEARTOPOLOGYFILTERS_H */
