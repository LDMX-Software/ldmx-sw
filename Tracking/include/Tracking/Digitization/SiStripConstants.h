#pragma once

#include <string_view>

namespace tracking::digitization {

// ---------------------------------------------------------------------------
// Physical constant
// ---------------------------------------------------------------------------

/// Energy to create one electron-hole pair in silicon [MeV].
/// (3.62 eV / pair × 1e-6 MeV/eV)
inline constexpr double ENERGY_PER_EHP_MEV = 3.62e-6;

// ---------------------------------------------------------------------------
// Sensor geometry
// ---------------------------------------------------------------------------

/// Sense (inner-electrode) strip pitch [mm].
inline constexpr double SENSE_PITCH_MM = 0.030;
/// Readout strip pitch [mm]  (= 2 × SENSE_PITCH_MM).
inline constexpr double READOUT_PITCH_MM = 0.060;
/// Total number of readout strips (strip 0 = most-negative U edge).
inline constexpr int N_READOUT_STRIPS = 767;
/// V-coordinate position uncertainty [mm]
/// (≈ strip half-length / √3, V is the unmeasured direction).
inline constexpr double SIGMA_V_MM = 20.0;

// ---------------------------------------------------------------------------
// Sensor operating conditions
// ---------------------------------------------------------------------------

/// Applied reverse-bias voltage [V].
inline constexpr double BIAS_VOLTAGE_V = 200.0;
/// Full-depletion voltage [V].
inline constexpr double DEPLETION_VOLTAGE_V = 70.0;
/// Operating temperature [K].
inline constexpr double TEMPERATURE_K = 300.0;

// ---------------------------------------------------------------------------
// Readout electronics and sampling
// ---------------------------------------------------------------------------

/// Pulse-shape type identifier (CRRC shaper).
inline constexpr std::string_view PULSE_SHAPE_NAME = "CRRC";
/// Shaper peaking time tp [ns].
inline constexpr double PEAKING_TIME_NS = 45.0;
/// Second time constant tp2 [ns] (FourPole shape only).
inline constexpr double SECOND_TIME_CONST_NS = 10.0;
/// ADC sampling interval [ns]  (40 MHz clock).
inline constexpr double SAMPLING_INTERVAL_NS = 25.0;
/// Number of ADC samples per strip hit.
inline constexpr int N_SAMPLES = 3;
/// ADC dynamic range [bits]  (max value = 2^ADC_BITS − 1 = 16383).
inline constexpr int ADC_BITS = 14;
/// ADC pedestal [counts].
inline constexpr int ADC_PEDESTAL = 0;
/// Time of sample 0 relative to the hit frame [ns].
inline constexpr double T0_OFFSET_NS = 0.0;

// ---------------------------------------------------------------------------
// Front-end gain and charge-to-ADC conversion (HPS SVT APV25 configuration)
//
// amplitude [ADC] = (charge / MIP_ELECTRONS)
//                 * READOUT_RESISTOR_OHMS * READOUT_INPUT_STAGE_GAIN
//                 * 2^ADC_BITS / ADC_FULL_SCALE_MV
// ---------------------------------------------------------------------------

/// MIP charge [e⁻] for 320 µm silicon (HPS value).
inline constexpr double MIP_ELECTRONS = 25000.0;
/// Shaper feedback resistor [Ω].
inline constexpr double READOUT_RESISTOR_OHMS = 100.0;
/// Input stage gain.
inline constexpr double READOUT_INPUT_STAGE_GAIN = 1.5;
/// ADC full-scale range [mV].
inline constexpr double ADC_FULL_SCALE_MV = 2000.0;

/// Derived: charge-to-ADC conversion [e⁻ per ADC count].
/// = 25000 / (100 × 1.5 × 16384 / 2000) ≈ 20.35 e⁻/count
inline constexpr double ADC_ELECTRONS_PER_COUNT =
    MIP_ELECTRONS / (READOUT_RESISTOR_OHMS * READOUT_INPUT_STAGE_GAIN *
                     (1 << ADC_BITS) / ADC_FULL_SCALE_MV);

// ---------------------------------------------------------------------------
// Sense-to-readout charge transfer efficiencies (AC-coupled sensor)
//
// From HPS CDFSiSensorSim / HpsSiSensor.  The transfer_efficiencies matrix
// is {{readoutTransferEfficiency, senseTransferEfficiency}}.
//
// For ratio = readout_pitch / sense_pitch = 2:
//   - Paired sense strips (even index, physically under a readout strip):
//       fraction transferred = READOUT_TRANSFER_EFFICIENCY
//   - Unpaired sense strips (odd index, between two readout strips):
//       fraction transferred to EACH adjacent readout strip
//       = SENSE_TRANSFER_EFFICIENCY  (total ≈ 0.838; ~16% lost)
// ---------------------------------------------------------------------------

/// Transfer efficiency from a paired sense strip to its readout strip.
inline constexpr double READOUT_TRANSFER_EFFICIENCY = 0.986;
/// Transfer efficiency from an unpaired sense strip to each adjacent readout
/// strip.  Applied twice (left and right neighbour), so total charge
/// transferred ≈ 2 × 0.419 = 0.838.
inline constexpr double SENSE_TRANSFER_EFFICIENCY = 0.419;

// ---------------------------------------------------------------------------
// Electronics noise and readout threshold
// ---------------------------------------------------------------------------

/// Equivalent noise charge [e⁻ ENC].
inline constexpr double NOISE_ELECTRONS = 1000.0;
/// Readout threshold [e⁻]  (≈ 3× NOISE_ELECTRONS).
inline constexpr double THRESHOLD_ELECTRONS = 3000.0;

/// Derived: noise RMS [ADC counts] = NOISE_ELECTRONS / ADC_ELECTRONS_PER_COUNT.
inline constexpr double NOISE_SIGMA_ADC =
    NOISE_ELECTRONS / ADC_ELECTRONS_PER_COUNT;
/// Derived: readout threshold [ADC counts] = THRESHOLD_ELECTRONS /
/// ADC_ELECTRONS_PER_COUNT.
inline constexpr double THRESHOLD_ADC =
    THRESHOLD_ELECTRONS / ADC_ELECTRONS_PER_COUNT;

}  // namespace tracking::digitization
