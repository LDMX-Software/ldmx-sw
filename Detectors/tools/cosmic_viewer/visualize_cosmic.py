#!/usr/bin/env python3
"""Render real GDML-derived material meshes and an explicitly illustrative ray.

Input scene objects are exported with export_cosmic_scene.C, or assembled from
exact primitive dimensions and positions by a provenance-recorded extraction.
No transport, response, or sensitive-detector hit simulation occurs here.
"""

import argparse
import base64
import json
from pathlib import Path

import matplotlib
import numpy as np


matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.collections import PolyCollection
from matplotlib.lines import Line2D


HERE = Path(__file__).resolve().parents[1]
PALETTE = {
    "trigger": "#d39720",
    "tracker": "#246fbc",
    "ecal": "#b74e73",
    "hcal": "#33877b",
    "target": "#7f64ae",
    "cad_hcal": "#16978e",
    "cad": "#c5cbd2",
    "other": "#aaaeb4",
}
LABELS = {
    "trigger": "Trigger scintillator",
    "tracker": "Silicon tracker",
    "ecal": "ECal silicon",
    "hcal": "HCal scintillator",
    "target": "LYSO target hodoscope",
    "cad_hcal": "CAD HCal (materials unassigned)",
    "cad": "CAD mechanics",
    "other": "Other",
}


def rgb(code):
    return np.array([int(code[i : i + 2], 16) for i in (1, 3, 5)]) / 255


def is_active(o):
    if "active_material" in o:
        return bool(o["active_material"])
    m = o.get("material", "").lower()
    p = o.get("path", "").lower()
    g = o["subsystem"]
    return (
        (g == "trigger" and m in ("polyvinyltoluene", "scintillator"))
        or (g == "tracker" and "active" in p and "silicon" in m)
        or (g == "ecal" and m == "silicon")
        or (g == "hcal" and m == "scintillator")
        or (g == "target" and "lyso" in m)
    )


def triangulate(objects):
    ts = []
    cs = []
    for o in objects:
        v = np.asarray(o["vertices_mm"], float)
        color = rgb(PALETTE.get(o["subsystem"], PALETTE["other"]))
        if o["subsystem"] == "hcal" and is_active(o):
            # Alternating shade reveals touching bars without changing geometry.
            strip = (
                int(o.get("path", "").rsplit("_strip", 1)[-1].split("_")[0])
                if "_strip" in o.get("path", "")
                else 0
            )
            color = np.clip(color * (1.18 if strip % 2 else 0.82), 0, 1)
        if not is_active(o) and o["subsystem"] != "cad_hcal":
            color = 0.30 * color + 0.70 * np.array([0.76, 0.78, 0.81])
        for face in o["polygons"]:
            for i in range(1, len(face) - 1):
                ts.append(v[[face[0], face[i], face[i + 1]]])
                cs.append(color)
    return np.asarray(ts).reshape(-1, 3, 3), np.asarray(cs).reshape(-1, 3)


def ray_extent(scene, objects):
    origin = np.asarray(scene["ray_origin_mm"])
    direction = np.asarray(scene["ray_direction"])
    points = np.concatenate([np.asarray(o["vertices_mm"]) for o in objects])
    t = (points - origin) @ direction
    return origin + direction * (t.min() - 100), origin + direction * (t.max() + 100)


def crossing_groups(scene):
    selected = []
    for o in scene["objects"]:
        hit = o.get("ray_intersection")
        if hit and is_active(o):
            selected.append(
                {
                    **hit,
                    "subsystem": o["subsystem"],
                    "volume": o.get("volume", o.get("id", "")),
                    "material": o.get("material", ""),
                    "path": o.get("path", ""),
                }
            )
    return sorted(selected, key=lambda h: h["entry_t_mm"])


