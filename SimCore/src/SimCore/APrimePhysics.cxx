/**
 * @file APrimePhysics.cxx
 * @brief Class which defines basic APrime physics
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimCore/APrimePhysics.h"

namespace simcore {

const std::string APrimePhysics::NAME = "APrime";

/**
 * Store the atomic Z for the element in which the dark brem occurred.
 *
 * This function is registered with G4DarkBremsstrahlung and will be
 * called with the element that the dark brem will occurr off of. We
 * store the element Z in UserEventInformation for later serialization
 * into the EventHeader
 *
 * @param[in] element G4Element off-which the dark brem occurred
 */
static void storeElementZ(const G4Element& element) {
  static_cast<UserEventInformation*>(
      G4EventManager::GetEventManager()->GetUserInformation())
      ->setDarkBremMaterialZ(element.GetZ());
}

APrimePhysics::APrimePhysics(const framework::config::Parameters& params)
    : G4VPhysicsConstructor(APrimePhysics::NAME),
      parameters_{params},
      process_{nullptr} {
  ap_mass_ = parameters_.get<double>("ap_mass", 0.) * MeV;
  enable_ = parameters_.get<bool>("enable", false);
  fcp_enable_ = parameters_.get<bool>("fcp_enable", false);
  fcp_mass_ = parameters_.get<double>("fcp_mass", 0.) * MeV;
  fcp_charge_ = parameters_.get<double>("fcp_charge", 0.1);
  fcp_xsec_factor_ = parameters_.get<double>("fcp_xsec_factor", 1.0);

  ldmx_log(info) << "Configuration:" << " A' mass: " << ap_mass_ / MeV << " MeV"
                 << ", Dark brem enabled: " << (enable_ ? "YES" : "NO")
                 << ", FCP enabled: " << (fcp_enable_ ? "YES" : "NO")
                 << ", FCP mass: " << fcp_mass_ / MeV << " MeV"
                 << ", FCP charge: " << fcp_charge_ << " e"
                 << ", FCP xsec factor: " << fcp_xsec_factor_;
}

void APrimePhysics::ConstructParticle() {
  auto model{parameters_.get<framework::config::Parameters>("model")};
  static const std::map<std::string, G4APrime::DecayMode> decay_lut = {
      {"no_decay", G4APrime::DecayMode::NoDecay},
      {"flat_decay", G4APrime::DecayMode::FlatDecay},
      {"geant_decay", G4APrime::DecayMode::GeantDecay}};
  auto decay_it{
      decay_lut.find(model.get<std::string>("decay_mode", "no_decay"))};
  if (decay_it == decay_lut.end()) {
    EXCEPTION_RAISE(
        "BadConf",
        "Unrecognized decay mode '" + model.get<std::string>("decay_mode") +
            "',"
            " options are 'no_decay', 'flat_decay', or 'geant_decay'.");
  }

  double ap_tau = model.get<double>("ap_tau", -1.0);

  /**
   * Insert A-prime into the Geant4 particle table.
   *
   * Geant4 registers all instances derived from G4ParticleDefinition and
   * deletes them at the end of the run. We configure the A' to have
   * the input mass and the PDG ID number of 622.
   *
   * If fcp is enabled, the A' decays to fcp+fcp- (decay_id=17),
   * otherwise to e+e- (decay_id=11).
   */
  int decay_id = fcp_enable_ ? 17 : 11;
  ldmx_log(info) << "Initializing A' with:" << ", Mass: " << ap_mass_ / MeV
                 << " MeV" << ", PDG ID: 622" << ", Lifetime (tau): " << ap_tau
                 << ", Decay mode: "
                 << model.get<std::string>("decay_mode", "no_decay")
                 << ", Decay product ID: " << decay_id
                 << (fcp_enable_ ? " (fcp+fcp-)" : " (e+e-)");

  // Initialize the A' with the specified parameters
  G4APrime::Initialize(ap_mass_, 622, ap_tau, decay_it->second, decay_id);

  /**
   * If fcp is enabled, initialize the fractionally charged particles.
   * This registers fcp- and fcp+ in the Geant4 particle table.
   */
  if (fcp_enable_) {
    G4FractionallyCharged::Initialize(fcp_mass_, 17, fcp_charge_);
  }
}

