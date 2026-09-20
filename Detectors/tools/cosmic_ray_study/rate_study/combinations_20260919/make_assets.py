#!/usr/bin/env python3
"""Generate all report numbers, tables, and vector figures from frozen results."""
from pathlib import Path
import json,math
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import legacy_geometry as geo
HERE=Path(__file__).resolve().parent;BASE=HERE.parent
R=json.loads((HERE/'results/results.json').read_text())
D={x['id']:x for x in R['specified']}
I={(x['hcal_mask'],x['S'],x['Y'],x['E'],x['tracker_k']):x for x in R['inclusive']}
A={x['trigger']:x for x in json.loads((HERE/'results/hcal_any_layer_triggers.json').read_text())['rows']}
T=HERE/'tables';T.mkdir(exist_ok=True)
F=HERE/'figures';F.mkdir(exist_ok=True)

def f(x):
    if x>=1e4:return r'$'+format(x/10**int(math.log10(x)),'.3f')+r'\times10^{'+str(int(math.log10(x)))+'}$'
    if x>=100:return f'{x:,.1f}'
    return f'{x:.3f}'
def compact(x):
    if x>=1e4:return f'{x/1000:.1f}k'
    if x>=100:return f'{x:.1f}'
    return f'{x:.3f}'
def pm(x,metric):
    m=x[metric+'_per_hour'];s=x[metric+'_se_per_hour']
    places=max(0,min(5,1-int(math.floor(math.log10(s))))) if s>0 else 3
    return '$'+format(m,f',.{places}f')+r'\pm'+format(s,f'.{places}f')+'$'
def table(name,caption,label,columns,head,rows,size='small'):
    txt='\\begin{table}[htbp]\n\\centering\\'+size+'\n\\caption{'+caption+'}\\label{'+label+'}\n'
    if name.startswith('all_'):txt+='\\renewcommand{\\arraystretch}{1.0}\n'
    txt+='\\begin{tabular}{'+columns+'}\n\\toprule\n'+head+r'\\'+'\n\\midrule\n'
    for row in rows:txt+=row if isinstance(row,str) else ' & '.join(row)+r'\\'+'\n'
    txt+='\\bottomrule\n\\end{tabular}\n\\end{table}\n';(T/name).write_text(txt)

gold=D[1023];hcal=D[7]
macros={'goldNo':f'{gold["no_roof_per_hour"]:.3f}','goldRoof':f'{gold["concrete_per_hour"]:.3f}',
        'hcalNo':f'{hcal["no_roof_per_hour"]:,.0f}','hcalRoof':f'{hcal["concrete_per_hour"]:,.0f}',
        'goldFraction':f'{100*gold["concrete_per_hour"]/hcal["concrete_per_hour"]:.4f}',
        'goldDenominator':f'{hcal["concrete_per_hour"]/gold["concrete_per_hour"]:,.0f}',
        'goldWait':f'{60/gold["concrete_per_hour"]:.1f}','fluxNorm':f'{R["flux_normalization"]:.6f}'}
(T/'numbers.tex').write_text(''.join('\\newcommand{\\'+k+'}{'+v+'}\n' for k,v in macros.items()))
table('inventory.tex','Active detector inventory. Heights are CAD $Z$; within each range, layers are ordered from top to bottom.',
      'tab:inventory','lrrl','Requirement & Layers & Active volumes & Height range (mm)',[
      ['$H_T$: top HCAL','2','24','3758.711--3735.339'],['$S$: TS counters','6','72','3449.388--3140.588'],
      ['$T_1,T_2,T_3,T_4$: tracker','4','4','3309.888--3203.888'],['$Y$: LYSO','2','33','3066.788--3065.988'],
      ['$H_M$: HCAL above ECAL','4','32','2796.346--2720.146'],['$E$: ECAL silicon','4','4','2566.032--2508.962'],
      ['$H_B$: HCAL below ECAL','4','48','2291.504--2220.931'],r'\midrule'+'\n',['Golden event','26','217','All requirements']])
hn={1:'$H_T$',2:'$H_M$',4:'$H_B$',3:'$H_TH_M$',5:'$H_TH_B$',6:'$H_MH_B$',7:'$H_TH_MH_B$'}
rows=[]
for h in [1,2,4,3,5,6,7]:
    d=D[h];a=A[f'any_layer_stations_{h}']
    rows.append([hn[h],compact(d['no_roof_per_hour']),compact(d['concrete_per_hour']),compact(a['no_roof_per_hour']),compact(a['concrete_per_hour'])])
