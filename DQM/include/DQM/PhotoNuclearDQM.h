#ifndef DQM_PHOTONUCLEARDQM_H
#define DQM_PHOTONUCLEARDQM_H

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

#include "SimCore/Event/SimParticle.h"

namespace dqm {

// Forward declarations within the ldmx workspace
class Event;
class SimParticle;

class PhotoNuclearDQM : public framework::Analyzer {

  // Classification schemes for PN events
  enum class CompactEventType {
    single_neutron = 0,
    single_charged_kaon = 1,
    single_neutral_kaon = 2,
    two_neutrons = 3,
    soft = 4,
    other = 5,
  };
  enum class EventType {
    nothing_hard = 0,
    single_neutron = 1,
    two_neutrons = 2,
    three_or_more_neutrons = 3,
    single_charged_pion = 4,
    two_charged_pions = 5,
    single_neutral_pion = 6,
    single_charged_pion_and_nucleon = 7,
    single_charged_pion_and_two_nucleons = 8,
    two_charged_pions_and_nucleon = 9,
    single_neutral_pion_and_nucleon = 10,
    single_neutral_pion_and_two_nucleons = 11,
    single_neutral_pion_charged_pion_and_nucleon = 12,
    single_proton = 13,
    two_protons = 14,
    proton_neutron = 15,
    klong = 16,
    charged_kaon = 17,
    kshort = 18,
    exotics = 19,
    multibody = 20,
  };

public:
  /// Constructor
  PhotoNuclearDQM(const std::string &name, framework::Process &process);

  /// Destructor
  ~PhotoNuclearDQM();

  /**
   * Configure this analyzer using the user specified parameters.
   *
   * @param parameters Set of parameters used to configure this
   *                   analyzer.
   */
  void configure(framework::config::Parameters &parameters) final override;

  /**
   * Process the event and create the histogram summaries.
   *
   * @param event The event to analyze.
   */
  void analyze(const framework::Event &event) final override;

  /// Method executed before processing of events begins.
  void onProcessStart();

private:
  /** Method used to classify events. */
  EventType
  classifyEvent(const std::vector<const ldmx::SimParticle *> daughters,
                double threshold);

  /** Method used to classify events in a compact manner. */
  CompactEventType
  classifyCompactEvent(const ldmx::SimParticle *pnGamma,
                       const std::vector<const ldmx::SimParticle *> daughters,
                       double threshold);

  /**
   * Fill the recoil electron-histograms
   * */
  void findRecoilProperties(const ldmx::SimParticle *recoil);

  /**
   *
   * Find all daughter particles of a parent. Particles are included if they>
   *
   * - Are in the particle map,
   * - Are not photons or nuclear fragment, and
   * - Are not a light ion (Z < 4) if the \ref count_light_ions_ parameter is
       set to false
   *
   * The products are sorted by kinetic energy, in descending order.
   *
   **/
  std::vector<const ldmx::SimParticle *>
  findDaughters(const std::map<int, ldmx::SimParticle> particleMap,
                const ldmx::SimParticle *parent) const;

  /**
   *
   * Fill histograms related to kinematics of PN products
   *
   **/
  void findParticleKinematics(
      const std::vector<const ldmx::SimParticle *> &pnDaughters);

  void findSubleadingKinematics(
      const ldmx::SimParticle *pnGamma,
      const std::vector<const ldmx::SimParticle *> &pnDaughters, //
      const EventType eventType);

public:
  /**
   *  Check if the PDG code corresponds to a light ion.
   *
   *  Nuclear PDG codes are given by ±10LZZZAAAI So to find the atomic
   *  number, we first divide by 10 (to lose the I-component) and then
   *  take the modulo with 1000.
   *
   *  TODO: Repeated code from SimCore, could probably live elsewhere.
   *
   */
  constexpr bool isLightIon(const int pdgCode) const {
    //
    // TODO: Is the < check necessary?
    if (pdgCode > 1000000000 && pdgCode < 10000000000) {
      // Check if the atomic number is less than or equal to 4
      return ((pdgCode / 10) % 1000) <= 4;
    }
    return false;
  }

  bool verbose_;
  bool count_light_ions_;
};

} // namespace dqm

#endif // _DQM_ECAL_PN_H_