void APrimePhysics::ConstructProcess() {
  ldmx_log(trace) << "=== APrimePhysics::ConstructProcess ===";
  // add process to electron if we are enabled
  if (enable_) {
    ldmx_log(debug)
        << "Dark brem is enabled, setting up G4DarkBremsstrahlung...";
    auto model{parameters_.get<framework::config::Parameters>("model")};
    auto model_name{model.get<std::string>("name")};
    if (model_name == "vertex_library" or model_name == "g4db") {
      static const std::map<std::string, g4db::G4DarkBreMModel::ScalingMethod>
          method_lut = {
              {"forward_only",
               g4db::G4DarkBreMModel::ScalingMethod::ForwardOnly},
              {"cm_scaling", g4db::G4DarkBreMModel::ScalingMethod::CMScaling},
              {"undefined", g4db::G4DarkBreMModel::ScalingMethod::Undefined}};
      auto scaling_method_it{method_lut.find(model.get<std::string>("method"))};
      if (scaling_method_it == method_lut.end()) {
        EXCEPTION_RAISE(
            "BadConf",
            "Unrecognized scaling method '" + model.get<std::string>("method") +
                "',"
                " options are 'forward_only', 'cm_scaling', or 'undefined'.");
      }
      // Note: The process variable isn't used here, but creating the
      // G4DarkBremsstahlung object has side-effects
      process_ = std::make_unique<G4DarkBremsstrahlung>(
          std::make_shared<g4db::G4DarkBreMModel>(
              model.get<std::string>("library_path"),
              false /* dark brem off muons instead of electrons - we
                       always DB off electrons here */
              ,
              model.get<double>("threshold"), model.get<double>("epsilon"),
              scaling_method_it->second,
              g4db::G4DarkBreMModel::XsecMethod::Auto,
              model.get<double>("max_R_for_full", 50.0),
              model.get<int>("aprime_lhe_id", 1023),
              true,  // always load the library
              model.get<bool>("scale_APrime", false),
              model.get<double>("dist_decay_min", 0.0),
              model.get<double>("dist_decay_max", 1.0)),
          parameters_.get<bool>("only_one_per_event"),
          1., /* global bias - should use bias operator instead */
          parameters_.get<bool>("cache_xsec"));
      process_->RegisterStorageMechanism(storeElementZ);
    } else {
      EXCEPTION_RAISE("BadConf",
                      "Unrecognized model name '" + model_name + "'.");
    }
    ldmx_log(trace) << "Initialization of dark brem complete";
  }

  // Add A' -> fcp+ fcp- conversion process if enabled
  if (fcp_enable_) {
    ldmx_log(debug)
        << "FCP is enabled, setting up A' -> fcp+ fcp- conversion process...";
    G4ProcessManager* aprime_proc_man = G4APrime::APrime()->GetProcessManager();
    if (aprime_proc_man == nullptr) {
      ldmx_log(error) << "FATAL: Unable to access process manager for A'!";
      EXCEPTION_RAISE("APrimePhysics",
                      "Was unable to access the process manager for A', "
                      "something is very wrong!");
    }
    ldmx_log(debug) << "Creating APrimeConversionToFCPs process...";
    fcp_conversion_process_ = new APrimeConversionToFCPs();

    // Apply cross section biasing factor
    if (fcp_xsec_factor_ != 1.0) {
      fcp_conversion_process_->setCrossSecFactor(fcp_xsec_factor_);
      ldmx_log(debug) << "Applied cross section biasing factor: "
                      << fcp_xsec_factor_;
    }

    aprime_proc_man->AddDiscreteProcess(fcp_conversion_process_);
    aprime_proc_man->SetProcessOrderingToFirst(
        fcp_conversion_process_, G4ProcessVectorDoItIndex::idxAll);
    ldmx_log(info)
        << "A' -> fcp+ fcp- conversion process registered successfully!";
  } else {
    ldmx_log(info) << "FCP not enabled, A' conversion process not registered";
  }
}

}  // namespace simcore
