#!/usr/bin/env python3
"""Make the rate figures and a self-contained LaTeX report from saved results."""
from pathlib import Path
import json,subprocess
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle
from matplotlib.lines import Line2D

HERE=Path(__file__).resolve().parent
r=json.loads((HERE/'results.json').read_text());s=json.loads((HERE/'shielding_results.json').read_text())
layers=json.loads((HERE/'layer_inventory.json').read_text());a=np.load(HERE/'accepted_rays.npz')
colors={'hcal':'#257f78','trigger':'#b78011','tracker':'#286cb0','target':'#7951a4','ecal':'#b3456d'}
labels={'hcal':'HCal','trigger':'Trigger','tracker':'Tracker','target':'LYSO','ecal':'ECal'}
plt.rcParams.update({'font.size':10,'axes.spines.top':False,'axes.spines.right':False,'axes.labelsize':10,'figure.facecolor':'white','savefig.facecolor':'white','pdf.fonttype':42})
fig,axes=plt.subplots(1,2,figsize=(8.8,4.75),sharey=True,layout='constrained')
origin=np.array([5027.51999312,-155.750230102]);zlo,zhi=2180,3810
ids=np.linspace(0,len(a['ray'])-1,32,dtype=int)
for dim,ax in enumerate(axes):
    for i in ids:
        ray=a['ray'][i];zz=np.array([zlo,zhi]);xx=ray[dim]+ray[dim+2]*(float(a['zref'])-zz)-origin[dim]
        ax.plot(xx,zz/1000,color='#325a70',alpha=.20,lw=.65,zorder=1)
    for l in layers:
        lo,hi=np.array(l['xy_bbox_mm']);ax.hlines(l['z_mm']/1000,lo[dim]-origin[dim],hi[dim]-origin[dim],color=colors[l['subsystem']],lw=2.1,zorder=3)
    ax.set_xlim(-145,145);ax.set_ylim(zlo/1000,zhi/1000);ax.grid(alpha=.13)
    ax.set_xlabel(('X','Y')[dim]+' relative to trigger centre [mm]')
    ax.set_title(('A  X-Z projection','B  Y-Z projection')[dim],loc='left',fontsize=11,fontweight='bold')
    if dim==0: ax.set_ylabel('CAD height Z [m]')
axes[1].text(.97,.86,'32 accepted straight rays',ha='right',va='top',transform=axes[1].transAxes,fontsize=9,bbox={'facecolor':'white','edgecolor':'none','alpha':.9,'pad':2})
fig.legend(handles=[Line2D([0],[0],color=colors[k],lw=3,label=labels[k]) for k in ['hcal','trigger','tracker','target','ecal']],loc='outside lower center',ncol=5,frameon=False)
fig.savefig(HERE/'geometry_acceptance.pdf');fig.savefig(HERE/'geometry_acceptance.png',dpi=180);plt.close(fig)
curve=np.loadtxt(HERE/'concrete_scan.csv',delimiter=',',skiprows=1)
fig,axes=plt.subplots(1,2,figsize=(8.8,3.75),layout='constrained')
axes[0].plot(curve[:,0]/30.48,curve[:,1],color=colors['hcal'],lw=2.2)
for case,col in zip(s['cases'],['#286cb0','#b3456d']):
    x=case['concrete_cm']/30.48;y=case['rate_per_hour'];axes[0].scatter(x,y,s=40,color=col,zorder=4)
    axes[0].annotate(f"{case['label']}\n{y:.2f}/hour",(x,y),xytext=(9,9),textcoords='offset points',fontsize=9,color=col)
axes[0].set(xlabel='Concrete thickness [ft]',ylabel='Ideal coincidence rate [muons/hour]',xlim=(-.15,5.25),ylim=(0,1.7))
axes[0].set_title('C  Continuous energy-loss estimate',loc='left',fontsize=11,fontweight='bold');axes[0].grid(alpha=.15)
for fname,case,col in zip(['angular_no_roof.csv','angular_concrete.csv'],s['cases'],['#286cb0','#b3456d']):
    vals=np.loadtxt(HERE/fname,delimiter=',',skiprows=1)
    # Rebin the fine integration histogram for an uncluttered presentation.
    bins=np.arange(0,8.401,.4);hist,_=np.histogram(vals[:,0],bins,weights=vals[:,1]*3600)
    axes[1].stairs(hist/.4,bins,color=col,lw=1.8,label=case['label'])
