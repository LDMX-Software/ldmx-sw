/**
 * @file LDMXCascadeInterface.cxx
 */

#include "SimCore/Bertini/LDMXCascadeInterface.h"

#include <G4HadFinalState.hh>
#include <G4HadSecondary.hh>
#include <cmath>

#include "G4InuclParticleNames.hh"
#include "SimCore/Bertini/CascadeHistoryStore.h"

namespace simcore {
namespace bertini {

namespace {

/**
 * Special PDG-like codes for Bertini quasi-deuteron states.
 * These are virtual particles used internally by Bertini to represent
 * correlated nucleon pairs in the nucleus that absorb the photon.
 * Format: 99XXX where XXX matches the Bertini internal code.
 */
constexpr int PDG_DIPROTON = 99111;   ///< pp quasi-deuteron (Bertini code 111)
constexpr int PDG_UNBOUNDPN = 99112;  ///< pn quasi-deuteron (Bertini code 112)
constexpr int PDG_DINEUTRON = 99122;  ///< nn quasi-deuteron (Bertini code 122)

/**
 * Get PDG code from Bertini internal type code
 * Uses manual mapping since G4InuclElementaryParticle::makeDefinition is
 * protected
 */
int getPdgCode(int inuclType) {
  using namespace G4InuclParticleNames;
  switch (inuclType) {
    case proton:
      return 2212;
    case neutron:
      return 2112;
    case pionPlus:
      return 211;
    case pionMinus:
      return -211;
    case pionZero:
      return 111;
    case photon:
      return 22;
    case kaonPlus:
      return 321;
    case kaonMinus:
      return -321;
    case kaonZero:
      return 311;
    case kaonZeroBar:
      return -311;
    case lambda:
      return 3122;
    case sigmaPlus:
      return 3222;
    case sigmaZero:
      return 3212;
    case sigmaMinus:
      return 3112;
    case xiZero:
      return 3322;
    case xiMinus:
      return 3312;
    case omegaMinus:
      return 3334;
    case electron:
      return 11;
    case positron:
      return -11;
    case muonMinus:
      return 13;
    case muonPlus:
      return -13;
    case deuteron:
      return 1000010020;
    case triton:
      return 1000010030;
    case He3:
      return 1000020030;
    case alpha:
      return 1000020040;
    // Quasi-deuteron states (virtual nucleon pairs for photon absorption)
    case diproton:
      return PDG_DIPROTON;
    case unboundPN:
      return PDG_UNBOUNDPN;
    case dineutron:
      return PDG_DINEUTRON;
    default:
      return 0;
  }
}

/**
 * Get the electric charge for a PDG code
 * Used for inferring target nucleon from conservation laws
 */
int getCharge(int pdg) {
  // Baryons
  if (pdg == 2212) return +1;  // proton
  if (pdg == 2112) return 0;   // neutron
  if (pdg == 3122) return 0;   // Lambda
  if (pdg == 3222) return +1;  // Sigma+
  if (pdg == 3212) return 0;   // Sigma0
  if (pdg == 3112) return -1;  // Sigma-
  if (pdg == 3322) return 0;   // Xi0
  if (pdg == 3312) return -1;  // Xi-
  if (pdg == 3334) return -1;  // Omega-

  // Mesons
  if (pdg == 211) return +1;   // pi+
  if (pdg == -211) return -1;  // pi-
  if (pdg == 111) return 0;    // pi0
  if (pdg == 321) return +1;   // K+
  if (pdg == -321) return -1;  // K-
  if (pdg == 311) return 0;    // K0
  if (pdg == -311) return 0;   // K0bar
  if (pdg == 310) return 0;    // K0S
  if (pdg == 130) return 0;    // K0L

  // Leptons/photons
  if (pdg == 22) return 0;    // photon
  if (pdg == 11) return -1;   // electron
  if (pdg == -11) return +1;  // positron
  if (pdg == 13) return -1;   // muon-
  if (pdg == -13) return +1;  // muon+

  // Light nuclei
  if (pdg == 1000010020) return +1;  // deuteron
  if (pdg == 1000010030) return +1;  // triton
  if (pdg == 1000020030) return +2;  // He3
  if (pdg == 1000020040) return +2;  // alpha

  // Quasi-deuterons
  if (pdg == PDG_DIPROTON) return +2;   // pp
  if (pdg == PDG_UNBOUNDPN) return +1;  // pn
  if (pdg == PDG_DINEUTRON) return 0;   // nn

  return 0;  // unknown
}

/**
 * Get baryon number for a PDG code
 * Used for inferring target nucleon from conservation laws
 */
int getBaryonNumber(int pdg) {
  // Baryons
  if (pdg == 2212 || pdg == 2112) return +1;  // p, n
  if (pdg == 3122 || pdg == 3222 || pdg == 3212 || pdg == 3112)
    return +1;                                               // Lambda, Sigmas
  if (pdg == 3322 || pdg == 3312 || pdg == 3334) return +1;  // Xis, Omega

  // Light nuclei
  if (pdg == 1000010020) return +2;  // deuteron
  if (pdg == 1000010030) return +3;  // triton
  if (pdg == 1000020030) return +3;  // He3
  if (pdg == 1000020040) return +4;  // alpha

  // Quasi-deuterons
  if (pdg == PDG_DIPROTON || pdg == PDG_UNBOUNDPN || pdg == PDG_DINEUTRON)
    return +2;

  return 0;  // mesons, leptons, photons
}

/**
 * Infer target nucleon PDG from charge and baryon number conservation
 *
 * For a reaction: bullet + target -> products
 * deltaCharge = Q_products - Q_bullet
 * deltaBaryon = B_products - B_bullet
 *
 * Single nucleon: deltaBaryon = 1
 *   - deltaCharge = 1 -> proton
 *   - deltaCharge = 0 -> neutron
 *
 * Quasi-deuteron: deltaBaryon = 2
 *   - deltaCharge = 2 -> pp (diproton)
 *   - deltaCharge = 1 -> pn (unbound pn)
 *   - deltaCharge = 0 -> nn (dineutron)
 */
int inferTargetPdg(int deltaCharge, int deltaBaryon) {
  if (deltaBaryon == 1) {
    // Single nucleon target
    if (deltaCharge == 1) return 2212;  // proton
    if (deltaCharge == 0) return 2112;  // neutron
  } else if (deltaBaryon == 2) {
    // Quasi-deuteron target
    if (deltaCharge == 2) return PDG_DIPROTON;   // pp
    if (deltaCharge == 1) return PDG_UNBOUNDPN;  // pn
    if (deltaCharge == 0) return PDG_DINEUTRON;  // nn
  }
  return 0;  // unknown
}

}  // anonymous namespace

LDMXCascadeInterface::LDMXCascadeInterface(const G4String& name)
    : G4CascadeInterface(name), incident_track_id_{-1} {}

LDMXCascadeInterface::~LDMXCascadeInterface() = default;

G4HadFinalState* LDMXCascadeInterface::ApplyYourself(
    const G4HadProjectile& projectile, G4Nucleus& targetNucleus) {
  double photon_energy = projectile.GetTotalEnergy();

  ldmx_log(debug) << "LDMXCascadeInterface::ApplyYourself";
  ldmx_log(debug) << "  Track ID: " << incident_track_id_;
  ldmx_log(debug) << "  Photon energy: " << photon_energy << " MeV";
  ldmx_log(debug) << "  Energy threshold: " << energy_threshold_ << " MeV";

  last_history_.clear();

  bool should_record = (photon_energy >= energy_threshold_);

  if (should_record) {
    ldmx_log(debug) << "  Recording history (above threshold)";
    // Geant4 only creates the history object if G4CASCADE_SHOW_HISTORY is set,
    // so we force-create it here
    ensureCascadeHistoryExists();
  } else {
    ldmx_log(debug) << "  Skipping history (below threshold)";
  }

  G4HadFinalState* result =
      G4CascadeInterface::ApplyYourself(projectile, targetNucleus);

  if (should_record) {
    captureHistory();
    last_history_.setIncidentEnergy(photon_energy);

    ldmx_log(debug) << "  Captured " << last_history_.getSteps().size()
                    << " steps";

    // De-excitation (evaporation, gamma) happens after the cascade via
    // G4ExcitationHandler. These products appear in the final state but
    // are not in G4CascadeHistory, so we capture them separately.
    captureDeexcitationProducts(result);

    ldmx_log(debug) << "  Total steps: " << last_history_.getSteps().size();

    if (!last_history_.empty()) {
      CascadeHistoryStore::getInstance().addHistory(incident_track_id_,
                                                    last_history_);
    }
  }

  return result;
}

void LDMXCascadeInterface::ensureCascadeHistoryExists() {
  // Force-create the cascade history object if it doesn't exist
  // Geant4 normally only creates this if G4CASCADE_SHOW_HISTORY envvar is set

  if (!collider) {
    ldmx_log(debug) << "  ensureCascadeHistoryExists: no collider yet";
    return;
  }

  G4IntraNucleiCascader* cascader = collider->theIntraNucleiCascader;
  if (!cascader) {
    ldmx_log(debug) << "  ensureCascadeHistoryExists: no cascader yet";
    return;
  }

  if (!cascader->theCascadeHistory) {
    ldmx_log(debug)
        << "  ensureCascadeHistoryExists: creating G4CascadeHistory";
    cascader->theCascadeHistory = new G4CascadeHistory;
  } else {
    ldmx_log(debug) << "  ensureCascadeHistoryExists: history already exists";
  }
}

void LDMXCascadeInterface::captureHistory() {
  // Navigate: this->collider->theIntraNucleiCascader->theCascadeHistory
  if (!collider) return;

  G4IntraNucleiCascader* cascader = collider->theIntraNucleiCascader;
  if (!cascader) return;

  G4CascadeHistory* g4_history = cascader->theCascadeHistory;
  if (!g4_history) return;

  last_history_.setIncidentTrackId(incident_track_id_);

  if (cascader->tnuclei) {
    last_history_.setTargetNucleus(cascader->tnuclei->getA(),
                                   cascader->tnuclei->getZ());
  }

  const std::vector<G4CascadeHistory::HistoryEntry>& entries =
      g4_history->theHistory;

  if (entries.empty()) {
    return;
  }

  // Build parent ID map from daughter lists
  std::vector<int> parent_ids(entries.size(), -1);

  for (size_t i = 0; i < entries.size(); ++i) {
    const auto& entry = entries[i];
    for (int d = 0; d < entry.n && d < 10; ++d) {
      int daughter_id = entry.dId[d];
      if (daughter_id >= 0 &&
          static_cast<size_t>(daughter_id) < entries.size()) {
        parent_ids[daughter_id] = static_cast<int>(i);
      }
    }
  }

  // First pass: convert G4 entries to LDMX steps
  std::vector<ldmx::CascadeStep> steps;
  steps.reserve(entries.size());

  for (size_t i = 0; i < entries.size(); ++i) {
    const auto& entry = entries[i];
    int parent_id = parent_ids[i];

    ldmx::CascadeStep step;

    const G4CascadParticle& cpart = entry.cpart;
    const G4InuclElementaryParticle& particle = cpart.getParticle();

    step.setHistoryId(cpart.getHistoryId());
    step.setParentId(parent_id);
    step.setPdgId(getPdgCode(particle.type()));

    // Geant4 Bertini uses GeV for momentum, fm for position
    G4LorentzVector mom = cpart.getMomentum();
    step.setMomentum(mom.px() * 1000.0, mom.py() * 1000.0, mom.pz() * 1000.0,
                     mom.e() * 1000.0);

    const G4ThreeVector& pos = cpart.getPosition();
    step.setPosition(pos.x(), pos.y(), pos.z());

    step.setGeneration(cpart.getGeneration());
    step.setZone(cpart.getCurrentZone());

    std::vector<int> daughter_ids;
    for (int d = 0; d < entry.n && d < 10; ++d) {
      daughter_ids.push_back(entry.dId[d]);
    }
    step.setDaughterIds(daughter_ids);

    bool interacted = (entry.n > 0);
    step.setInteracted(interacted);

    // Escaped if didn't interact (simplification - true escape needs more
    // tracking)
    bool escaped = !interacted && cpart.getGeneration() >= 0;
    step.setEscaped(escaped);
    step.setTargetPdgId(0);  // inferred in second pass

    ldmx::CascadeStage stage = ldmx::CascadeStage::UNKNOWN;
    int generation = cpart.getGeneration();

    if (generation == 0) {
      stage = ldmx::CascadeStage::INCIDENT;
    } else if (generation == 1) {
      stage = ldmx::CascadeStage::PRIMARY;
    } else if (!interacted && !escaped) {
      stage = ldmx::CascadeStage::ABSORBED;
    } else if (escaped && generation <= 2) {
      stage = ldmx::CascadeStage::PREEQUILIBRIUM;
    } else {
      stage = ldmx::CascadeStage::CASCADE;
    }
    step.setStage(stage);

    steps.push_back(std::move(step));
  }

  // Second pass: infer target nucleon from charge/baryon conservation
  for (auto& step : steps) {
    if (!step.didInteract()) continue;

    int bullet_charge = getCharge(step.getPdgId());
    int bullet_baryon = getBaryonNumber(step.getPdgId());
    int daughter_charge = 0;
    int daughter_baryon = 0;

    for (int daughter_id : step.getDaughterIds()) {
      for (const auto& s : steps) {
        if (s.getHistoryId() == daughter_id) {
          daughter_charge += getCharge(s.getPdgId());
          daughter_baryon += getBaryonNumber(s.getPdgId());
          break;
        }
      }
    }

    int delta_charge = daughter_charge - bullet_charge;
    int delta_baryon = daughter_baryon - bullet_baryon;
    step.setTargetPdgId(inferTargetPdg(delta_charge, delta_baryon));
  }

  last_history_.reserve(steps.size());
  for (auto& step : steps) {
    last_history_.addStep(std::move(step));
  }

  // Excitation energy approximation: E_incident - sum(KE of escaped)
  double total_escaped_energy = 0.0;
  int escaped_protons = 0;
  int escaped_neutrons = 0;

  for (const auto& step : last_history_.getSteps()) {
    if (step.didEscape()) {
      total_escaped_energy += step.getKineticEnergy();
      int pdg = step.getPdgId();
      if (pdg == 2212)
        escaped_protons++;
      else if (pdg == 2112)
        escaped_neutrons++;
    }
  }

  double excitation_energy =
      last_history_.getIncidentEnergy() - total_escaped_energy;
  if (excitation_energy < 0) excitation_energy = 0;
  last_history_.setExcitationEnergy(excitation_energy);

  int target_a = last_history_.getTargetA();
  int target_z = last_history_.getTargetZ();
  int residual_a = target_a - escaped_protons - escaped_neutrons;
  int residual_z = target_z - escaped_protons;
  if (residual_a < 0) residual_a = 0;
  if (residual_z < 0) residual_z = 0;
  last_history_.setResidualNucleus(residual_a, residual_z);
}

void LDMXCascadeInterface::captureDeexcitationProducts(
    G4HadFinalState* finalState) {
  if (!finalState) return;

  int n_secondaries = finalState->GetNumberOfSecondaries();
  if (n_secondaries == 0) return;

  // Match final state particles against cascade escapees to identify
  // de-excitation products. Energy matching has 1 MeV tolerance for recoil.
  struct EscapedParticle {
    int pdg_;
    double energy_;
    bool matched_;
  };
  std::vector<EscapedParticle> cascade_escaped;

  for (const auto& step : last_history_.getSteps()) {
    if (step.didEscape()) {
      cascade_escaped.push_back({step.getPdgId(), step.getEnergy(), false});
    }
  }

  int next_history_id = 0;
  for (const auto& step : last_history_.getSteps()) {
    if (step.getHistoryId() >= next_history_id) {
      next_history_id = step.getHistoryId() + 1;
    }
  }

  for (int i = 0; i < n_secondaries; ++i) {
    G4HadSecondary* secondary = finalState->GetSecondary(i);
    if (!secondary) continue;

    const G4DynamicParticle* dyn_particle = secondary->GetParticle();
    if (!dyn_particle) continue;

    int pdg = dyn_particle->GetPDGcode();
    double energy = dyn_particle->GetTotalEnergy();  // MeV

    bool is_deexcitation = true;
    const double energy_tolerance = 1.0;

    for (auto& escaped : cascade_escaped) {
      if (!escaped.matched_ && escaped.pdg_ == pdg &&
          std::abs(escaped.energy_ - energy) < energy_tolerance) {
        escaped.matched_ = true;
        is_deexcitation = false;
        break;
      }
    }

    if (is_deexcitation) {
      ldmx::CascadeStep step;
      step.setHistoryId(next_history_id++);
      step.setParentId(-2);  // marker for de-excitation origin
      step.setPdgId(pdg);

      G4ThreeVector mom = dyn_particle->GetMomentum();
      step.setMomentum(mom.x(), mom.y(), mom.z(), energy);
      step.setPosition(0, 0, 0);
      step.setGeneration(-1);  // marker for de-excitation
      step.setZone(0);
      step.setInteracted(false);
      step.setEscaped(true);
      step.setStage(ldmx::CascadeStage::DEEXCITATION);

      last_history_.addStep(std::move(step));
    }
  }
}

}  // namespace bertini
}  // namespace simcore