rows += [r'\midrule'+'\n']
for key,label in [('any_station','Any station'),('at_least_two_stations',r'At least two stations')]:
    a=A[key];d=A['strict_'+key];rows.append([label,compact(d['no_roof_per_hour']),compact(d['concrete_per_hour']),compact(a['no_roof_per_hour']),compact(a['concrete_per_hour'])])
table('hcal_triggers.tex','HCAL muon-trigger estimates per hour. Strict means every layer in each selected station; loose means any layer in each selected station. A suffix k means $10^3$ events/hour. These rows impose no central-detector requirements; the full numerical errors are in the CSV tables.',
      'tab:hcal','lrrrr',r' & \multicolumn{2}{c}{Strict: all station layers} & \multicolumn{2}{c}{Loose: any station layer}\\'+r'Stations & No roof & Concrete & No roof & Concrete',rows)
principal=[('Golden: everything',(1,1,1,4)),('Everything minus ECAL',(1,1,0,4)),
('Everything minus one tracker layer',(1,1,1,3)),('Minus ECAL and one tracker layer',(1,1,0,3)),
('Everything minus two tracker layers',(1,1,1,2)),('Minus ECAL and two tracker layers',(1,1,0,2)),
('Everything minus three tracker layers',(1,1,1,1)),('Minus ECAL and three tracker layers',(1,1,0,1)),
('Everything minus all tracker layers',(1,1,1,0)),('$H+S+Y$; no ECAL or tracker cut',(1,1,0,0)),
('$H+Y+E+T_{\ge4}$; no TS cut',(0,1,1,4)),('$H+E+T_{\ge4}$',(0,0,1,4)),
('$H+Y+T_{\ge4}$',(0,1,0,4)),('$H+E$',(0,0,1,0)),('$H+T_{\ge4}$',(0,0,0,4)),
('$H+S$',(1,0,0,0)),('$H+Y$',(0,1,0,0)),('$H$ only: all ten HCAL layers',(0,0,0,0))]
rows=[]
for label,syek in principal:
    d=I[(7,*syek)];rows.append([label,pm(d,'no_roof'),pm(d,'concrete')])
table('principal.tex','Requested selections, in muons/hour, with $H=H_TH_MH_B$. An omitted hit requirement removes no material. Errors are numerical standard errors only. Each tracker omission is inclusive.',
      'tab:principal','lrr','Selection & No roof & 3 ft concrete',rows,'footnotesize')
rows=[]
for j in range(4):
    for e in [1,0]:
        d=D[7+8+16+32*e+64*(15^(1<<j))]
        rows.append([f'$T_{j+1}$', 'Required' if e else 'Unrestricted',pm(d,'no_roof'),pm(d,'concrete')])
table('tracker_omissions.tex','Specified single-tracker omissions with $H,S,Y$ and the other three tracker layers required. The omitted layer may still be hit. Tracker heights are $Z=(3309.888,3303.888,3209.888,3203.888)$ mm for $T_1$ through $T_4$.',
      'tab:tracker','llrr','Unrestricted tracker & ECAL & No roof/hour & Concrete/hour',rows,'footnotesize')

for metric,title,name in [('concrete','3 ft concrete','all_concrete.tex'),('no_roof','No overhead slab','all_no_roof.tex')]:
    rows=[]
    for s,y,e in [(1,1,1),(1,1,0),(1,0,1),(1,0,0),(0,1,1),(0,1,0),(0,0,1),(0,0,0)]:
        for k in [4,3,2,1,0]:rows.append([f'{s}\ {y}\ {e}\quad {k}']+[compact(I[(h,s,y,e,k)][metric+'_per_hour']) for h in [1,2,4,3,5,6,7]])
        if (s,y,e)!=(0,0,0):rows.append(r'\addlinespace[2pt]'+'\n')
    table(name,title+': all inclusive equipment combinations, in muons/hour. A suffix k means $10^3$ events/hour. Read the first column as $S,Y,E$ and the minimum tracker count $k$.',
          'tab:all-'+metric,'lrrrrrrr',r'$S\ Y\ E\quad k$ & $H_T$ & $H_M$ & $H_B$ & $H_TH_M$ & $H_TH_B$ & $H_MH_B$ & $H_TH_MH_B$',rows,'footnotesize')
