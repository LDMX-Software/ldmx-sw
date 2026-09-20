#!/usr/bin/env python3
"""Add the centered-only and physically absent ECal comparisons to the report."""
from pathlib import Path
import json
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

HERE=Path(__file__).resolve().parent
BASE=HERE.parent
original=json.loads((BASE/'shielding_results.json').read_text())
raised=json.loads((BASE/'shift_up_200mm/comparison.json').read_text())
raised_center=json.loads((BASE/'shift_up_200mm/ecal_centered/comparison.json').read_text())
center=json.loads((HERE/'centered_only/comparison.json').read_text())
absent=json.loads((HERE/'without_ecal/comparison.json').read_text())
center_geo=json.loads((HERE/'centered_only/results.json').read_text())
absent_geo=json.loads((HERE/'without_ecal/results.json').read_text())

labels=['Original (26 layers)','Raised 20 cm (26 layers)',
        'Raised + centered ECal (26)', 'Centered ECal only (26)', 'ECal removed (22 layers)']
values=np.array([[c['rate_per_hour'] for c in original['cases']],
                 [c['shifted_rate_per_hour'] for c in raised['cases']],
                 [c['shifted_rate_per_hour'] for c in raised_center['cases']],
                 [c['rate_per_hour'] for c in center['cases']],
                 [c['rate_per_hour'] for c in absent['cases']]])
plt.rcParams.update({'font.size':10,'axes.spines.top':False,'axes.spines.right':False,
                     'pdf.fonttype':42,'figure.facecolor':'white','savefig.facecolor':'white'})
fig,ax=plt.subplots(figsize=(8.8,3.3),layout='constrained')
y=np.arange(len(labels))
for k,(offset,color,label) in enumerate([(-.18,'#286cb0','No roof'),(.18,'#b3456d','3 ft concrete')]):
    ax.barh(y+offset,values[:,k],height=.31,color=color,label=label)
    for yy,rate in zip(y+offset,values[:,k]):
        ax.text(rate+.025,yy,f'{rate:.2f}',va='center',fontsize=9)
ax.set_yticks(y,labels)
ax.invert_yaxis()
ax.set(xlim=(0,2.67),xlabel='Ideal coincidence rate [muons/hour]')
ax.xaxis.grid(alpha=.15)
ax.set_axisbelow(True)
ax.legend(loc='upper right',frameon=False,fontsize=9)
fig.savefig(HERE/'all_layout_rates.pdf')
fig.savefig(HERE/'all_layout_rates.png',dpi=180)
plt.close(fig)

c0,c3=center['cases'];n0,n3=absent['cases']
text=r'''\clearpage
\section*{Centering ECal without raising the assembly}
Keep TS, tracker, LYSO and all HCal groups at their \emph{original heights}.
Move only ECal by $\Delta X=+46.56$ mm and $\Delta Y=-46.08$ mm, using the same
active-envelope alignment as on the previous page. ECal height, rotations and
internal layer offsets stay unchanged. All 26 active layers are still required.

\textbf{Centering alone gives @@C0@@ muons/hour without a roof and @@C3@@ with
3 ft of concrete.} This is a @@GAIN@@\% increase over the original layout, and
@@OVER@@\% above the raised-and-centred configuration. In this geometry,
raising the assembly is not needed to obtain the gain from ECal alignment.

\subsection*{Removing ECal}
The second test physically removes the entire ECal subtree, including its
passive materials, while keeping every other subsystem at its original position.
The coincidence now requires \textbf{22 active layers}: six TS, four tracker,
two LYSO and ten HCal. It no longer tests passage through an ECal.

\begin{center}
\begin{tabular}{lrrrr}
\toprule
Configuration & Layers & No roof/hour & Concrete/hour & No roof/day\\
\midrule
Original & 26 & @@B0@@ & @@B3@@ & @@BD@@\\
ECal centred only & 26 & @@C0@@ & @@C3@@ & @@CD@@\\
ECal removed & 22 & @@N0@@ & @@N3@@ & @@ND@@\\
\bottomrule
\end{tabular}
\end{center}
Here ``concrete'' means the same 3 ft uniform overhead slab. The 22-layer
geometric reference is $@@NG@@\pm@@NE@@$ muons/hour for $p>1$ GeV/$c$;
centering ECal retains @@GEOM@@\% of that geometrical coincidence rate while
still requiring all four ECal layers.

\begin{center}\includegraphics[width=\linewidth]{ecal_comparison/all_layout_rates.pdf}\end{center}
\textbf{Figure 4.} All five configurations under the same incident-spectrum
model. The last row uses the relaxed 22-layer requirement and excludes ECal
material. The raised rows move TS, tracker and LYSO upward by 200 mm;
the two new cases use their original positions.

\textbf{Material treatment.} The centred-only case retains the original
$32.70\ \mathrm{g\,cm^{-2}}$ stopping proxy, consistent with the earlier placement
studies. For ECal removal, ROOT chords for 96 accepted rays give
$@@MASS@@\ \mathrm{g\,cm^{-2}}$ for the remaining detector. The same polystyrene
CSDA approximation represents that reduced column. At the original column,
the 22-layer angular acceptance alone would give @@FIX0@@ and @@FIX3@@
muons/hour; removing ECal material raises them to the values in the table.

\textbf{Checks and limits.} Eight Sobol scrambles of $2^{18}$ trials were run
for each new case, with independent pseudorandom checks. ROOT agrees with all
4,992 layer decisions for centering and all 4,224 for removal in 192-ray tests.
Scattering, straggling, decay, detector inefficiency and mechanical-fit validation
remain outside this estimate. The scripts and results are in
\texttt{ecal\_comparison/}; \texttt{build\_report.sh} rebuilds these sections too.
'''
vals={'C0':f'{c0["rate_per_hour"]:.3f}','C3':f'{c3["rate_per_hour"]:.3f}',
      'GAIN':f'{100*c0["fractional_change_from_original"]:.1f}',
      'OVER':f'{100*(c0["rate_per_hour"]/raised_center["cases"][0]["shifted_rate_per_hour"]-1):.1f}',
      'N0':f'{n0["rate_per_hour"]:.3f}','N3':f'{n3["rate_per_hour"]:.3f}',
      'B0':f'{original["cases"][0]["rate_per_hour"]:.3f}','B3':f'{original["cases"][1]["rate_per_hour"]:.3f}',
      'BD':f'{original["cases"][0]["rate_per_day"]:.1f}','CD':f'{c0["rate_per_day"]:.1f}','ND':f'{n0["rate_per_day"]:.1f}',
      'NG':f'{absent_geo["rate_per_hour"]:.4f}','NE':f'{absent_geo["numerical_SE_per_hour"]:.4f}',
      'GEOM':f'{100*center_geo["rate_Hz"]/absent_geo["rate_Hz"]:.1f}',
      'MASS':f'{absent["detector_vertical_mass_column_g_cm2"]:.2f}',
      'FIX0':f'{absent["fixed_nominal_mass_comparison"][0]["rate_per_hour"]:.3f}',
      'FIX3':f'{absent["fixed_nominal_mass_comparison"][1]["rate_per_hour"]:.3f}'}
for key,value in vals.items():text=text.replace('@@'+key+'@@',value)
assert '@@' not in text
(HERE/'report_section.tex').write_text(text)
print('Wrote ECal-only/removal section and comparison chart.')