def draw_scene(ax, objects, right, up, ray_start, ray_end, hits, title, linewidth=0.2):
    tri, colors = triangulate(objects)
    depth = np.cross(right, up)
    projected = np.stack((tri @ right, tri @ up), axis=-1)
    order = np.argsort(tri.mean(1) @ depth)
    normals = np.cross(tri[:, 1] - tri[:, 0], tri[:, 2] - tri[:, 0])
    normals /= np.maximum(np.linalg.norm(normals, axis=1)[:, None], 1e-20)
    shade = 0.68 + 0.32 * np.abs(normals @ np.array([0.4, -0.3, 0.866]))
    ax.add_collection(
        PolyCollection(
            projected[order],
            facecolors=(colors * shade[:, None])[order],
            edgecolors="#ffffff28",
            linewidths=linewidth,
            zorder=2,
        )
    )
    a = np.array([ray_start, ray_end])
    xy = np.column_stack((a @ right, a @ up))
    ax.plot(*xy.T, color="#db453d", lw=2, zorder=8)
    # Arrow indicates direction of flight; marker size is intentionally enlarged.
    ax.annotate(
        "",
        xy=xy[0] * 0.25 + xy[1] * 0.75,
        xytext=xy[0] * 0.32 + xy[1] * 0.68,
        arrowprops={"arrowstyle": "-|>", "color": "#db453d", "lw": 2},
        zorder=9,
    )
    if hits:
        p = np.array([(np.array(h["entry_mm"]) + h["exit_mm"]) / 2 for h in hits])
        ax.scatter(
            p @ right,
            p @ up,
            s=18,
            facecolor="#faeedf",
            edgecolor="#ce362f",
            lw=0.7,
            zorder=10,
        )
    lo = projected.min((0, 1))
    hi = projected.max((0, 1))
    lo = np.minimum(lo, xy.min(0))
    hi = np.maximum(hi, xy.max(0))
    span = np.maximum(hi - lo, 1)
    ax.set_xlim(lo[0] - 0.09 * span[0], hi[0] + 0.09 * span[0])
    ax.set_ylim(lo[1] - 0.04 * span[1], hi[1] + 0.04 * span[1])
    ax.set_aspect("equal")
    ax.set_title(title, loc="left", fontsize=12, weight="bold", color="#233447", pad=11)
    ax.set_facecolor("#ffffff")
    ax.grid(alpha=0.12, zorder=0)
    ax.tick_params(labelsize=8, colors="#5b697c")
    ax.set_xlabel("Projected distance [mm]", fontsize=9)
    ax.set_ylabel("Projected height [mm]", fontsize=9)
    for spine in ax.spines.values():
        spine.set_color("#ccd3dc")


