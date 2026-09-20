#!/usr/bin/env python3
"""Compare the original, raised, and raised-with-centred-ECal layouts."""
from pathlib import Path
import json
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D

HERE = Path(__file__).resolve().parent
BASE = HERE.parent
CENTER = HERE/'ecal_centered'
nominal = json.loads((BASE/'results.json').read_text())
raised = json.loads((HERE/'results.json').read_text())
centered = json.loads((CENTER/'results.json').read_text())
comparison = json.loads((HERE/'comparison.json').read_text())
center_comparison = json.loads((CENTER/'comparison.json').read_text())
c0, c3 = comparison['cases']
e0, e3 = center_comparison['cases']
colors = {'hcal':'#257f78', 'trigger':'#b78011', 'tracker':'#286cb0',
          'target':'#7951a4', 'ecal':'#b3456d'}
labels = {'hcal':'HCal', 'trigger':'TS', 'tracker':'Tracker', 'target':'LYSO', 'ecal':'ECal'}
plt.rcParams.update({'font.size':9, 'axes.spines.top':False, 'axes.spines.right':False,
                     'pdf.fonttype':42, 'figure.facecolor':'white', 'savefig.facecolor':'white'})
fig, axes = plt.subplots(1, 3, figsize=(8.8, 3.5), sharex=True, sharey=True,
                         layout='constrained')
for ax, folder, title in zip(axes, [BASE,HERE,CENTER],
        ['A  Original', 'B  Assembly raised 0.20 m', 'C  Raised + ECal centred']):
    layers = json.loads((folder/'layer_inventory.json').read_text())
    sample = np.load(folder/'accepted_rays.npz')
    for i in np.linspace(0, len(sample['ray'])-1, 24, dtype=int):
        ray = sample['ray'][i]
        zz = np.array([2180,3810])
        xx = ray[0] + ray[2]*(float(sample['zref'])-zz) - 5027.51999312
        ax.plot(xx, zz/1000, color='#325a70', alpha=.22, lw=.65, zorder=1)
    for layer in layers:
        lo,hi = np.array(layer['xy_bbox_mm'])
        ax.hlines(layer['z_mm']/1000, lo[0]-5027.51999312, hi[0]-5027.51999312,
                  color=colors[layer['subsystem']], lw=2, zorder=3)
    ax.set(xlim=(-145,145), ylim=(2.18,3.81), xlabel='X relative to TS axis [mm]')
    ax.set_xticks([-100,0,100])
    ax.set_title(title, loc='left', fontsize=9.5, fontweight='bold')
    ax.grid(alpha=.13)
axes[0].set_ylabel('CAD height Z [m]')
fig.legend(handles=[Line2D([0],[0],color=colors[k],lw=3,label=labels[k])
                    for k in colors],loc='outside lower center',ncol=5,frameon=False)
fig.savefig(HERE/'geometry_shift_comparison.pdf')
fig.savefig(HERE/'geometry_shift_comparison.png',dpi=180)
plt.close(fig)