rows=[]
for h,inds in [(1,[0,1]),(2,[14,15,16,17]),(4,[22,23,24,25])]:
    for j,i in enumerate(inds,1):
        d=A[f'layer_{i}'];rows.append([hn[h]+f', layer {j}',compact(d['no_roof_per_hour']),compact(d['concrete_per_hour'])])
table('hcal_singles.tex','Single-layer muon crossings per hour. A suffix k means $10^3$ events/hour. Other layer hits are unrestricted.','tab:singles','lrr','HCAL layer & No roof & 3 ft concrete',rows)
table('historical.tex','Previously published placement comparisons (18 September). These rates use the earlier mean vertical column; they are preserved, not refitted.','tab:historical','lrrr','Layout & Required layers & No roof/hour & Concrete/hour',[
['Original, offset ECAL','26','1.407','1.199'],['TS/tracker/LYSO raised 200 mm','26','1.264','1.077'],['Raised assembly, centred ECAL','26','1.949','1.660'],['ECAL centred only','26','2.163','1.842'],['ECAL physically removed','22','2.214','1.883']],'small')
table('material_sensitivity.tex','Concrete rates per hour when the installed detector column on every ray is rescaled. The roof and hit logic are unchanged. The last column gives the no-roof rate-weighted mean material column in $\\mathrm{g\\,cm^{-2}}$. These variations are not systematic uncertainty bounds.','tab:material','lrrrr','Selection & $X_d/2$ & Nominal $X_d$ & $2X_d$ & Mean $X_d$',[['Golden']+[f(gold[k+'_per_hour']) for k in ['half_column_concrete','concrete','double_column_concrete']]+[f'{gold["mean_column_g_cm2"]:.2f}'],['All ten HCAL layers']+[f(hcal[k+'_per_hour']) for k in ['half_column_concrete','concrete','double_column_concrete']]+[f'{hcal["mean_column_g_cm2"]:.2f}']],'small')
(T/'validation_text.tex').write_text(r'''The new prism code reproduces every active-hit decision for the 192 saved
centred-layout ROOT rays and a further 192 broad-angle ROOT rays (9,984 layer
decisions in total). It also agrees with the independent Python intersection
code for 16,384 broad rays. Material columns agree with ROOT to less than
$2.4\times10^{-9}\ \mathrm{g\,cm^{-2}}$, including partial columns to the
layer exit planes. All 896 specified-subset rates obey requirement monotonicity,
as do the 280 inclusive multiplicity rows.

Independent pseudorandom estimates agree with the golden and strict ten-layer
HCAL rates within 0.20 and 1.57 combined numerical standard errors,
respectively. Separate angular checks of the three-station any-layer trigger
and the union of all HCAL layers agree within 1.37 and 0.16 standard errors.
The tracker plateau also has a geometric check: the full TS--LYSO anchor
envelope lies at least 6.62 mm inside every tracker active polygon.

The golden geometric reference is $1.55021\pm0.00045$ per hour, consistent
with the earlier $1.54995\pm0.00030$ result. An independent angular proposal
$q(c)=2c$, which samples more oblique rays, agrees with all three strict
single-station rates within 1.95 combined numerical standard errors; the
largest difference is 0.0092\%. The flux interpolation check has maximum
relative residual 0.0209\% over 250 test points; the energy-quadrature check
against adaptive integration reaches $1.3\times10^{-7}$. These residuals are
separate from the reported Sobol standard errors. Validation scripts and
machine-readable checks are included in the reproduction package.
''')

plt.rcParams.update({'font.family':'serif','font.size':12,'axes.spines.top':False,'axes.spines.right':False,'pdf.fonttype':42})
layers=geo.prepare(json.loads((HERE/'inputs/aligned_scene.json').read_text()))
colors={'hcal':'#257f78','trigger':'#b78011','tracker':'#286cb0','target':'#7951a4','ecal':'#b3456d'}
fig,axs=plt.subplots(1,2,figsize=(10,4.7),sharey=True)
for ax,dim,origin in zip(axs,[0,1],[5027.519993117254,-155.750230102]):
    seen=set()
    for l in layers:
        ax.plot([l['lo'][dim]-origin,l['hi'][dim]-origin],[l['z_mm']]*2,color=colors[l['subsystem']],lw=3 if l['subsystem']!='tracker' else 2,
                label=l['subsystem'].upper() if l['subsystem'] not in seen else None)
        seen.add(l['subsystem'])
    ax.axvline(0,color='black',lw=.5,ls=':');ax.set_xlabel(('X' if dim==0 else 'Y')+' relative to tracker axis [mm]');ax.grid(alpha=.15)