def render(scene, out):
    objects = scene["objects"]
    active = [o for o in objects if is_active(o)]
    if not active:
        raise ValueError("No active-material objects in scene")
    hits = crossing_groups(scene)
    r0, r1 = ray_extent(scene, objects)
    direction = np.asarray(scene["ray_direction"], float)
    # Set visual up opposite the trajectory; horizontal view axes retain scale.
    up = -direction
    ref = np.array([1.0, 0.0, 0.0])
    right = ref - up * np.dot(ref, up)
    if np.linalg.norm(right) < 1e-4:
        right = np.cross(up, np.array([0.0, 1.0, 0.0]))
    right /= np.linalg.norm(right)
    depth = np.cross(right, up)
    ir = 0.88 * right + 0.475 * depth
    ir /= np.linalg.norm(ir)
    iu = 0.91 * up - 0.415 * np.cross(ir, up)
    iu /= np.linalg.norm(iu)
    fig = plt.figure(figsize=(13, 9), dpi=170, facecolor="#f4f6f9")
    gs = fig.add_gridspec(
        1,
        3,
        width_ratios=[1.4, 1.1, 0.8],
        left=0.055,
        right=0.98,
        bottom=0.19,
        top=0.80,
        wspace=0.3,
    )
    ax = fig.add_subplot(gs[0, 0])
    draw_scene(
        ax,
        objects,
        ir,
        iu,
        r0,
        r1,
        hits,
        "CAD supports and detectors"
        if any(o["subsystem"] == "cad" for o in objects)
        else "Detector geometry",
    )
    ar0, ar1 = ray_extent(scene, active)
    ax = fig.add_subplot(gs[0, 1])
    draw_scene(ax, active, ir, iu, ar0, ar1, hits, "Active materials along the ray")
    ax.locator_params(axis="x", nbins=3)
    ax = fig.add_subplot(gs[0, 2])
    ax.axis("off")
    ax.text(
        0,
        1,
        "Geometric crossings",
        fontsize=12,
        weight="bold",
        color="#233447",
        va="top",
    )
    by = {}
    for hit in hits:
        by.setdefault(hit["subsystem"], []).append(hit)
    runs = []
    for h in hits:
        if not runs or runs[-1][0] != h["subsystem"]:
            runs.append([h["subsystem"], []])
        runs[-1][1].append(h)
    y = 0.90
    for i, (g, runhits) in enumerate(runs):
        ax.text(
            0, y, f"{i + 1:02d}", fontsize=17, weight="bold", color=PALETTE[g], va="top"
        )
        ax.text(
            0.22, y, LABELS[g], fontsize=10.5, weight="bold", va="top", color="#25394d"
        )
        ax.text(
            0.22,
            y - 0.048,
            f"{len(runhits)} material-solid crossings",
            fontsize=9,
            va="top",
            color="#5b697b",
        )
        y -= 0.092
    ax.text(
        0,
        max(0.02, y - 0.03),
        (
            "Ordered along the ray.\nMarkers denote geometry,\nnot simulated "
            "hits.\n\nNo Geant4 transport\nor energy-loss model."
        ),
        fontsize=9.5,
        linespacing=1.5,
        color="#5b697b",
        va="top",
    )
    fig.text(
        0.055,
        0.95,
        "Cosmic ray illustration",
        fontsize=23,
        weight="bold",
        color="#1d2e44",
    )
    fig.text(
        0.055,
        0.908,
        scene.get(
            "title", "Modular test stand · detector solids from the ESA 2025 foundation"
        ),
        fontsize=11,
        color="#506079",
    )
    handles = [
        Line2D([0], [0], color=PALETTE[g], lw=5, label=LABELS[g])
        for g in ["trigger", "tracker", "ecal", "hcal", "target"]
        if any(o["subsystem"] == g for o in active)
    ]
    if any(o["subsystem"] == "cad_hcal" for o in objects):
        handles.append(
            Line2D([0], [0], color=PALETTE["cad_hcal"], lw=5, label=LABELS["cad_hcal"])
        )
    fig.legend(
        handles=handles,
        loc="lower center",
        bbox_to_anchor=(0.46, 0.061),
        ncol=3,
        frameon=False,
        fontsize=9.5,
    )
    if scene.get("cad_hcal_stations"):
        fig.text(
            0.055,
            0.867,
            (
                "HCal bars use upstream dimensions and CAD station positions. "
                "Layout and strip counts need hardware confirmation."
            ),
            fontsize=9,
            color="#226b66",
        )
    fig.text(
        0.055,
        0.035,
        (
            "Illustrative straight trajectory; geometry intersections are "
            "not simulated hits. Thin layers and crossing markers remain "
            "visible at enlarged screen scale."
        ),
        fontsize=8.5,
        color="#59687a",
    )
    fig.savefig(out / "cosmic_ray_geometry.png", dpi=170)
    fig.savefig(out / "cosmic_ray_geometry.jpg", dpi=110, pil_kwargs={"quality": 88})
    plt.close(fig)
    (out / "cosmic_ray_crossings.json").write_text(
        json.dumps(
            {
                "source_gdml": scene.get("source_gdml"),
                "ray_origin_mm": scene["ray_origin_mm"],
                "ray_direction": scene["ray_direction"],
                "method": scene.get("method"),
                "caveat": scene["caveat"],
                "crossings": hits,
                "by_subsystem": {k: len(v) for k, v in by.items()},
            },
            indent=2,
        )
    )
    interactive(scene, out, r0, r1, hits)
    return {
        "objects": len(objects),
        "active_material_objects": len(active),
        "crossings": len(hits),
        "subsystem_crossings": {k: len(v) for k, v in by.items()},
        "cad_hcal_stations": scene.get("cad_hcal_stations", []),
        "figure": "cosmic_ray_geometry.png",
        "viewer": "cosmic_ray_interactive.html",
        "claim": (
            "Illustrative trajectory; no Geant4 transport, energy loss, or "
            "simulated hits."
        ),
    }


def webdata(tri, colors, center, scale):
    n = np.cross(tri[:, 1] - tri[:, 0], tri[:, 2] - tri[:, 0])
    n /= np.maximum(np.linalg.norm(n, axis=1)[:, None], 1e-20)
    a = np.concatenate(
        (
            ((tri - center) / scale).reshape(-1, 3),
            np.repeat(n, 3, axis=0),
            np.repeat(colors, 3, axis=0),
        ),
        axis=1,
    ).astype("<f4")
    return base64.b64encode(a.tobytes()).decode()