text = r'''\clearpage
\section*{Raising the TS/tracker/LYSO assembly by 0.20 m}
All three trigger-scintillator (TS) stations, all four tracker sensors and both
LYSO layers move together by $+200$ mm in CAD $Z$. Their transverse positions,
rotations, internal spacings and segmentation stay the same. ECal and all three
HCal groups remain fixed. A muon must still cross every one of the 26 active layers.

\textbf{Raising this assembly alone reduces the estimated rate by @@RED@@\%.}
The top-TS to lower-LYSO separation stays at $383.4$ mm, preserving the internal
collimation, but the distance to the fixed, offset ECal grows by 200 mm. Fewer
accepted directions also cross every ECal layer. The geometric
$p>1\ \mathrm{GeV}/c$ reference changes from @@G0@@ to
$@@GS@@\pm@@GE@@$ muons/hour; the error is numerical only.

\begin{center}
\begin{tabular}{lrrr}
\toprule
Overhead shielding & Original/hour & Raised/hour & Change\\
\midrule
None & @@B0@@ & @@S0@@ & $@@P0@@\%$\\
3 ft concrete & @@B3@@ & @@S3@@ & $@@P3@@\%$\\
\bottomrule
\end{tabular}
\end{center}

\subsection*{Centering the ECal as well}
Keep the TS/tracker/LYSO assembly raised and translate the entire ECal in $X,Y$
so that the centre of its combined active-silicon envelope lies on the LYSO/TS
axis. This means $\Delta X=@@DX@@$ mm and $\Delta Y=@@DY@@$ mm; ECal height,
rotations and internal layer offsets stay unchanged. All HCal positions stay fixed.
\textbf{Centering increases the rate by @@GAIN@@\% relative to raising alone},
and by @@TOTAL@@\% relative to the original layout.

\begin{center}
\begin{tabular}{lrrr}
\toprule
Overhead shielding & Raised + centred/hour & Per day & Gain over raised\\
\midrule
None & @@E0@@ & @@ED0@@ & $+@@GAIN0@@\%$\\
3 ft concrete & @@E3@@ & @@ED3@@ & $+@@GAIN3@@\%$\\
\bottomrule
\end{tabular}
\end{center}

\begin{center}
\includegraphics[width=\linewidth]{shift_up_200mm/geometry_shift_comparison.pdf}
\end{center}
\textbf{Figure 3.} The three layouts with 24 accepted straight rays each, in
$X$--$Z$. TS, tracker and LYSO move upward together in B and C; ECal is also
centred in C. Its $Y$ shift is not visible in this projection. Line thicknesses
are enlarged, and HCal extends outside the transverse window.

\textbf{Checks and limits.} Each variation uses eight Sobol scrambles of $2^{18}$
trials. ROOT agrees with all 4,992 layer decisions in each 192-ray test, including
109 translated active solids for B and 113 for C. Both independent pseudorandom
checks agree within @@MC@@ numerical standard errors. The spectrum and roof
model are unchanged. To isolate placement effects, the detector stopping proxy
stays at the original $@@MASS@@\ \mathrm{g\,cm^{-2}}$ vertical column; it is not
remeasured. Scattering, detector inefficiency and mechanical clearances remain
unmodelled. These are rate studies, not validated mechanical or transport geometries.

'''
mc_sigma = max(abs(r['rate_Hz']-r['independent_MC_Hz']) / np.hypot(
    r['numerical_SE_Hz'],r['independent_MC_SE_Hz']) for r in [raised,centered])
gain0 = 100*(e0['shifted_rate_per_hour']/c0['shifted_rate_per_hour']-1)
gain3 = 100*(e3['shifted_rate_per_hour']/c3['shifted_rate_per_hour']-1)
assert c0['fractional_change']<0 and gain0>0 and gain3>0
values = {
    'RED':f'{-100*c0["fractional_change"]:.1f}',
    'B0':f'{c0["nominal_rate_per_hour"]:.3f}', 'S0':f'{c0["shifted_rate_per_hour"]:.3f}',
    'B3':f'{c3["nominal_rate_per_hour"]:.3f}', 'S3':f'{c3["shifted_rate_per_hour"]:.3f}',
    'P0':f'{100*c0["fractional_change"]:.1f}', 'P3':f'{100*c3["fractional_change"]:.1f}',
    'G0':f'{nominal["rate_per_hour"]:.4f}', 'GS':f'{raised["rate_per_hour"]:.4f}',
    'GE':f'{raised["numerical_SE_per_hour"]:.4f}', 'MC':f'{np.ceil(mc_sigma*100)/100:.2f}',
    'MASS':f'{comparison["detector_vertical_mass_column_g_cm2"]:.2f}',
    'DX':f'{center_comparison["ecal_translation_mm"][0]:+.2f}',
    'DY':f'{center_comparison["ecal_translation_mm"][1]:+.2f}',
    'GAIN':f'{gain0:.1f}', 'GAIN0':f'{gain0:.1f}', 'GAIN3':f'{gain3:.1f}',
    'TOTAL':f'{100*e0["fractional_change"]:.1f}',
    'E0':f'{e0["shifted_rate_per_hour"]:.3f}', 'E3':f'{e3["shifted_rate_per_hour"]:.3f}',
    'ED0':f'{e0["shifted_rate_per_day"]:.1f}', 'ED3':f'{e3["shifted_rate_per_day"]:.1f}',
}
for key,value in values.items():
    text = text.replace('@@'+key+'@@',value)
assert '@@' not in text
(HERE/'report_section.tex').write_text(text)
print('Wrote shift/centering comparison figure and report_section.tex')