axs[0].set_ylabel('CAD Z [mm]');axs[1].legend(loc='upper right',fontsize=9,frameon=False)
fig.tight_layout();fig.savefig(F/'aligned_geometry.pdf');fig.savefig(F/'aligned_geometry.png',dpi=150);plt.close(fig)

items=[('HCAL only',(0,0,0,0)),('HCAL + ECAL',(0,0,1,0)),('HCAL + 4 tracker layers',(0,0,0,4)),
       ('HCAL + ECAL + 4 tracker layers',(0,0,1,4)),('HCAL + TS',(1,0,0,0)),('HCAL + LYSO',(0,1,0,0)),
       ('All central, ECAL unrestricted',(1,1,0,4)),('All central, no tracker requirement',(1,1,1,0)),('Golden: all central',(1,1,1,4))]
from matplotlib.offsetbox import AnnotationBbox, HPacker, TextArea
fig,ax=plt.subplots(figsize=(10.8,4.8));ys=np.arange(len(items))
for delta,metric,color,label in [(-.14,'no_roof','#ababab','No roof'),(.14,'concrete','#254a6b','3 ft concrete')]:
    vals=[I[(7,*syek)][metric+'_per_hour'] for _,syek in items];ax.scatter(vals,ys+delta,s=35,color=color,label=label)
ax.set_yticks(ys,['']*len(items));ax.invert_yaxis();ax.set_xscale('log');ax.set_xlabel('Muon coincidence rate [hour$^{-1}$]');ax.grid(axis='x',alpha=.18);ax.legend(loc='lower right',fontsize=10)
def label_rate(value):
    return f'{value:,.0f}' if value>=1000 else f'{value:.2f}'
for y,(label,syek) in zip(ys,items):
    row=I[(7,*syek)]
    numbers=f" ({label_rate(row['no_roof_per_hour'])} / {label_rate(row['concrete_per_hour'])})"
    packed=HPacker(children=[TextArea(label,textprops={'fontsize':10.5}),
        TextArea(numbers,textprops={'fontsize':10.5,'color':'#8C1515','fontweight':'bold'})],align='center',pad=0,sep=2)
    ax.add_artist(AnnotationBbox(packed,(0,y),xycoords=ax.get_yaxis_transform(),
        xybox=(-8,0),boxcoords='offset points',box_alignment=(1,.5),frameon=False,annotation_clip=False))
fig.subplots_adjust(left=.60,right=.985,bottom=.15,top=.88)
fig.text(.02,.96,'Parentheses: no roof / 3 ft concrete, in muons per hour',fontsize=11,va='top')
fig.savefig(F/'rate_hierarchy.pdf');fig.savefig(F/'rate_hierarchy.png',dpi=150);plt.close(fig)

from make_overview import make_overview
make_overview()

wrapper=r'''\documentclass[11pt,letterpaper]{article}
\usepackage[margin=0.9in]{geometry}
\usepackage[T1]{fontenc}
\usepackage{lmodern,amsmath,amssymb,booktabs,graphicx,microtype,array,placeins}
\usepackage[colorlinks=true,linkcolor=blue!50!black,citecolor=blue!50!black,urlcolor=blue!50!black]{hyperref}
\usepackage{xcolor,xurl}
\setlength{\parskip}{3pt}
\setlength{\emergencystretch}{2em}
\renewcommand{\arraystretch}{1.13}
\makeatletter
\setlength{\@fptop}{0pt}
\setlength{\@fpsep}{18pt}
\makeatother
\hypersetup{pdftitle={Cosmic-muon trigger and coincidence rates in the aligned ESA test stand},pdfauthor={Emrys Peets; Matthew Gignac; Takumi Britt}}
\input{combinations_20260919/tables/numbers.tex}
\title{Cosmic-muon trigger and coincidence rates\\in the aligned ESA test stand}
\author{Emrys Peets \and Matthew Gignac \and Takumi Britt}
\date{20 September 2026}
\begin{document}
\maketitle
\input{combinations_20260919/report_body.tex}
\end{document}
'''
(BASE/'cosmic_rate_report.tex').write_text(wrapper)
print('Generated tables, figures and cosmic_rate_report.tex')
