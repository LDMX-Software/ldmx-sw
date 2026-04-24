#include "Tracking/Digitization/SiStripDigitizer.h"
#include "Tracking/Digitization/SiStripConstants.h"

#include <cmath>
#include <set>

namespace tracking::digitization {

// ---------------------------------------------------------------------------
// Physical constants
// ---------------------------------------------------------------------------

/// kT/q at 300 K [V].  Scale linearly with temperature.
static constexpr double KT_Q_300K = 0.025852;

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

SiStripDigitizer::SiStripDigitizer(const SensorParams& params,
                                    std::default_random_engine& generator)
    : params_(params), generator_(&generator) {}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

int SiStripDigitizer::adaptiveNSegments(const Acts::Vector3& local_dir,
                                         double path_length) const {
  // Ensure U-displacement per segment <= granularity * sense_pitch.
  // For near-normal-incidence tracks (dir_u ≈ 0) the adaptive count can be
  // very small; clamp to n_segments_min.
  const double max_step =
      params_.deposition_granularity * params_.sense_pitch;
  const double total_u = std::abs(path_length * local_dir[0]);
  const int adaptive =
      (max_step > 0.0) ? static_cast<int>(std::ceil(total_u / max_step)) : 1;
  return std::max(params_.n_segments_min, adaptive);
}

double SiStripDigitizer::diffusionSigma(double d, bool is_minority) const {
  const double kt_q  = KT_Q_300K * params_.temperature / 300.0;
  const double t     = params_.thickness;
  const double vb    = params_.bias_voltage;
  const double vd    = params_.depletion_voltage;

  double sigma_sq = 0.0;

  if (vb > vd && vd > 0.0) {
    const double cf    = 2.0 * vd * d / t;
    const double base  = kt_q * t * t / vd;

    if (is_minority) {
      // Minority carriers (e⁻ in p-type, h⁺ in n-type):
      //   σ² = (kT/q)·t²/Vd · ln( V_sum / (V_sum − cf) )
      // These carriers drift quickly through the high-field region →
      // smaller diffusion sigma.
      const double v_sum = vb + vd;
      const double denom = v_sum - cf;
      if (denom > 0.0) {
        sigma_sq = base * std::log(v_sum / denom);
      }
    } else {
      // Majority carriers (h⁺ in p-type, e⁻ in n-type):
      //   σ² = (kT/q)·t²/Vd · ln( (ΔV + cf) / ΔV )
      // These carriers drift through the low-field region →
      // larger diffusion sigma.
      const double delta_v = vb - vd;
      if (delta_v > 0.0) {
        sigma_sq = base * std::log(1.0 + cf / delta_v);
      }
    }
  } else if (vb > 0.0) {
    // Uniform-field fallback (V_bias ≤ V_dep or V_dep = 0):
    //   σ² = 2·(kT/q)·d·t / V_bias
    sigma_sq = 2.0 * kt_q * d * t / vb;
  }

  return std::sqrt(std::max(sigma_sq, 0.0));
}

double SiStripDigitizer::stripFraction(double u0, double sigma,
                                        double strip_center,
                                        double pitch) const {
  // Integral of N(u0, sigma) over [strip_center − pitch/2, strip_center + pitch/2]
  const double inv_sqrt2_sigma = 1.0 / (std::sqrt(2.0) * sigma);
  const double lo = (strip_center - 0.5 * pitch - u0) * inv_sqrt2_sigma;
  const double hi = (strip_center + 0.5 * pitch - u0) * inv_sqrt2_sigma;
  return 0.5 * (std::erf(hi) - std::erf(lo));
}

std::map<int, double> SiStripDigitizer::computeCarrierCharges(
    double q_per_seg, int n_seg, const Acts::Vector3& local_pos,
    const Acts::Vector3& local_dir, double path_length, double w_collect,
    double lorentz_tan, bool is_minority) const {

  // 1/cos²(θ_L) = 1 + tan²(θ_L): Lorentz broadening of the diffusion sigma.
  const double inv_cos2 = 1.0 + lorentz_tan * lorentz_tan;

  std::map<int, double> sense_charges;

  for (int iseg = 0; iseg < n_seg; ++iseg) {
    // Fractional position along the track: f = 0 (entry) → 1 (exit).
    const double f = (iseg + 0.5) / n_seg;

    // 3D segment centre in sensor-local coordinates [mm].
    const Acts::Vector3 seg_pos =
        local_pos + (f - 0.5) * path_length * local_dir;

    const double u_seg = seg_pos[0];
    const double w_seg = seg_pos[2];

    // Drift distance to collection electrode, clamped to sensor thickness.
    const double drift = std::max(
        0.0, std::min(params_.thickness, std::abs(w_collect - w_seg)));

    // Charge trapping: linear model from CDFSiSensorSim.
    // trapping_ = fraction lost per 100 µm (= 0.1 mm) of drift.
    // collection_efficiency = 1 − 10·trapping·drift_mm
    double q_seg = q_per_seg;
    if (params_.trapping > 0.0) {
      const double efficiency =
          std::max(0.0, std::min(1.0, 1.0 - 10.0 * params_.trapping * drift));
      q_seg *= efficiency;
    }

    // Diffusion sigma [mm] with 1/cos²(θ_L) broadening from Lorentz angle.
    const double sigma_1d = diffusionSigma(drift, is_minority);
    const double sigma =
        std::max(sigma_1d * std::sqrt(inv_cos2), 1.0e-4);  // ≥ 0.1 µm

    // Lorentz shift: carriers arrive at U = u_seg + drift · tan(θ_L).
    const double u_dest = u_seg + drift * lorentz_tan;

    // Deposit charge on sense strips within 5 sigma of the charge centroid.
    const int strip_lo = static_cast<int>(
        std::floor((u_dest - 5.0 * sigma) / params_.sense_pitch));
    const int strip_hi = static_cast<int>(
        std::ceil((u_dest + 5.0 * sigma) / params_.sense_pitch));

    for (int istrip = strip_lo; istrip <= strip_hi; ++istrip) {
      const double strip_center = istrip * params_.sense_pitch;
      const double frac =
          stripFraction(u_dest, sigma, strip_center, params_.sense_pitch);
      if (frac > 1.0e-7) {
        sense_charges[istrip] += q_seg * frac;
      }
    }
  }

  return sense_charges;
}

std::map<int, double> SiStripDigitizer::senseToReadout(
    const std::map<int, double>& sense_charges) const {
  const int ratio = std::max(
      1, static_cast<int>(
             std::round(params_.readout_pitch / params_.sense_pitch)));

  const int offset = params_.n_readout_strips / 2;
  std::map<int, double> readout_charges;

  if (ratio == 1) {
    // No interleaving: paired strip only, apply readout transfer efficiency.
    for (const auto& [sense_strip, charge] : sense_charges) {
      readout_charges[sense_strip + offset] +=
          charge * params_.readout_transfer_efficiency;
    }
    return readout_charges;
  }

  // AC-coupled sense→readout transfer following HPS CDFSiSensorSim.
  //
  // For each sense strip n, compute its position within its readout group:
  //   k                = floor(n / ratio)   — group index
  //   position_in_group = n − ratio × k     — 0 … ratio−1
  //
  // position_in_group == 0: "paired" strip, physically under readout strip r.
  //   → transfers readout_transfer_efficiency × charge to readout r.
  //
  // position_in_group > 0: "unpaired" strip, between readout strips r and r+1.
  //   → transfers sense_transfer_efficiency × charge to EACH of r and r+1.
  //   (total ≈ 2 × 0.419 = 0.838; ~16% lost to capacitive cross-talk)

  for (const auto& [sense_strip, charge] : sense_charges) {
    const int k = static_cast<int>(
        std::floor(static_cast<double>(sense_strip) / ratio));
    const int position_in_group = sense_strip - ratio * k;
    const int r = k + offset;

    if (position_in_group == 0) {
      readout_charges[r] += charge * params_.readout_transfer_efficiency;
    } else {
      readout_charges[r]     += charge * params_.sense_transfer_efficiency;
      readout_charges[r + 1] += charge * params_.sense_transfer_efficiency;
    }
  }
  return readout_charges;
}

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

std::map<int, double> SiStripDigitizer::computeStripCharges(
    double edep, const Acts::Vector3& local_pos, const Acts::Vector3& local_dir,
    double path_length) const {

  const double total_electrons = edep / ENERGY_PER_EHP_MEV;
  const int    n_seg            = adaptiveNSegments(local_dir, path_length);
  const double q_per_seg        = total_electrons / n_seg;

  // Minority-carrier flags for the two bulk types.
  // p-type bulk (is_n_type = false): electrons = minority, holes = majority.
  // n-type bulk (is_n_type = true):  holes     = minority, electrons = majority.
  const bool electron_is_minority = !params_.is_n_type;
  const bool hole_is_minority     =  params_.is_n_type;

  std::map<int, double> readout_charges;

  // Electron side: collection at W = +thickness/2 (n-strip side).
  if (params_.electron_side_readout) {
    const double w_electron = +0.5 * params_.thickness;
    auto sense = computeCarrierCharges(q_per_seg, n_seg, local_pos, local_dir,
                                       path_length, w_electron,
                                       params_.electron_lorentz_tangent,
                                       electron_is_minority);
    for (const auto& [strip, charge] : senseToReadout(sense)) {
      readout_charges[strip] += charge;
    }
  }

  // Hole side: collection at W = −thickness/2 (p-bulk / backplane side).
  if (params_.hole_side_readout) {
    const double w_hole = -0.5 * params_.thickness;
    auto sense = computeCarrierCharges(q_per_seg, n_seg, local_pos, local_dir,
                                       path_length, w_hole,
                                       params_.hole_lorentz_tangent,
                                       hole_is_minority);
    for (const auto& [strip, charge] : senseToReadout(sense)) {
      readout_charges[strip] += charge;
    }
  }

  return readout_charges;
}

void SiStripDigitizer::applyNoiseAndThreshold(
    std::map<int, double>& strip_charges) {
  if (!generator_) return;

  // Add the immediate neighbours of every signal strip so that noise alone
  // can promote them above threshold (as in a real detector where every
  // strip has readout noise).
  std::set<int> extra;
  for (const auto& [strip, charge] : strip_charges) {
    extra.insert(strip - 1);
    extra.insert(strip + 1);
  }
  for (int s : extra) {
    strip_charges.emplace(s, 0.0);  // does not overwrite existing entries
  }

  // Add Gaussian noise to all strips.
  for (auto& [strip, charge] : strip_charges) {
    charge += params_.noise_electrons * normal_(*generator_);
  }

  // Remove strips below the readout threshold.
  for (auto it = strip_charges.begin(); it != strip_charges.end();) {
    it = (it->second < params_.threshold_electrons) ? strip_charges.erase(it)
                                                    : std::next(it);
  }
}

std::pair<double, double> SiStripDigitizer::clusterToPosition(
    const std::map<int, double>& strip_charges) const {
  if (strip_charges.empty()) {
    return {0.0, params_.readout_pitch / std::sqrt(12.0)};
  }

  // Charge-weighted centroid using readout-strip centres.
  // With AC-coupled transfer efficiencies, readout strip r is anchored at the
  // position of its paired sense strip (position_in_group == 0):
  //   U = (r - N_int) * readout_pitch,  N_int = integer(N/2).
  // For N=767: offset = 383, so readout 383 → U=0, 384 → U=60 µm, etc.
  const int    n_int  = params_.n_readout_strips / 2;  // integer division = 383
  const double offset = static_cast<double>(n_int);
  double sum_q  = 0.0;
  double sum_qu = 0.0;
  for (const auto& [strip, charge] : strip_charges) {
    const double u = (strip - offset) * params_.readout_pitch;
    sum_q  += charge;
    sum_qu += charge * u;
  }

  const double centroid_u = (sum_q > 0.0) ? sum_qu / sum_q : 0.0;
  const double resolution =
      params_.readout_pitch / std::sqrt(12.0 * strip_charges.size());

  return {centroid_u, resolution};
}

}  // namespace tracking::digitization
