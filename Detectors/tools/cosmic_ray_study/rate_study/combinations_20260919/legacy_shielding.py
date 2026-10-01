#!/usr/bin/env python3
"""Continuous-range comparison: no roof versus a full-coverage concrete slab.

The detector's ROOT-measured mass thickness is represented by polystyrene for
stopping only. No scattering, detector efficiency, straggling or decay is applied.
Energy in the flux fit is total energy; PDG CSDA tables use kinetic energy.
"""
from pathlib import Path
import collections,csv,json,math
import xml.etree.ElementTree as ET
import numpy as np
from scipy.integrate import quad
from scipy.interpolate import PchipInterpolator

HERE=Path(__file__).resolve().parent
MUON_MASS_GEV=.10565839

def range_table(name):
    rows=[]
    for line in (HERE/'sources'/f'{name}.txt').read_text().splitlines():
        try: row=list(map(float,line.split()))
        except ValueError: continue
        if len(row)>=9 and row[0]>=10 and row[8]>0: rows.append(row)
    a=np.array(rows);assert len(a)>50 and np.all(np.diff(a[:,8])>0)
    return (PchipInterpolator(np.log(a[:,0]/1000),np.log(a[:,8]),extrapolate=False),
            PchipInterpolator(np.log(a[:,8]),np.log(a[:,0]/1000),extrapolate=False))

def spectrum(E,co):
    p1,p2,p3,p4,p5=.102573,-.068287,.958633,.0407253,.817285
    cs=math.sqrt((co*co+p1*p1+p2*co**p3+p4*co**p5)/(1+p1*p1+p2+p4))
    return 1e4*.14*(E+3.64/cs**1.29)**(-2.7)*(1/(1+1.1*E*cs/115)+.054/(1+1.1*E*cs/850))

def integral(E,co):return quad(spectrum,float(E),np.inf,args=(float(co),),epsabs=2e-6,epsrel=2e-7)[0]

def main():
    summary=json.loads((HERE/'results.json').read_text())
    doc=ET.parse(HERE/'geometry/detector.gdml')
    rho={m.get('name'):float(m.find('D').get('value')) for m in doc.findall('materials/material')}
    rays=np.loadtxt(HERE/'root_test_rays.csv',delimiter=',')
    columns=collections.defaultdict(float);by_mat=collections.defaultdict(float)
    for row in csv.DictReader((HERE/'material_chords.csv').open()):
        col=float(row['length_mm'])*rho[row['material']]/10
        columns[int(row['ray'])]+=col;by_mat[row['material']]+=col
    mass_vertical=np.array([v*abs(rays[k,5]) for k,v in columns.items()])
    Xdet=float(mass_vertical.mean())
    rc,ic=range_table('concrete');rp,ip=range_table('polystyrene')
    range_poly=lambda k:np.exp(rp(np.log(k)))
    kin_poly=lambda x:np.exp(ip(np.log(x)))
    range_con=lambda k:np.exp(rc(np.log(k)))
    kin_con=lambda x:np.exp(ic(np.log(x)))
    data=np.loadtxt(HERE/'angular_rate.csv',delimiter=',',skiprows=1)
    use=data[:,1]>0;theta,rate_geo=data[use].T;co=np.cos(np.deg2rad(theta))
    Eref=math.sqrt(1+MUON_MASS_GEV**2)
    norm=70/integral(Eref,1.0)
    Kexit=.010 # PDG table reliability floor, not an electronics threshold.
    def calc(thickness_cm,det_scale=1,Kout=Kexit):
        Kdet=kin_poly(range_poly(Kout)+Xdet*det_scale/co)
        if thickness_cm==0:Ksurf=Kdet
        else: Ksurf=kin_con(range_con(Kdet)+2.3*thickness_cm/co)
        flux=np.array([norm*integral(k+MUON_MASS_GEV,c) for k,c in zip(Ksurf,co)])
        bin_rate=rate_geo*flux/(70*co**2)
        return float(bin_rate.sum()),bin_rate,float(np.average(Ksurf,weights=bin_rate))
    cases=[]
    for cm,label in [(0,'No roof'),(91.44,'3 ft concrete')]:
        hz,bins,k=calc(cm);cases.append({'label':label,'concrete_cm':cm,'concrete_column_g_cm2':cm*2.3,'rate_Hz':hz,'rate_per_hour':hz*3600,'rate_per_day':hz*86400,'mean_wait_min':1/hz/60,'mean_required_surface_kinetic_GeV':k})
        np.savetxt(HERE/('angular_no_roof.csv' if cm==0 else 'angular_concrete.csv'),np.column_stack([theta,bins]),delimiter=',',header='theta_deg,rate_Hz_per_0.02deg_bin',comments='')
    curve=np.array([[cm,calc(cm)[0]*3600] for cm in np.linspace(0,152.4,41)])
    np.savetxt(HERE/'concrete_scan.csv',curve,delimiter=',',header='concrete_cm,rate_per_hour',comments='')
    result={'cases':cases,'concrete_density_g_cm3':2.3,'coverage':'uniform horizontal slab covering all accepted directions','spectrum':'Guan et al. Eq. 3, E interpreted as total energy, normalized to PDG I(p>1 GeV/c,theta=0)=70 /m2/s/sr','spectrum_normalization_factor':norm,'normalization_threshold_total_energy_GeV':Eref,'exit_kinetic_energy_floor_MeV':10,'detector_stopping_model':'polystyrene CSDA proxy for the ROOT-derived detector mass column','detector_vertical_mass_column_g_cm2':Xdet,'detector_column_std_g_cm2':float(mass_vertical.std()),'detector_mass_budget_rays':len(columns),'mean_material_columns_g_cm2':{k:v/len(columns) for k,v in by_mat.items()},'roof_fractional_rate_reduction':1-cases[1]['rate_Hz']/cases[0]['rate_Hz'],'scattering_straggling_decay_detection_efficiency_included':False,'sensitivity':{'detector_column_half_and_double_per_hour':{str(cm):[calc(cm,det_scale=s)[0]*3600 for s in [.5,2]] for cm in [0,91.44]},'exit_100MeV_per_hour':{str(cm):calc(cm,Kout=.1)[0]*3600 for cm in [0,91.44]}}}
    (HERE/'shielding_results.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))
if __name__=='__main__':main()
