"""Basic diagnostic plots for a StripMeasurements collection.

Reads the ROOT file written by waveforms_to_measurements.py and produces a small
set of PNGs: per-layer beam-spot occupancy, local-U distributions, cluster
amplitude and cluster size, and the global-position spread that shows the two
recoil stations.

Coordinate note: the Measurement's global position is stored in the Acts frame
ordering, so component 0 is the beam axis (the two stations sit at ~ -243/-237
and -143/-137 mm) and components 1,2 are the in-plane coordinates.

Usage
-----
    denv_workspace="$PWD" denv fire Tracking/exampleConfigs/plot_measurements.py \
        -- [--input meas.root] [--outdir plots_meas]
"""

import argparse
import os
import sys

sys.argv = [a for a in sys.argv if a != "--"]

parser = argparse.ArgumentParser(f"ldmx fire {sys.argv[0]}")
parser.add_argument("--input", default="meas.root")
parser.add_argument("--outdir", default="plots_meas")
parser.add_argument("--tree", default="LDMX_Events")
parser.add_argument("--branch", default="StripMeasurements_trackerReco")
arg = parser.parse_args()

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import ROOT

ROOT.gSystem.Load("libTracking_Event")

os.makedirs(arg.outdir, exist_ok=True)

f = ROOT.TFile.Open(arg.input)
t = f.Get(arg.tree)
meas = ROOT.std.vector("ldmx::Measurement")()
t.SetBranchAddress(arg.branch, meas)

# Collect per-layer arrays.
layers = {}  # layer_id -> dict of lists
n_per_event = []
for i in range(t.GetEntries()):
    t.GetEntry(i)
    n_per_event.append(meas.size())
    for m in meas:
        lid = m.getLayerID()
        d = layers.setdefault(
            lid, dict(gx=[], gy=[], gz=[], u=[], amp=[], nstrips=[], t=[])
        )
        g = m.getGlobalPosition()
        lp = m.getLocalPosition()
        d["gx"].append(g[0])
        d["gy"].append(g[1])
        d["gz"].append(g[2])
        d["u"].append(lp[0])
        d["amp"].append(m.getClusterAmplitude())
        d["nstrips"].append(m.getNStrips())
        d["t"].append(m.getTime())

layer_ids = sorted(layers)
total = sum(len(layers[l]["u"]) for l in layer_ids)
print(f"{arg.input}: {t.GetEntries()} events, {total} measurements, "
      f"layers {layer_ids}")
for lid in layer_ids:
    d = layers[lid]
    print(f"  layer {lid}: {len(d['u']):5d} meas, "
          f"beam-pos {np.mean(d['gx']):.2f} mm")

colors = dict(zip(layer_ids, plt.cm.viridis(np.linspace(0.1, 0.85, len(layer_ids)))))


def save(fig, name):
    path = os.path.join(arg.outdir, name)
    fig.tight_layout()
    fig.savefig(path, dpi=110)
    plt.close(fig)
    print("wrote", path)


# 1. Global in-plane occupancy (beam spot) per layer.
fig, axes = plt.subplots(1, len(layer_ids), figsize=(4 * len(layer_ids), 4),
                         squeeze=False)
for ax, lid in zip(axes[0], layer_ids):
    d = layers[lid]
    ax.scatter(d["gy"], d["gz"], s=6, alpha=0.4, color=colors[lid])
    ax.set_title(f"layer {lid}  (beam {np.mean(d['gx']):.1f} mm)")
    ax.set_xlabel("global y [mm]")
    ax.set_ylabel("global z [mm]")
    ax.set_aspect("equal", "datalim")
save(fig, "occupancy_global.png")

# 2. Local-U distributions.
fig, ax = plt.subplots(figsize=(7, 4.5))
for lid in layer_ids:
    ax.hist(layers[lid]["u"], bins=60, histtype="step", label=f"layer {lid}",
            color=colors[lid])
ax.set_xlabel("local U [mm]")
ax.set_ylabel("measurements")
ax.set_title("Cluster local-U position per layer")
ax.legend()
save(fig, "local_u.png")

# 3. Cluster amplitude.
fig, ax = plt.subplots(figsize=(7, 4.5))
amp_max = max(max(layers[l]["amp"]) for l in layer_ids)
bins = np.linspace(0, amp_max, 60)
for lid in layer_ids:
    ax.hist(layers[lid]["amp"], bins=bins, histtype="step", label=f"layer {lid}",
            color=colors[lid])
ax.set_xlabel("cluster amplitude [ADC]")
ax.set_ylabel("measurements")
ax.set_title("Cluster amplitude per layer")
ax.legend()
save(fig, "cluster_amplitude.png")

# 4. Cluster size.
fig, ax = plt.subplots(figsize=(7, 4.5))
smax = max(max(layers[l]["nstrips"]) for l in layer_ids)
bins = np.arange(0.5, smax + 1.5, 1)
for lid in layer_ids:
    ax.hist(layers[lid]["nstrips"], bins=bins, histtype="step",
            label=f"layer {lid}", color=colors[lid])
ax.set_xlabel("strips per cluster")
ax.set_ylabel("measurements")
ax.set_title("Cluster size per layer")
ax.legend()
save(fig, "cluster_size.png")

# 5. Global beam-axis positions (shows the two stations + axial/stereo split).
fig, ax = plt.subplots(figsize=(7, 4.5))
all_gx = [gx for lid in layer_ids for gx in layers[lid]["gx"]]
ax.hist(all_gx, bins=120, color="0.3")
ax.set_xlabel("global beam-axis position [mm]")
ax.set_ylabel("measurements")
ax.set_title("Measurement beam-axis position (two recoil stations)")
save(fig, "beam_axis_positions.png")

# 6. Measurements per event.
fig, ax = plt.subplots(figsize=(7, 4.5))
ax.hist(n_per_event, bins=np.arange(-0.5, max(n_per_event) + 1.5, 1),
        color="0.3")
ax.set_xlabel("measurements per event")
ax.set_ylabel("events")
ax.set_title(f"Measurements per event (mean {np.mean(n_per_event):.2f})")
save(fig, "meas_per_event.png")

print("done")