axes[1].set(xlabel='Zenith angle [degrees]',ylabel='Rate [muons/hour/degree]',xlim=(0,8.4));axes[1].grid(alpha=.15)
axes[1].set_title('D  Accepted angular distribution',loc='left',fontsize=11,fontweight='bold');axes[1].legend(frameon=False,fontsize=9)
fig.savefig(HERE/'rate_comparison.pdf');fig.savefig(HERE/'rate_comparison.png',dpi=180);plt.close(fig)

tex=r'''\documentclass[10pt,letterpaper]{article}
\usepackage[margin=0.78in]{geometry}
\usepackage{amsmath,amssymb,booktabs,graphicx,xcolor,microtype,enumitem}
\usepackage[colorlinks=true,urlcolor=blue!55!black,linkcolor=blue!55!black,citecolor=blue!55!black]{hyperref}
\usepackage{xurl}
\definecolor{navy}{HTML}{203C56}
\setlength{\parindent}{0pt}\setlength{\parskip}{5pt}
\setlist{nosep,leftmargin=*}
\begin{document}
{\Large\bfseries\color{navy} Cosmic muons through the ESA test stand}\par
{\large A rate estimate with and without 3 ft of concrete}\par
18 September 2026 \hfill CAD-registered geometry: \texttt{eae30ac8f}
\vspace{4pt}\hrule\vspace{7pt}

\textbf{Result.} The current geometry gives approximately \textbf{@@R0@@ muons/hour without an overhead slab} and \textbf{@@R3@@ muons/hour with 3 ft of concrete}. Both numbers require a single muon to intersect all 26 active layers. They are conditional predictions from a sea-level spectrum and a straight-line continuous-slowing-down model, before detector efficiency; they are not measured rates or a full Geant4 transport result.

\begin{center}
\begin{tabular}{lrrrr}
\toprule
Overhead shielding & Muons/hour & Muons/day & Mean interval & $K_{\min}$ at surface\\
\midrule
None & @@R0@@ & @@D0@@ & @@W0@@ min & @@K0@@ GeV\\
3 ft concrete & @@R3@@ & @@D3@@ & @@W3@@ min & @@K3@@ GeV\\
\bottomrule
\end{tabular}
\end{center}
Here $K_{\min}$ is the acceptance-weighted mean kinetic energy needed to emerge from the modelled detector with at least 10 MeV remaining. That small residual is a range-table reliability floor, not an electronics threshold. The concrete reduces this model's rate by approximately \textbf{@@RED@@\%}.

\section*{What counts as going through everything?}
A trajectory must have nonzero path length in an active volume in \emph{every layer} listed below. Within a segmented layer, hitting any one bar is sufficient. A track need not hit all bars in that layer.
\begin{center}
\begin{tabular}{lrrl}
\toprule
Subsystem & Required layers & Active volumes & Geometry used\\
\midrule
Top HCal & 2 & 24 & Two crossed layers, 12 bars each\\
Trigger counters & 6 & 72 & Three stations, two staggered layers each\\
Silicon tracker & 4 & 4 & Active daughters; inactive borders excluded\\
LYSO target & 2 & 33 & Two staggered bar arrays\\
HCal above ECal & 4 & 32 & Four crossed layers, 8 bars each\\
ECal silicon & 4 & 4 & Four hexagonal active silicon layers\\
HCal below ECal & 4 & 48 & Four crossed layers, 12 bars each\\
\midrule
Total & 26 & 217 & Bars and sensors are distinct volumes\\
\bottomrule
\end{tabular}
\end{center}
The STEP model supplies the placement framework; ESA \texttt{ldmx-reduced-v3} supplies the ECal, trigger, tracker and target internals. HCal dimensions follow the test-stand branch. The inferred HCal bar counts and detector registration still need hardware confirmation. The support renderings are not additional material in the GDML used here.

\section*{The geometric bottleneck}
The trigger bars are only 30 mm wide in $X$, while the HCal spans metres. After requiring the six trigger and two LYSO layers, the tracker and HCal do not remove further tracks in this integration; the displaced ECal footprint reduces the remaining rate by about 36\%. The accepted rays are close to vertical: 90\% are within @@TH90@@$^\circ$ and 99\% within @@TH99@@$^\circ$ under the reference angular model.

For the simple reference intensity $I(\theta)=70\cos^2\theta\ \mathrm{m}^{-2}\mathrm{s}^{-1}\mathrm{sr}^{-1}$, associated with sea-level muons above $1\ \mathrm{GeV}/c$\cite{pdg},
\[
G_2=\int dA\,d\Omega\,\cos^3\theta\,\mathbf{1}_{\mathrm{all}}=@@G@@\ \mathrm{m}^2\mathrm{sr},\qquad
R_{p>1}^{\mathrm{geom}}=@@RG@@\pm@@ERR@@\ \mathrm{hour}^{-1}.
\]
The quoted error is numerical integration error only. The headline rates are larger because the energy-dependent calculation also includes lower-energy muons that have enough range to cross the selected materials.

\newpage
\section*{Geometry and shielding illustrations}
\begin{center}\includegraphics[width=\linewidth]{geometry_acceptance.pdf}\end{center}
\textbf{Figure 1.} Two projections of 32 accepted sample trajectories. Coloured segments mark the actual layer centres and transverse extents; their line thickness is enlarged for legibility. HCal bars extend beyond the displayed transverse window. The ECal offset and narrow trigger/LYSO corridor restrict the accepted directions. These are straight geometric rays, not simulated scattering histories.

\vspace{7pt}
\begin{center}\includegraphics[width=\linewidth]{rate_comparison.pdf}\end{center}
\textbf{Figure 2.} The continuous-loss estimate versus concrete thickness, and the angular distribution of muons that satisfy all 26 layer requirements. The 3 ft comparison assumes a uniform horizontal slab that covers the stand and all accepted directions, with concrete density $2.30\ \mathrm{g\,cm^{-3}}$\cite{concrete}. Roof height and openings are not modelled. No efficiency correction or calibrated uncertainty band is implied.

\newpage
\section*{Calculation and checks}
\textbf{Geometry integration.} Two points are sampled on horizontal planes through the upper trigger and lower LYSO layer, separated by $D=383.4$ mm. Sampling rectangles are slightly expanded to include tracks that clip a finite-thickness anchor volume. If $\boldsymbol{u}=({\bf b}-{\bf a})/D$, then
\[
R=\int d^2a\,d^2b\;\frac{I(\theta)\cos^4\theta}{D^2}\,
\mathbf{1}_{\mathrm{all}},\qquad \cos\theta=(1+|\boldsymbol{u}|^2)^{-1/2}.
\]
This includes both the projected-area factor and the solid-angle Jacobian. For $I=I_0\cos^2\theta$, each ray carries a $\cos^6\theta$ weight. Box and hexagonal-prism intersections retain the physical bar gaps, stagger, tracker rotations and finite layer thickness. Eight independently scrambled Sobol sequences of $2^{18}$ point pairs give 2,097,152 trials. An independent pseudorandom estimate agrees within its numerical error. ROOT's original GDML solids agree with all 4,992 individual layer decisions in a separate 192-ray test.

\textbf{Spectrum.} The energy-dependent model uses Guan et al.'s modified Gaisser spectrum\cite{guan}, with total energy $E$ in GeV and $j$ in $\mathrm{cm^{-2}s^{-1}sr^{-1}GeV^{-1}}$:
\[
j(E,\theta)=C\,0.14\left[E+\frac{3.64}{(\cos\theta^*)^{1.29}}\right]^{-2.7}
\left[\frac{1}{1+1.1E\cos\theta^*/115}+\frac{0.054}{1+1.1E\cos\theta^*/850}\right].
\]
The factor $C=@@CNORM@@$ sets the vertical integral above $p=1\ \mathrm{GeV}/c$ to $70\ \mathrm{m^{-2}s^{-1}sr^{-1}}$. The published curvature correction for $\theta^*$ is implemented in the supplied script. The flux above each ray's required surface energy replaces the reference $70\cos^2\theta$ weight. The resulting lower-energy contribution is model dependent.

\textbf{Stopping model.} The supplied 3 ft thickness is $91.44$ cm, or $210.312\ \mathrm{g\,cm^{-2}}=2.10312$ m water equivalent at the assumed density. ROOT chords and the GDML densities give a mean vertical detector column of $@@XDET@@\ \mathrm{g\,cm^{-2}}$ over 96 accepted rays, of which roughly $20.6\ \mathrm{g\,cm^{-2}}$ is HCal scintillator. For this estimate, the whole detector column is represented by a polystyrene CSDA range\cite{poly}; the roof uses the concrete CSDA table\cite{concrete}. This is an approximation to the mixed detector materials, not material-by-material transport. With tabulated range functions $\mathcal R_p$ and $\mathcal R_c$,
\[
K_d=\mathcal R_p^{-1}\!\left[\mathcal R_p(10\ \mathrm{MeV})+X_d\sec\theta\right],\quad
K_s=\mathcal R_c^{-1}\!\left[\mathcal R_c(K_d)+X_c\sec\theta\right].
\]
The flux integral starts at $E=K_s+m_\mu$. With no roof, $K_s=K_d$. Doubling the detector column changes the 3 ft prediction from @@R3@@ to @@RDOUBLE@@/hour; requiring 100 MeV at the final exit gives @@R100@@/hour.

\textbf{Interpretation.} The recorded coincidence rate will also depend on the light-yield thresholds, dead channels, trigger logic and live time. Multiple scattering, decay and stochastic energy losses are omitted. Grazing tracks can count here: requiring a hit at every layer's midplane instead reduces the reference rate from @@RG@@ to @@RMID@@/hour, showing that the signal threshold matters. The flux normalization itself is not exact; the PDG review notes measurements 10--15\% below the reference value\cite{pdg}. No combined systematic uncertainty or local flux calibration is claimed. The estimate is for cosmic muons of both charges, not all cosmic-ray species or shower coincidences.

\textbf{Reproduction.} The folder contains the geometry snapshot, scripts, numerical tables, range data, figures and this \LaTeX{} source. Run \texttt{build\_report.sh} with Python (NumPy, SciPy, Matplotlib), ROOT and Tectonic available. The \texttt{SHA256SUMS} file records artifact identities. The original STEP SHA-256 and all source GDML hashes are preserved in \texttt{geometry/provenance.json}.

\begin{thebibliography}{4}\small\setlength{\itemsep}{0pt}
\bibitem{pdg} Particle Data Group, \emph{Cosmic Rays}, 2022 review, Sec. 30.3.1.
\url{https://pdg.lbl.gov/2022/reviews/rpp2022-rev-cosmic-rays.pdf}.
\bibitem{guan} M. Guan et al., \emph{A parametrization of the cosmic-ray muon flux at sea-level}, arXiv:1509.06176 (2015), Eqs. (2)--(3). \url{https://arxiv.org/abs/1509.06176}.
\bibitem{concrete} PDG, \emph{Shielding concrete: properties and muon CSDA range}.
\url{https://pdg.lbl.gov/2022/AtomicNuclearProperties/HTML/shielding_concrete.html}.
\bibitem{poly} PDG, \emph{Muon stopping power and range in polystyrene}.
\url{https://pdg.lbl.gov/2019/AtomicNuclearProperties/MUE/muE_polystyrene.txt}.
\end{thebibliography}
\end{document}
'''
c0,c3=s['cases']
replace={'R0':f"{c0['rate_per_hour']:.2f}",'R3':f"{c3['rate_per_hour']:.2f}",'D0':f"{c0['rate_per_day']:.1f}",'D3':f"{c3['rate_per_day']:.1f}",'W0':f"{c0['mean_wait_min']:.1f}",'W3':f"{c3['mean_wait_min']:.1f}",'K0':f"{c0['mean_required_surface_kinetic_GeV']:.3f}",'K3':f"{c3['mean_required_surface_kinetic_GeV']:.3f}",'RED':f"{100*s['roof_fractional_rate_reduction']:.0f}",'TH90':f"{r['angular_quantiles_deg']['90']:.2f}",'TH99':f"{r['angular_quantiles_deg']['99']:.2f}",'G':f"{r['geometrical_factor_m2_sr']*1e6:.4f}\\times10^{{-6}}",'RG':f"{r['rate_per_hour']:.4f}",'ERR':f"{r['numerical_SE_per_hour']:.4f}",'CNORM':f"{s['spectrum_normalization_factor']:.4f}",'XDET':f"{s['detector_vertical_mass_column_g_cm2']:.2f}",'RDOUBLE':f"{s['sensitivity']['detector_column_half_and_double_per_hour']['91.44'][1]:.2f}",'R100':f"{s['sensitivity']['exit_100MeV_per_hour']['91.44']:.2f}",'RMID':f"{r['midplane_rate_per_hour']:.3f}"}
for k,v in replace.items():tex=tex.replace('@@'+k+'@@',v)
assert '@@' not in tex
for relative in ['shift_up_200mm/report_section.tex','ecal_comparison/report_section.tex']:
    addition = HERE/relative
    if addition.exists():
        tex = tex.replace(r'\end{document}', addition.read_text() + '\n' + r'\end{document}')
(HERE/'cosmic_rate_report.tex').write_text(tex)
print('Wrote figures and cosmic_rate_report.tex')
