#ifndef DQM_NUCLEARDQM_H
#define DQM_NUCLEARDQM_H

#include <Math/Vector3D.h>

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"
#include "SimCore/Event/SimParticle.h"

namespace dqm {

/**
 * Base class for nuclear interaction DQM analyzers.
 *
 * Provides shared event classification, daughter finding, and kinematics
 * methods reused by PhotoNuclearDQM and ElectroNuclearDQM.
 */
class NuclearDQM : public framework::Analyzer {
 public:
  /**
   * Classification of PN/EN events by the hard particles produced above a
   * kinetic-energy threshold. Neutral pions are a separate category from
   * charged pions throughout.
   */
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

  /**
   * Compact classification focusing on very-high-energy single particles.
   */
  enum class CompactEventType {
    single_neutron = 0,
    single_charged_kaon = 1,
    single_neutral_kaon = 2,
    two_neutrons = 3,
    soft = 4,
    other = 5,
  };

  NuclearDQM(const std::string& name, framework::Process& process);
  virtual ~NuclearDQM() = default;

  /**
   * Read common configuration parameters:
   *   sim_particles_coll_name, sim_particles_passname, count_light_ions
   */
  void configure(framework::config::Parameters& parameters) override;

 protected:
  /**
   * Return daughters of @p parent that pass PDG-based filtering:
   *   - must be in the particle map
   *   - not photons (PDG 22)
   *   - not heavy nuclei (PDG > 10000) unless count_light_ions_ is true
   *     and the ion has Z <= 4
   *
   * If @p require_process_type >= 0, only daughters whose processType matches
   * are kept. Pass -1 (default) to disable process-type filtering.
   *
   * Results are sorted by kinetic energy in descending order.
   */
  std::vector<const ldmx::SimParticle*> findDaughters(
      const std::map<int, ldmx::SimParticle>& particleMap,
      const ldmx::SimParticle* parent, int require_process_type = -1) const;

  /**
   * Fill kinematic histograms for the nuclear interaction products.
   *
   * Generic (no-prefix) histograms filled:
   *   hardest_ke, hardest_theta, h_ke_h_theta,
   *   hardest_p_ke, hardest_p_theta,
   *   hardest_n_ke, hardest_n_theta,
   *   hardest_pi_ke, hardest_pi_theta,   (charged pions only)
   *   hardest_pi0_ke, hardest_pi0_theta  (neutral pions separately)
   *
   * Prefixed histograms filled (using @p prefix, e.g. "pn" or "en"):
   *   {prefix}_neutron_mult, {prefix}_proton_mult,
   *   {prefix}_charged_pion_mult, {prefix}_neutral_pion_mult,
   *   {prefix}_total_ke, {prefix}_total_neutron_ke
   */
  void findParticleKinematics(
      const std::vector<const ldmx::SimParticle*>& daughters,
      const std::string& prefix);

  /**
   * Fill extended kinematic histograms for EN interactions:
   *   hardest_pi_ke/theta (π± only), hardest_pi0_ke/theta,
   *   {prefix}_proton_mult, {prefix}_charged_pion_mult,
   *   {prefix}_neutral_pion_mult, leading_particle_type
   *
   * leading_particle_type bins:
   *   0=π±+X, 1=π⁰+X, 2=K±+X, 3=KS/KL+X, 4=p+X, 5=n+X, 6=other+X
   */
  void findExtendedKinematics(
      const std::vector<const ldmx::SimParticle*>& daughters,
      const std::string& prefix);

  /**
   * Fill subleading-kinematics histograms for 1n, 2n, charged-kaon and
   * neutral-kaon event types. Assumes daughters are sorted by KE descending.
   *
   * @p initiator is the particle that underwent the nuclear reaction
   * (pn gamma or en electron); its energy is the reference for fractions.
   */
  void findSubleadingKinematics(
      const ldmx::SimParticle* initiator,
      const std::vector<const ldmx::SimParticle*>& daughters,
      EventType eventType);

  /**
   * Classify the event by the number and type of hard daughters above
   * @p threshold [MeV] of kinetic energy. Assumes daughters sorted by KE.
   */
  EventType classifyEvent(
      const std::vector<const ldmx::SimParticle*>& daughters, double threshold);

  /**
   * Compact classification: looks for a single particle carrying >= 80% of
   * the initiator energy, or two neutrons each above @p threshold [MeV].
   */
  CompactEventType classifyCompactEvent(
      const ldmx::SimParticle* initiator,
      const std::vector<const ldmx::SimParticle*>& daughters, double threshold);

  /**
   * Return true if @p pdgCode is a light ion (Z <= 4).
   * Nuclear PDG code convention: +-10LZZZAAAI
   */
  constexpr bool isLightIon(int pdgCode) const {
    if (pdgCode > 1000000000) {
      return ((pdgCode / 10) % 1000) <= 4;
    }
    return false;
  }

  std::string sim_particles_coll_name_;
  std::string sim_particles_passname_;
  bool count_light_ions_{true};
};

}  // namespace dqm

#endif  // DQM_NUCLEARDQM_H
