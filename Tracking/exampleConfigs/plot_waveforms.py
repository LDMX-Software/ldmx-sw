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
parser.add_argument("--max-waveforms", type=int, default=10,
                    help="Number of highest-significance waveforms to plot (default: 10)")
parser.add_argument("--pedestal-file", default="pedestals.json",
                    help="Pedestal JSON (the conditions source) used to recover "
                         "per-channel noise for significance. If missing, raw ADC "
                         "is plotted and waveforms are ranked by peak amplitude.")
parser.add_argument("--output-dir", default="",
                    help="Directory for output PNGs (default: same as the ROOT file)")
parser.add_argument("--cols", type=int, default=2,
                    help="Columns in the multi-panel overview (default: 2)")
args = parser.parse_args()

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

# ---------------------------------------------------------------------------
# Scan the tree: collect top-N waveforms by peak significance + occupancy
# ---------------------------------------------------------------------------
N_STRIPS = 640  # APV25 hybrid: 5 APVs * 128 channels
CHANNELS_PER_APV = 128


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


top = []            # list of waveform record dicts
occupancy = {}      # (feb, hybrid) -> np.array(N_STRIPS) of counts

for ievt, entry in enumerate(tree):
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

        top.append({
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
        })

tfile.Close()

# Rank by significance when noise is available, otherwise by raw peak amplitude.
rank = "peak_sigma" if have_noise else "peak_amp"
top.sort(key=lambda w: w[rank], reverse=True)
waveforms = top[:args.max_waveforms]

ranked_by = "peak significance" if have_noise else "peak amplitude"
print(f"Found {len(top)} waveforms; plotting the top {len(waveforms)} by {ranked_by}")

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
            markeredgewidth=1.0, zorder=3)

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

    fig.suptitle(f"Top waveforms by {ranked_by}", fontsize=10, y=1.01)
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
# 3. Occupancy: hits vs strip number, all hybrids on one plot
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
