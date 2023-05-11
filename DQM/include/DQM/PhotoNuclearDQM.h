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
  /**
   * Print the particle tree.
   *
   * @param[in] particleMap The map containing the SimParticles.
   */
  void printParticleTree(std::map<int, ldmx::SimParticle> particleMap);

  /**
   * Print the daughters of a particle.
   *
   * @param[in] particleMap The map containing the SimParticles.
   * @param[in] particle The particle whose daughters will be printed.
   * @param[in] depth The tree depth.
   *
   * @return[out] A vector with the track IDs of particles that have
   *      already been printed.
   */
  std::vector<int> printDaughters(std::map<int, ldmx::SimParticle> particleMap,
                                  const ldmx::SimParticle particle, int depth);

  /** Method used to classify events. */
  int classifyEvent(const std::vector<const ldmx::SimParticle *> daughters,
                    double threshold);

  /** Method used to classify events in a compact manner. */
  int classifyCompactEvent(
      const ldmx::SimParticle *pnGamma,
      const std::vector<const ldmx::SimParticle *> daughters, double threshold);

  bool verbose_;
  std::vector<const ldmx::SimParticle *>
  findPNDaughters(const std::map<int, ldmx::SimParticle> particleMap,
                  const ldmx::SimParticle *pnGamma) const;

  void findLeadingKinematics(
      const std::vector<const ldmx::SimParticle *> &pnDaughters);

  void findSubleadingKinematics(
      const ldmx::SimParticle *pnGamma,
      const std::vector<const ldmx::SimParticle *> &pnDaughters,
      const int eventType);
  enum EventType {
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
};

} // namespace dqm

#endif // _DQM_ECAL_PN_H_