def interactive(scene, out, r0, r1, hits):
    active = [o for o in scene["objects"] if is_active(o) and o["subsystem"] != "hcal"]
    passive = [
        o
        for o in scene["objects"]
        if not is_active(o) and o["subsystem"] not in ("cad", "cad_hcal")
    ]
    hcal = [o for o in scene["objects"] if o["subsystem"] == "hcal" and is_active(o)]
    cad = [o for o in scene["objects"] if o["subsystem"] == "cad"]
    t, c = triangulate(active)
    pt, pc = triangulate(passive)
    ct, cc = triangulate(cad)
    ht, hc = triangulate(hcal)
    allp = np.concatenate(
        [
            t.reshape(-1, 3),
            pt.reshape(-1, 3),
            ct.reshape(-1, 3),
            ht.reshape(-1, 3),
            [r0, r1],
        ]
    )
    center = (allp.min(0) + allp.max(0)) / 2
    scale = max(allp.max(0) - allp.min(0)) / 2
    # Rotate scene so physical trajectory is displayed downward by default.
    up = -np.asarray(scene["ray_direction"])
    ref = np.array([1.0, 0, 0])
    right = ref - up * (ref @ up)
    right /= np.linalg.norm(right)
    front = np.cross(right, up)
    view_rotation = np.vstack([right, up, front])
    t = (t - center) @ view_rotation.T
    pt = (pt - center) @ view_rotation.T
    ct = (ct - center) @ view_rotation.T
    ht = (ht - center) @ view_rotation.T
    a = (np.array([r0, r1]) - center) @ view_rotation.T
    marker = []
    for h in hits:
        p = ((np.asarray(h["entry_mm"]) + h["exit_mm"]) / 2 - center) @ view_rotation.T
        v = (
            p
            + np.array(
                [[1, 0, 0], [-1, 0, 0], [0, 1, 0], [0, -1, 0], [0, 0, 1], [0, 0, -1]]
            )
            * 8
        )
        for face in [
            [0, 2, 4],
            [2, 1, 4],
            [1, 3, 4],
            [3, 0, 4],
            [2, 0, 5],
            [1, 2, 5],
            [3, 1, 5],
            [0, 3, 5],
        ]:
            marker.append(v[face])
    # Thin two-sided ribbon along the ray gives stable visibility in an orbit view.
    rad = 5.0
    delta = np.array([rad, 0, 0])
    delta2 = np.array([0, 0, rad])
    for d in [delta, delta2]:
        marker.extend(
            [
                np.array([a[0] - d, a[0] + d, a[1] + d]),
                np.array([a[0] - d, a[1] + d, a[1] - d]),
            ]
        )
    rt = np.array(marker)
    rc = np.repeat(rgb("#dc433d")[None, :], len(rt), axis=0)
    data = {
        "active": webdata(t, c, np.zeros(3), scale),
        "passive": webdata(pt, pc, np.zeros(3), scale),
        "cad": webdata(ct, cc, np.zeros(3), scale),
        "hcal_bars": webdata(ht, hc, np.zeros(3), scale),
        "ray": webdata(rt, rc, np.zeros(3), scale),
    }
    groups = {
        g: sum(h["subsystem"] == g for h in hits)
        for g in LABELS
        if any(h["subsystem"] == g for h in hits)
    }
    legend = "".join(
        f'<p><b style="color:{PALETTE[g]}">{LABELS[g]}</b> — {n} crossings</p>'
        for g, n in groups.items()
    )
    html = (Path(__file__).resolve().parent / "viewer.html").read_text()
    html = html.replace("__DATA__", json.dumps(data)).replace("__LEGEND__", legend)
    (out / "cosmic_ray_interactive.html").write_text(html)


def main():
    p = argparse.ArgumentParser()
    p.add_argument("scene", type=Path)
    p.add_argument("--output", type=Path, default=HERE / "visualization")
    a = p.parse_args()
    a.output.mkdir(parents=True, exist_ok=True)
    scene = json.loads(a.scene.read_text())
    stats = render(scene, a.output)
    (a.output / "cosmic_visualization_metadata.json").write_text(
        json.dumps(stats, indent=2)
    )
    print(json.dumps(stats))


if __name__ == "__main__":
    main()
