/**
 * @file HcalVetoProcessor.cxx
 * @brief Processor that determines if an event is vetoed by the Hcal.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Tamas Almos Vami, UCSB
 */

#include "Hcal/HcalVetoProcessor.h"

namespace hcal {

HcalVetoProcessor::HcalVetoProcessor(const std::string &name,
                                     framework::Process &process)
    : Producer(name, process) {}

void HcalVetoProcessor::configure(framework::config::Parameters &parameters) {
  total_PE_threshold_ = parameters.getParameter<double>("pe_threshold");
  max_time_ = parameters.getParameter<double>("max_time");
  output_coll_name_ = parameters.getParameter<std::string>("output_coll_name");
  input_hit_coll_name_ =
      parameters.getParameter<std::string>("input_hit_coll_name");
  input_hit_pass_name_ =
      parameters.getParameter<std::string>("input_hit_pass_name");
  track_pass_name_ = parameters.getParameter<std::string>("track_pass_name");

  // A fake-hit that gets added for the rare case where no hit actually reaches
  // the maxPE < pe check to avoid producing uninitialized memory
  //
  // Default constructed hits have nonsense-but predictable values and are
  // harder to mistake for real hits
  default_max_hit_.Clear();
  default_max_hit_.setPE(-9999);
  default_max_hit_.setMinPE(-9999);
  default_max_hit_.setSection(-9999);
  default_max_hit_.setLayer(-9999);
  default_max_hit_.setStrip(-9999);
  default_max_hit_.setEnd(-999);
  default_max_hit_.setTimeDiff(-9999);
  default_max_hit_.setToaPos(-9999);
  default_max_hit_.setToaNeg(-9999);
  default_max_hit_.setAmplitudePos(-9999);
  default_max_hit_.setAmplitudeNeg(-9999);

  double max_depth_ = parameters.getParameter<double>("max_depth", 0.);
  if (max_depth_ != 0.) {
    EXCEPTION_RAISE(
        "InvalidParam",
        "Earlier versions of the Hcal veto defined a max depth for "
        "positions which is no longer implemented. Remove the "
        "parameter (max_depth) from your configuration. See "
        "https://github.com/LDMX-Software/Hcal/issues/61 for details");
  }
  back_min_PE_ = parameters.getParameter<double>("back_min_pe");
  exclude_recoil_ele_ =
      parameters.getParameter<bool>("exclude_recoil_ele", false);
  track_collection_ =
      parameters.getParameter<std::string>("track_collection", "RecoilTracks");
  dr_from_recoil_max_ =
      parameters.getParameter<double>("dr_from_recoil_max", 100);
  inverse_skim_ = parameters.getParameter<bool>("inverse_skim");
}

void HcalVetoProcessor::produce(framework::Event &event) {
  // Get the collection of sim particles from the event
  const std::vector<ldmx::HcalHit> hcal_rec_hits =
      event.getCollection<ldmx::HcalHit>(input_hit_coll_name_,
                                         input_hit_pass_name_);

  // Where deos the recoil electron end up in the HCAL?
  float recoil_pos_x{0.0};
  float recoil_pos_y{0.0};
  float recoil_pos_z{-9999.0};
  float recoil_mom_x{-9999.0};
  float recoil_mom_y{-9999.0};
  float recoil_mom_z{-9999.0};

  if (exclude_recoil_ele_) {
    std::vector<float> recoil_track_states;
    // Get the recoil track collection
    auto recoil_tracks{
        event.getCollection<ldmx::Track>(track_collection_, track_pass_name_)};

    // Use ACTS to propage the recoil track to the end of the magnetic field
    // This happens to be at the ECAL face
    ldmx::TrackStateType ts_type = ldmx::TrackStateType::AtECAL;
    recoil_track_states = trackProp(recoil_tracks, ts_type, "ecal");
    if (!recoil_track_states.empty()) {
      recoil_pos_x = recoil_track_states[0];
      recoil_pos_y = recoil_track_states[1];
      recoil_pos_z = recoil_track_states[2];
      recoil_mom_x = recoil_track_states[3];
      recoil_mom_y = recoil_track_states[4];
      recoil_mom_z = recoil_track_states[5];
    }
  }

  // Loop over all of the Hcal hits and calculate to total photoelectrons
  // in the event.
  float total_PE{0.0};
  float max_PE{-1000};
  int num_total_hits{0};
  int num_valid_hits{0};
  int num_non_recoil_hits{0};

  const ldmx::HcalHit *max_PE_hit{&default_max_hit_};
  for (const ldmx::HcalHit &hcal_hit : hcal_rec_hits) {
    num_total_hits++;
    // If the hit time is outside the readout window, don't consider it.
    if (hcal_hit.getTime() >= max_time_) {
      continue;
    }

    // Get the total PE in the bar
    float pe = hcal_hit.getPE();
    // Keep track of the total PE
    total_PE += pe;

    // Check that both sides of the bar have a PE value above threshold.
    // If not, don't consider the hit.  Double sided readout is only
    // being used for the back HCal bars.  For the side HCal, just
    // use the maximum PE as before.
    ldmx::HcalID id(hcal_hit.getID());
    if ((id.section() == ldmx::HcalID::BACK) &&
        (hcal_hit.getMinPE() < back_min_PE_))
      continue;

    num_valid_hits++;

    // Max PE should not be caused by the recoil ele
    if (exclude_recoil_ele_) {
      // Get the position of this hit
      auto hit_pos_x = hcal_hit.getXPos();
      auto hit_pos_y = hcal_hit.getYPos();
      auto hit_pos_z = hcal_hit.getZPos();
      auto dZ = recoil_pos_z - hit_pos_z;

      auto drift_recoil_x = (dZ * (recoil_mom_x / recoil_mom_z)) + recoil_pos_x;
      auto drift_recoil_y = (dZ * (recoil_mom_y / recoil_mom_z)) + recoil_pos_y;
      auto dx = drift_recoil_x - hit_pos_x;
      auto dy = drift_recoil_y - hit_pos_y;
      auto dR_squared = dx * dx + dy * dy + dZ * dZ;
      auto dR = sqrt(dR_squared);
      ldmx_log(debug) << "    This hit is at " << hit_pos_x << " / "
                      << hit_pos_y << " /  " << hit_pos_z << " mm";
      ldmx_log(debug) << "    Ele is projected at " << drift_recoil_x << " / "
                      << drift_recoil_y << " /  " << recoil_pos_z - dZ;
      ldmx_log(debug) << "    from " << recoil_pos_x << " / " << recoil_pos_y
                      << " / " << recoil_pos_z << " / " << " mm";

      ldmx_log(debug) << "       Ele had momentum of  " << recoil_mom_x << " / "
                      << recoil_mom_y << " /  " << recoil_mom_z << " MeV";
      ldmx_log(debug) << "    This hit has PE = " << pe
                      << " and dR from ele = " << "is " << dR << " mm";

      // Dont consider this hit for max PE hit if it's too close to the recoil
      // electron trajectory
      if (dR < dr_from_recoil_max_) {
        continue;
      }
    }
    num_non_recoil_hits++;

    // Find the maximum PE in the list
    if (max_PE < pe) {
      max_PE = pe;
      max_PE_hit = &hcal_hit;
    }
  }

  ldmx_log(info) << "There are " << num_valid_hits << " / " << num_total_hits
                 << " HCal hits read out. " << num_non_recoil_hits
                 << " are not associated with the recoil ele. Total PE of "
                 << total_PE;
  // If the maximum PE found is below threshold, it passes the veto.
  bool passes_veto = (max_PE < total_PE_threshold_);
  ldmx_log(info) << "HCAL veto passed? " << passes_veto;

  ldmx::HcalVetoResult result;
  result.setVetoResult(passes_veto);
  result.setMaxPEHit(*max_PE_hit);
  result.setTotalPE(total_PE);
  result.setNumValidHits(num_valid_hits);

  // Skimming rules
  if (!inverse_skim_) {
    if (passes_veto) {
      setStorageHint(framework::hint_shouldKeep);
    } else {
      setStorageHint(framework::hint_shouldDrop);
    }
  } else {
    // Inverse skimming rules
    if (passes_veto) {
      setStorageHint(framework::hint_shouldDrop);
    } else {
      setStorageHint(framework::hint_shouldKeep);
    }
  }

  event.add(output_coll_name_, result);
}

std::vector<float> HcalVetoProcessor::trackProp(const ldmx::Tracks &tracks,
                                                ldmx::TrackStateType ts_type,
                                                const std::string &ts_title) {
  // Vector to hold the new track state variables
  std::vector<float> new_track_states;

  // Return if no tracks
  if (tracks.empty()) return new_track_states;

  // Otherwise loop on the tracks
  for (auto &track : tracks) {
    // Get track state for ts_type
    auto trk_ts = track.getTrackState(ts_type);
    // Continue if there's no value
    if (!trk_ts.has_value()) continue;
    ldmx::Track::TrackState &hcal_track_state = trk_ts.value();

    // Check that the track state is filled
    if (hcal_track_state.params.size() < 5) continue;

    float track_state_loc0 = static_cast<float>(hcal_track_state.params[0]);
    float track_state_loc1 = static_cast<float>(hcal_track_state.params[1]);

    // param 2 = phi (azimuthal), param 3 = theta (polar)
    // param 4 = QoP
    // ACTS (local)  to  LDMX (global) coordinates: (y,z,x)->  (x,y,z)
    // convert qop [1/GeV] to p [MeV]
    double p_track_state = (-1 / hcal_track_state.params[4]) * 1000;
    // p * sin(theta) * sin(phi)
    double recoil_mom_x = p_track_state * sin(hcal_track_state.params[3]) *
                          sin(hcal_track_state.params[2]);
    // p * cos(theta)
    double recoil_mom_y = p_track_state * cos(hcal_track_state.params[3]);
    // p * sin(theta) * cos(phi)
    double recoil_mom_z = p_track_state * sin(hcal_track_state.params[3]) *
                          cos(hcal_track_state.params[2]);

    // Store the new track state variables
    new_track_states.push_back(track_state_loc0);
    new_track_states.push_back(track_state_loc1);
    // z-position as in the tracking exptrapolation
    new_track_states.push_back(240.5);
    new_track_states.push_back(recoil_mom_x);
    new_track_states.push_back(recoil_mom_y);
    new_track_states.push_back(recoil_mom_z);
    break;
  }

  return new_track_states;
}

}  // namespace hcal

DECLARE_PRODUCER(hcal::HcalVetoProcessor);
