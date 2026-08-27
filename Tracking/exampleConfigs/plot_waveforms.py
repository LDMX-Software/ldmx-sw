"""Plot SiStripWaveforms straight from a decode_to_waveforms.py ROOT file.

This is step 3 of the real-data tracker waveform chain:

    1. compute_pedestals.py                   -> pedestals.json
    2. decode_to_waveforms.py                 -> <output>.root (TrackerWaveforms)
    3. plot_waveforms.py       (this script)  -> PNG figures

Unlike the in-framework approach, this is a STANDALONE script: it reads the
ldmx::SiStripWaveform collection directly out of the ROOT event tree with
PyROOT (which deserializes the custom class via the Tracking Event dictionary)
and uses the object getters -- no C++ analyzer needed.  Noise is NOT stored on
the waveform; for significance it reads the pedestal JSON (the same conditions
source the framework uses) via --pedestal-file.

Produces:
  <stem>_all.png          -- multi-panel overview of the top waveforms
  <stem>_wf##_....png     -- one PNG per individual waveform
  <stem>_occupancy.png    -- hit counts vs strip number, per (FEB, hybrid)

Usage
-----
    python Tracking/exampleConfigs/plot_waveforms.py tracker_waveforms.root
    python Tracking/exampleConfigs/plot_waveforms.py tracker_waveforms.root \
        --collection TrackerWaveforms --max-waveforms 10 --output-dir plots/

Requires the ldmx-sw environment (so the Tracking Event dictionary is on the
library path) plus matplotlib + numpy.
"""

import argparse
import json
import math
import os
import re
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.ticker as mticker
import numpy as np

import ROOT

# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------
parser = argparse.ArgumentParser(
    description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
parser.add_argument("root_file", help="ROOT file from decode_to_waveforms.py")
parser.add_argument("--tree", default="LDMX_Events",
                    help="Event tree name (default: LDMX_Events)")
parser.add_argument("--collection", default="TrackerWaveforms",
                    help="SiStripWaveform collection name (default: TrackerWaveforms)")
parser.add_argument("--fitted-collection", default="FittedSiStripHits",
                    help="FittedSiStripHit collection holding the pulse fits "
                         "(default: FittedSiStripHits). The fit is no longer "
                         "stored on the waveform; it is joined back on via the "
                         "DAQ map. If the collection or the map is missing, the "
                         "fit overlay is simply omitted.")
parser.add_argument("--daq-map", default=None,
                    help="DAQ map JSON used to join fits back onto waveforms "
                         "(default: search the install tree and ./Tracking/data)")
parser.add_argument("--n-examples", type=int, default=10,
                    help="Number of example waveforms to draw individual/overview "
                         "panels for (default: 10). Summary histograms always use "
                         "ALL waveforms.")
parser.add_argument("--example-selection", choices=("random", "top"),
                    default="random",
                    help="How to pick the example waveforms: a 'random' sample "
                         "(default) or the 'top' by peak significance.")
parser.add_argument("--seed", type=int, default=12345,
                    help="Random seed for example-waveform sampling (default: 12345)")
parser.add_argument("--max-waveforms", type=int, default=None,
                    help="Deprecated alias for --n-examples")
parser.add_argument("--pedestal-file", default="pedestals.json",
                    help="Pedestal JSON (the conditions source) used to recover "
                         "per-channel noise for significance. If missing, raw ADC "
                         "is plotted and waveforms are ranked by peak amplitude.")
parser.add_argument("--output-dir", default="",
                    help="Directory for output PNGs (default: same as the ROOT file)")
parser.add_argument("--cols", type=int, default=2,
                    help="Columns in the multi-panel overview (default: 2)")
parser.add_argument("--t0-range", type=float, nargs=2, default=(150.0, 225.0),
                    metavar=("MIN", "MAX"),
                    help="x-range [ns] for the fit-t0 histogram (default: 150 225)")
parser.add_argument("--t0-bins", type=int, default=75,
                    help="Number of bins for the fit-t0 histogram (default: 75 = 1 ns)")
parser.add_argument("--t0-select", type=float, nargs=2, default=None,
                    metavar=("MIN", "MAX"),
                    help="Restrict the example-waveform panels to converged fits "
                         "with t0 in [MIN, MAX] ns (e.g. --t0-select 205 215). "
                         "Summary histograms still use ALL waveforms.")
parser.add_argument("--t0-band-split", type=float, default=200.0,
                    help="t0 [ns] separating the in-time (below) from the late "
                         "(above) band in the t0-diagnostics figure (default: 200)")
args = parser.parse_args()

# Backwards compatibility: --max-waveforms used to control the panel count.
if args.max_waveforms is not None:
    args.n_examples = args.max_waveforms

# ---------------------------------------------------------------------------
# Open the file and locate the waveform branch
# ---------------------------------------------------------------------------
# Loading the dictionary is normally automatic via ROOT's rootmap files when the
# ldmx-sw libraries are on LD_LIBRARY_PATH.  Try an explicit load as a fallback.
for lib in ("libTracking_Event", "libEvent"):
    if ROOT.gSystem.Load(lib) >= 0:
        break

tfile = ROOT.TFile.Open(args.root_file)
if not tfile or tfile.IsZombie():
    sys.exit(f"ERROR: could not open '{args.root_file}'")

tree = tfile.Get(args.tree)
if not tree:
    sys.exit(f"ERROR: tree '{args.tree}' not found in '{args.root_file}'")

# Event-bus branches are named '<collection>_<pass>'.  Match by prefix.
branch_name = None
for b in tree.GetListOfBranches():
    name = b.GetName()
    if name == args.collection or name.startswith(args.collection + "_"):
        branch_name = name
        break
if branch_name is None:
    available = ", ".join(b.GetName() for b in tree.GetListOfBranches())
    sys.exit(f"ERROR: no branch matching '{args.collection}' found.\n"
             f"       Available branches: {available}")

print(f"Reading '{branch_name}' from '{args.tree}' in {args.root_file}")


def find_branch(collection):
    """First branch named '<collection>' or '<collection>_<pass>', else None."""
    for b in tree.GetListOfBranches():
        name = b.GetName()
        if name == collection or name.startswith(collection + "_"):
            return name
    return None


fitted_branch = find_branch(args.fitted_collection)

# ---------------------------------------------------------------------------
# Scan the tree: collect top-N waveforms by peak significance + occupancy
# ---------------------------------------------------------------------------
N_STRIPS = 640  # APV25 hybrid: 5 APVs * 128 channels
CHANNELS_PER_APV = 128

# Readout timing / pulse shape, matching Tracking/Digitization/SiStripConstants.h
# (CRRC shaper, tp = 45 ns) so the overlaid curve reproduces the C++ fit.
SAMPLING_INTERVAL_NS = 25.0
PEAKING_TIME_NS = 45.0


def crrc_shape(t, tp=PEAKING_TIME_NS):
    """Peak-normalised CR-RC pulse: f(t) = (t/tp)*exp(1 - t/tp) for t > 0."""
    t = np.asarray(t, dtype=float)
    out = np.where(t > 0.0, (t / tp) * np.exp(1.0 - t / tp), 0.0)
    return out


def gaussian(x, amp, mu, sigma):
    """Unnormalised Gaussian for fitting histogram counts."""
    return amp * np.exp(-0.5 * ((x - mu) / sigma) ** 2)


def hybrid_label(feb, hybrid):
    return f"F{feb}H{hybrid}"


def pchannel(apv, channel):
    """Physical strip number within a hybrid, matching channelmap::pchannel()."""
    return (N_STRIPS - 1) - (apv * CHANNELS_PER_APV + (CHANNELS_PER_APV - 1) - channel)


def load_pedestal_noise(path):
    """Map (feb, hybrid, pchannel) -> scalar RMS noise from the pedestal JSON.

    This is the same conditions source the framework's TrackerPedestalProvider
    reads; the standalone plotter just reads it directly.  The scalar noise is
    the mean of the three per-sample noises, matching TrackerPedestals::noise().
    """
    with open(path) as fh:
        doc = json.load(fh)
    noise_map = {}
    for key, ch in doc["channels"].items():
        feb, hybrid, apv, channel = (int(x) for x in key.split(":"))
        noise = sum(ch["noise"]) / len(ch["noise"])
        noise_map[(feb, hybrid, pchannel(apv, channel))] = noise
    return noise_map


ped_noise = {}
if os.path.exists(args.pedestal_file):
    ped_noise = load_pedestal_noise(args.pedestal_file)
    print(f"Loaded noise for {len(ped_noise)} channels from '{args.pedestal_file}'")
else:
    print(f"WARNING: pedestal file '{args.pedestal_file}' not found; "
          "plotting raw ADC and ranking by peak amplitude")
have_noise = bool(ped_noise)


# ---------------------------------------------------------------------------
# DAQ map: needed to join the fit results back onto the waveforms
# ---------------------------------------------------------------------------
# The pulse fit is not stored on the SiStripWaveform; SiStripWaveformFitProcessor
# writes it to a FittedSiStripHit keyed by (layer_id, strip_id).  Applying the
# same DAQ-map strip transform the C++ uses (channelmap::stripId) turns a
# waveform's (feb, hybrid, pchannel) into that key, so the join is exact.
DAQ_MAP_CANDIDATES = [
    os.path.join(os.environ.get("LDMX_INSTALL_PREFIX", ""),
                 "data/Tracking/daqmap_esa_slice_test.json"),
    "Tracking/data/daqmap_esa_slice_test.json",
    os.path.join(os.path.dirname(os.path.abspath(__file__)),
                 "../data/daqmap_esa_slice_test.json"),
]


def load_daq_map(path):
    """Map (feb, hybrid) -> sensor record from the DAQ map JSON."""
    with open(path) as fh:
        doc = json.load(fh)
    sensors = {}
    for s in doc["sensors"]:
        sensors[(int(s["feb"]), int(s["hybrid"]))] = {
            "layer_id": int(s["layer_id"]),
            "n_strips": int(s["n_strips"]),
            "first_strip": int(s["first_strip"]),
            "reversed": bool(s["reversed"]),
        }
    return sensors


def strip_id(pch, sensor):
    """Sensor strip index for a physical channel, matching channelmap::stripId."""
    if sensor["reversed"]:
        return sensor["first_strip"] + sensor["n_strips"] - 1 - pch
    return sensor["first_strip"] + pch


daq_map = {}
if fitted_branch is not None:
    candidates = [args.daq_map] if args.daq_map else DAQ_MAP_CANDIDATES
    for cand in candidates:
        if cand and os.path.exists(cand):
            daq_map = load_daq_map(cand)
            print(f"Loaded DAQ map for {len(daq_map)} sensors from '{cand}'")
            break
    if not daq_map:
        print("WARNING: no DAQ map found; cannot join fit results onto "
              "waveforms, so the fit overlay will be omitted. Pass --daq-map.")

have_fits = bool(fitted_branch) and bool(daq_map)
if fitted_branch is None:
    print(f"WARNING: no '{args.fitted_collection}' branch; fit overlay omitted. "
          "Re-run decode_to_waveforms.py without --no-fit to produce it.")
elif have_fits:
    print(f"Joining fits from '{fitted_branch}' via the DAQ map")

NO_FIT = {"fit_converged": False, "fit_amplitude": 0.0, "fit_t0": 0.0,
          "fit_chi2": 0.0, "fit_ndf": 0}


top = []            # list of waveform record dicts
occupancy = {}      # (feb, hybrid) -> np.array(N_STRIPS) of counts

for ievt, entry in enumerate(tree):
    # Index this event's fits by the address the fit processor assigned them.
    fits = {}
    if have_fits:
        for h in getattr(entry, fitted_branch):
            fits[(int(h.getLayerID()), int(h.getStripID()))] = {
                "fit_converged": True,
                "fit_amplitude": float(h.getAmplitude()),
                "fit_t0": float(h.getT0()),
                "fit_chi2": float(h.getChi2()),
                "fit_ndf": int(h.getNDF()),
            }

    collection = getattr(entry, branch_name)
    for wf in collection:
        samples = list(wf.getSamples())
        feb = int(wf.getFebId())
        hybrid = int(wf.getHybridId())
        pch = int(wf.getPchannel())

        key = (feb, hybrid)
        counts = occupancy.setdefault(key, np.zeros(N_STRIPS, dtype=int))
        if 0 <= pch < N_STRIPS:
            counts[pch] += 1

        noise = ped_noise.get((feb, hybrid, pch), 0.0)
        peak_amp = max(samples) if samples else 0
        peak_sigma = (peak_amp / noise) if noise > 0 else 0.0

        # Join the fit back on.  A waveform with no entry either failed the fit
        # or came from a hybrid the DAQ map does not cover; either way it is
        # plotted without an overlay.
        fit = NO_FIT
        sensor = daq_map.get(key)
        if sensor is not None:
            fit = fits.get((sensor["layer_id"], strip_id(pch, sensor)), NO_FIT)

        record = {
            "event": ievt,
            "feb": feb,
            "hybrid": hybrid,
            "pchannel": pch,
            "n_triggers": int(wf.getNTriggers()),
            "noise": noise,
            "peak_amp": peak_amp,
            "peak_sigma": peak_sigma,
            "samples": samples,
            "label": hybrid_label(feb, hybrid),
        }
        record.update(fit)
        top.append(record)

tfile.Close()

# Keep ALL waveforms for the summary histograms; pick a subset for the
# example panels (random by default, or top-ranked).
all_waveforms = top
rank = "peak_sigma" if have_noise else "peak_amp"
ranked_by = "peak significance" if have_noise else "peak amplitude"

# Candidate pool for the example panels.  Optionally restrict to a fit-t0
# window so a specific timing population can be inspected in isolation.
candidate_pool = all_waveforms
t0_sel_desc = ""
if args.t0_select is not None:
    lo, hi = args.t0_select
    candidate_pool = [w for w in all_waveforms
                      if w["fit_converged"] and lo <= w["fit_t0"] <= hi]
    t0_sel_desc = f", t0 in [{lo:.0f},{hi:.0f}] ns"
    print(f"t0 window [{lo:.0f}, {hi:.0f}] ns -> {len(candidate_pool)} candidate "
          f"waveforms")

n_ex = min(args.n_examples, len(candidate_pool))
if args.example_selection == "top":
    examples = sorted(candidate_pool, key=lambda w: w[rank], reverse=True)[:n_ex]
    example_desc = f"top {n_ex} by {ranked_by}{t0_sel_desc}"
elif candidate_pool:
    rng = np.random.default_rng(args.seed)
    pick = sorted(rng.choice(len(candidate_pool), size=n_ex, replace=False))
    examples = [candidate_pool[i] for i in pick]
    example_desc = f"{n_ex} random (seed {args.seed}){t0_sel_desc}"
else:
    examples = []
    example_desc = "none"

# 'waveforms' drives the example panels below.
waveforms = examples
print(f"Found {len(all_waveforms)} waveforms; drawing {len(examples)} example "
      f"panels ({example_desc})")

stem = os.path.splitext(os.path.basename(args.root_file))[0]
out_dir = args.output_dir or os.path.dirname(os.path.abspath(args.root_file))
os.makedirs(out_dir, exist_ok=True)


def out_path(suffix):
    return os.path.join(out_dir, f"{stem}_{suffix}.png")


# ---------------------------------------------------------------------------
# Shared style
# ---------------------------------------------------------------------------
plt.rcParams.update({
    "font.family":       "serif",
    "font.size":         9,
    "axes.titlesize":    8,
    "axes.labelsize":    9,
    "xtick.labelsize":   7,
    "ytick.labelsize":   8,
    "axes.linewidth":    0.8,
    "lines.linewidth":   1.4,
    "lines.markersize":  4,
    "grid.linewidth":    0.5,
    "grid.alpha":        0.4,
    "figure.dpi":        150,
})

TRIGGER_COLOR = "#d0e8ff"
SIGNAL_COLOR  = "#1f4e8c"
ZERO_COLOR    = "#aaaaaa"
FIT_COLOR     = "#d1495b"


# ---------------------------------------------------------------------------
# Helper: draw a single waveform onto an axes object
# ---------------------------------------------------------------------------
def draw_waveform(ax, wf):
    n_trig  = wf["n_triggers"]
    use_sigma = wf["noise"] > 0
    # Plot in units of sigma when noise is known (panels comparable without
    # calibration), otherwise fall back to raw pedestal-subtracted ADC.
    scale   = wf["noise"] if use_sigma else 1.0
    samples = np.array(wf["samples"], dtype=float) / scale
    xs      = np.arange(len(samples))

    for t in range(n_trig):
        if t % 2 == 0:
            ax.axvspan(t * 3 - 0.5, t * 3 + 2.5,
                       color=TRIGGER_COLOR, alpha=0.35, linewidth=0)

    ax.axhline(0, color=ZERO_COLOR, linewidth=0.8, linestyle="--", zorder=1)

    ax.plot(xs, samples, color=SIGNAL_COLOR, marker="o",
            markerfacecolor="white", markeredgecolor=SIGNAL_COLOR,
            markeredgewidth=1.0, zorder=3, label="data")

    # Overlay the stored pulse-shape fit: curve = A * f(t_i - T), with the
    # sample index i mapped to time t = i * 25 ns (same frame as the C++ fit).
    if wf.get("fit_converged"):
        x_fine = np.linspace(0.0, len(samples) - 1, 400)
        t_fine = x_fine * SAMPLING_INTERVAL_NS
        curve = wf["fit_amplitude"] * crrc_shape(t_fine - wf["fit_t0"])
        ax.plot(x_fine, curve / scale, color=FIT_COLOR, linewidth=1.6,
                zorder=4, label="CR-RC fit")
        # Mark the fitted peak position (t = T + tp) in sample-index units.
        peak_x = (wf["fit_t0"] + PEAKING_TIME_NS) / SAMPLING_INTERVAL_NS
        ax.axvline(peak_x, color=FIT_COLOR, linewidth=0.8, linestyle="--",
                   alpha=0.6, zorder=2)

    for t in range(1, n_trig):
        ax.axvline(t * 3 - 0.5, color="#888888", linewidth=0.5,
                   linestyle=":", zorder=2)

    trigger_starts = [t * 3 for t in range(n_trig)]
    ax.set_xticks(trigger_starts)
    ax.set_xticklabels([f"T{t}" for t in range(n_trig)])
    ax.xaxis.set_minor_locator(mticker.MultipleLocator(1))
    ax.set_xlim(-0.7, len(samples) - 0.3)

    ymax = max(abs(samples.max()), abs(samples.min()), 1.0)
    ax.set_ylim(-0.3 * ymax, 1.15 * ymax)
    ax.yaxis.set_major_locator(mticker.MaxNLocator(5, integer=False))

    ax.set_xlabel("Trigger (3 APV25 samples each)", labelpad=2)
    ax.set_ylabel("Signal / noise  (sigma)" if use_sigma else "ADC (ped-subtracted)",
                  labelpad=3)

    if use_sigma:
        peak_str = (f"peak {wf['peak_sigma']:.1f} sigma  |  "
                    f"noise {wf['noise']:.1f} ADC")
    else:
        peak_str = f"peak {wf['peak_amp']} ADC"
    title = (f"{wf['label']}  strip {wf['pchannel']}  |  evt {wf['event']}  |  "
             + peak_str)
    ax.set_title(title, pad=4)
    ax.grid(axis="y", which="major", zorder=0)

    if wf.get("fit_converged"):
        ndf = wf["fit_ndf"]
        chi2ndf = wf["fit_chi2"] / ndf if ndf > 0 else float("nan")
        fit_txt = (f"fit: t0={wf['fit_t0']:.0f} ns\n"
                   f"amp={wf['fit_amplitude']:.0f} ADC\n"
                   f"$\\chi^2$/ndf={chi2ndf:.2f}")
        ax.text(0.97, 0.95, fit_txt, transform=ax.transAxes, ha="right",
                va="top", fontsize=6.5, color=FIT_COLOR,
                bbox=dict(boxstyle="round,pad=0.3", fc="white", ec=FIT_COLOR,
                          alpha=0.85, linewidth=0.6))
    ax.legend(loc="upper left", fontsize=6.5, framealpha=0.85)


# ---------------------------------------------------------------------------
# 1. Multi-panel overview
# ---------------------------------------------------------------------------
if waveforms:
    ncols = args.cols
    nrows = math.ceil(len(waveforms) / ncols)
    fig, axes = plt.subplots(nrows, ncols,
                             figsize=(ncols * 4.2, nrows * 3.0),
                             constrained_layout=True)
    if nrows == 1 and ncols == 1:
        axes = [axes]
    else:
        axes = list(np.array(axes).flat)

    for wf, ax in zip(waveforms, axes):
        draw_waveform(ax, wf)
    for ax in axes[len(waveforms):]:
        ax.set_visible(False)

    fig.suptitle(f"Example waveforms ({example_desc})", fontsize=10, y=1.01)
    path = out_path("all")
    fig.savefig(path, dpi=300, bbox_inches="tight")
    plt.close(fig)
    print(f"Overview  -> {path}")

    # -----------------------------------------------------------------------
    # 2. Individual waveform PNGs
    # -----------------------------------------------------------------------
    for idx, wf in enumerate(waveforms):
        label_safe = re.sub(r"\s+", "", wf["label"])
        fname = f"wf{idx:02d}_evt{wf['event']:04d}_{label_safe}_strip{wf['pchannel']:03d}"
        fig_i, ax_i = plt.subplots(figsize=(6.0, 3.5), constrained_layout=True)
        draw_waveform(ax_i, wf)
        path = out_path(fname)
        fig_i.savefig(path, dpi=300, bbox_inches="tight")
        plt.close(fig_i)
        print(f"Waveform  -> {path}")

# ---------------------------------------------------------------------------
# 3. Fit summary: 1-D histograms + vs-strip profiles (over ALL waveforms)
# ---------------------------------------------------------------------------
conv = [w for w in all_waveforms if w["fit_converged"] and w["fit_ndf"] > 0]
if conv:
    t0    = np.array([w["fit_t0"] for w in conv])
    amp   = np.array([w["fit_amplitude"] for w in conv])
    chi2n = np.array([w["fit_chi2"] / w["fit_ndf"] for w in conv])
    strip = np.array([w["pchannel"] for w in conv])
    keys  = [(w["feb"], w["hybrid"]) for w in conv]
    psig  = np.array([w["peak_sigma"] for w in conv])
    has_sig = have_noise and any(w["noise"] > 0 for w in conv)

    # Consistent color per (feb, hybrid) across all summary plots.
    uniq = sorted(set(keys))
    cmap = plt.get_cmap("tab10")
    hyb_color = {k: cmap(i % 10) for i, k in enumerate(uniq)}
    point_colors = [hyb_color[k] for k in keys]

    # --- 3a. 1-D histograms ---
    def hist1d(ax, data, xlabel, color, bins=30, hist_range=None, gauss=False):
        if len(data) == 0:
            ax.set_visible(False)
            return
        # Stats reflect the displayed window when a range is given.
        shown = data
        if hist_range is not None:
            shown = data[(data >= hist_range[0]) & (data <= hist_range[1])]
        counts, edges, _ = ax.hist(data, bins=bins, range=hist_range,
                                   color=color, edgecolor="white", linewidth=0.4)
        ax.set_xlabel(xlabel, labelpad=2)
        ax.set_ylabel("waveforms", labelpad=2)
        ax.grid(axis="y", alpha=0.4, zorder=0)
        if hist_range is not None:
            ax.set_xlim(*hist_range)

        stats = (f"N={len(shown)}\nmean={np.mean(shown):.1f}\n"
                 f"std={np.std(shown):.1f}")

        if gauss and len(shown) > 5:
            centers = 0.5 * (edges[:-1] + edges[1:])
            p0 = [counts.max(), np.mean(shown), max(np.std(shown), 1.0)]
            try:
                from scipy.optimize import curve_fit
                popt, _ = curve_fit(gaussian, centers, counts, p0=p0)
                amp_f, mu_f, sig_f = popt[0], popt[1], abs(popt[2])
                fit_kind = "Gauss fit"
            except Exception:
                # Fall back to a moment-based Gaussian if scipy/fit unavailable.
                amp_f, mu_f, sig_f = p0
                fit_kind = "Gauss (moments)"
            xfit = np.linspace(edges[0], edges[-1], 400)
            ax.plot(xfit, gaussian(xfit, amp_f, mu_f, sig_f),
                    color="#c1121f", linewidth=1.8, zorder=5)
            stats += f"\n\n{fit_kind}:\n$\\mu$={mu_f:.1f} ns\n$\\sigma$={sig_f:.1f} ns"

        ax.text(0.97, 0.95, stats, transform=ax.transAxes, ha="right",
                va="top", fontsize=7,
                bbox=dict(boxstyle="round,pad=0.3", fc="white", ec="#888888",
                          alpha=0.85, linewidth=0.5))

    fig_h, axh = plt.subplots(2, 2, figsize=(9.0, 6.0), constrained_layout=True)
    axh = axh.flat
    hist1d(axh[0], t0,    "fit t0  [ns]",          "#1f4e8c",
           bins=args.t0_bins, hist_range=tuple(args.t0_range), gauss=True)
    hist1d(axh[1], amp,   "fit amplitude  [ADC]",  "#2a9d8f")
    hist1d(axh[2], chi2n, r"fit $\chi^2$/ndf",     "#e9c46a")
    if has_sig:
        hist1d(axh[3], psig, r"peak significance  [$\sigma$]", "#d1495b")
    else:
        axh[3].set_visible(False)

    fig_h.suptitle(f"Pulse-fit summary  (N={len(conv)} converged fits)",
                   fontsize=11)
    path = out_path("fit_summary")
    fig_h.savefig(path, dpi=200, bbox_inches="tight")
    plt.close(fig_h)
    print(f"Fit hist  -> {path}")

    # --- 3b. vs-strip profiles + timing walk ---
    fig_p, axp = plt.subplots(1, 3, figsize=(13.0, 3.8), constrained_layout=True)

    axp[0].scatter(strip, t0, c=point_colors, s=14, edgecolor="none", alpha=0.8)
    axp[0].set(xlabel="Strip (pchannel)", ylabel="fit t0  [ns]",
               xlim=(-5, N_STRIPS + 4), title="Hit time vs strip")

    axp[1].scatter(strip, amp, c=point_colors, s=14, edgecolor="none", alpha=0.8)
    axp[1].set(xlabel="Strip (pchannel)", ylabel="fit amplitude  [ADC]",
               xlim=(-5, N_STRIPS + 4), title="Amplitude vs strip")

    axp[2].scatter(t0, amp, c=point_colors, s=14, edgecolor="none", alpha=0.8)
    axp[2].set(xlabel="fit t0  [ns]", ylabel="fit amplitude  [ADC]",
               title="Amplitude vs t0 (timing walk)")

    for ax in axp:
        ax.grid(alpha=0.3, zorder=0)

    handles = [plt.Line2D([], [], marker="o", linestyle="", markersize=5,
                          markerfacecolor=hyb_color[k], markeredgecolor="none",
                          label=hybrid_label(*k)) for k in uniq]
    fig_p.legend(handles=handles, loc="upper right", fontsize=7, framealpha=0.9,
                 ncol=max(1, len(uniq) // 2))

    path = out_path("fit_profiles")
    fig_p.savefig(path, dpi=200, bbox_inches="tight")
    plt.close(fig_p)
    print(f"Fit prof  -> {path}")

    # --- 3c. t0-population diagnostics: is the late band hybrid/event/strip
    #         localised, or a uniform ~one-sample-late copy of the bulk? ---
    ev       = np.array([w["event"] for w in conv])
    lo, hi   = args.t0_range
    split    = args.t0_band_split
    keys_idx = np.array([uniq.index(k) for k in keys])
    in_time  = (t0 >= lo) & (t0 < split)
    late     = (t0 >= split) & (t0 <= hi)

    frac = 100.0 * late.sum() / max(1, in_time.sum() + late.sum())
    bins_s = np.arange(0, N_STRIPS + 1)

    def make_diag(logy):
        """Build the 4-panel diagnostics figure; top-left t0 panel optionally
        log-y so the small late peak is visible."""
        fig_d, axd = plt.subplots(2, 2, figsize=(12.0, 8.5),
                                  constrained_layout=True)

        # (A) t0 stacked by hybrid -- which hybrids feed the late bump?
        axd[0, 0].hist([t0[keys_idx == i] for i in range(len(uniq))],
                       bins=args.t0_bins, range=(lo, hi), stacked=True,
                       color=[hyb_color[k] for k in uniq],
                       label=[hybrid_label(*k) for k in uniq],
                       edgecolor="white", linewidth=0.15)
        scale_tag = " [log y]" if logy else ""
        axd[0, 0].set(xlabel="fit t0  [ns]", ylabel="waveforms", xlim=(lo, hi),
                      title=f"t0 by hybrid (stacked){scale_tag}")
        if logy:
            axd[0, 0].set_yscale("log")
            axd[0, 0].set_ylim(bottom=0.5)
        axd[0, 0].legend(fontsize=6, ncol=2, framealpha=0.9)
        axd[0, 0].grid(axis="y", alpha=0.3, zorder=0)

        # (B) 2-D t0 vs hybrid -- localisation as a heatmap.
        h2 = axd[0, 1].hist2d(t0, keys_idx, bins=[args.t0_bins, len(uniq)],
                              range=[(lo, hi), (-0.5, len(uniq) - 0.5)],
                              cmap="viridis")
        axd[0, 1].set_yticks(range(len(uniq)))
        axd[0, 1].set_yticklabels([hybrid_label(*k) for k in uniq], fontsize=7)
        axd[0, 1].set(xlabel="fit t0  [ns]", title="t0 vs hybrid")
        fig_d.colorbar(h2[3], ax=axd[0, 1], label="waveforms")

        # (C) t0 vs event -- is the late band time-localised within the run?
        axd[1, 0].scatter(ev, t0, c=point_colors, s=10, alpha=0.6,
                          edgecolor="none")
        axd[1, 0].set(xlabel="event index", ylabel="fit t0  [ns]", ylim=(lo, hi),
                      title="t0 vs event")
        axd[1, 0].grid(alpha=0.3, zorder=0)

        # (D) occupancy split by t0 band -- do late hits sit on specific strips?
        axd[1, 1].hist(strip[in_time], bins=bins_s, histtype="step",
                       linewidth=1.2, color="#1f4e8c",
                       label=f"in-time [{lo:.0f},{split:.0f}) ns  "
                             f"N={int(in_time.sum())}")
        axd[1, 1].hist(strip[late], bins=bins_s, histtype="step", linewidth=1.2,
                       color="#d1495b",
                       label=f"late [{split:.0f},{hi:.0f}] ns  "
                             f"N={int(late.sum())}")
        axd[1, 1].set(xlabel="strip (pchannel)", ylabel="waveforms",
                      xlim=(-5, N_STRIPS + 4), title="Occupancy by t0 band")
        if logy:
            axd[1, 1].set_yscale("log")
            axd[1, 1].set_ylim(bottom=0.5)
        axd[1, 1].xaxis.set_major_locator(mticker.MultipleLocator(64))
        axd[1, 1].legend(fontsize=7, framealpha=0.9)
        axd[1, 1].grid(axis="y", alpha=0.3, zorder=0)

        fig_d.suptitle(f"t0-population diagnostics  (late band = {frac:.1f}% of "
                       f"in-time+late, split at {split:.0f} ns)", fontsize=11)
        suffix = "t0_diagnostics_logy" if logy else "t0_diagnostics"
        path = out_path(suffix)
        fig_d.savefig(path, dpi=200, bbox_inches="tight")
        plt.close(fig_d)
        print(f"t0 diag   -> {path}")

    make_diag(logy=False)
    make_diag(logy=True)
else:
    print("No converged fits found; skipping fit summary plots")

# ---------------------------------------------------------------------------
# 4. Occupancy: hits vs strip number, all hybrids on one plot
# ---------------------------------------------------------------------------
if occupancy:
    fig_o, ax_o = plt.subplots(figsize=(8.0, 3.8), constrained_layout=True)

    for (feb, hybrid), counts in sorted(occupancy.items()):
        strips = np.arange(len(counts))
        ax_o.step(strips, counts, where="mid", linewidth=1.2,
                  label=hybrid_label(feb, hybrid))

    ax_o.set_xlabel("Strip (pchannel)", labelpad=3)
    ax_o.set_ylabel("Hits (above threshold)", labelpad=3)
    ax_o.set_xlim(-5, N_STRIPS + 4)
    ax_o.set_ylim(bottom=0)
    ax_o.xaxis.set_major_locator(mticker.MultipleLocator(64))
    ax_o.xaxis.set_minor_locator(mticker.MultipleLocator(16))
    ax_o.grid(axis="y", which="major", zorder=0)
    ax_o.legend(framealpha=0.9, fontsize=8)
    ax_o.set_title("Strip occupancy (threshold-passing waveforms)", pad=4)

    path = out_path("occupancy")
    fig_o.savefig(path, dpi=300, bbox_inches="tight")
    plt.close(fig_o)
    print(f"Occupancy -> {path}")
