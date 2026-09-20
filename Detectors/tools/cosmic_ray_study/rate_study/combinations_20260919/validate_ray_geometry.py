#!/usr/bin/env python3
"""Reproduce independent hit/chord checks, including broad oblique HCAL rays."""
from pathlib import Path
import collections
import csv
import hashlib
import json
import subprocess
import sys
import numpy as np
from scipy.stats import qmc
import ray_geometry

HERE=Path(__file__).resolve().parent
BASE=HERE.parent
sys.path.insert(0,str(BASE))
import calculate_rate as reference


def main():
    out=HERE/'review'/'ray_geometry_checks';out.mkdir(parents=True,exist_ok=True)
    centered=BASE/'ecal_comparison'/'centered_only'
    model=ray_geometry.load(centered/'scene.json',BASE/'geometry'/'detector.gdml')
    rays=np.loadtxt(centered/'root_test_rays.csv',delimiter=',')
    result=model.trace(rays[:,:2],-rays[:,3:5]/rays[:,5:6],rays[0,2])
    expected=rays[:,6].astype(np.uint32)
    legacy_mismatch=int(np.count_nonzero(result['masks']!=expected))
    rootcolumns=collections.defaultdict(float)
    for row in csv.DictReader((centered/'material_chords.csv').open()):
        rootcolumns[int(row['ray'])]+=float(row['length_mm'])*model.densities[row['material']]/10
    ids=np.array(list(rootcolumns));old=np.array(list(rootcolumns.values()))
    massdiff=float(np.max(abs(result['material_columns'][ids]-old)))

    # Independent old vectorized intersection code, covering wide directions.
    u=qmc.Sobol(4,scramble=True,seed=20260919).random_base2(14)
    zref=3758.7109924317165
    xy=np.array([4000.,-1110.])+u[:,:2]*np.array([2050.,2050.])
    slope=(u[:,2:]-.5)*6
    broad=model.trace(xy,slope,zref)
    pymasks=np.zeros(len(xy),dtype=np.uint32)
    for j,layer in enumerate(model.layers):
        pymasks|=reference.crosses(layer,xy,slope,zref).astype(np.uint32)<<j
    broad_mismatch=int(np.count_nonzero(pymasks!=broad['masks']))

    # A new ROOT check uses original solids, with the same complete ECAL shift.
    # Mix broad accepted trajectories, null tracks, and existing narrow tracks.
    nonzero=np.flatnonzero(broad['masks']);zero=np.flatnonzero(broad['masks']==0)
    pick=np.r_[nonzero[np.linspace(0,len(nonzero)-1,96,dtype=int)],zero[np.linspace(0,len(zero)-1,32,dtype=int)]]
    origin=np.column_stack([xy[pick]+slope[pick]*(zref-4000),np.full(len(pick),4000.)])
    direction=np.column_stack([slope[pick],-np.ones(len(pick))]);direction/=np.linalg.norm(direction,axis=1)[:,None]
    broadrows=np.column_stack([origin,direction,broad['masks'][pick]])
    check=np.vstack([broadrows,rays[:32],rays[96:128]])
    np.savetxt(out/'root_test_rays.csv',check,delimiter=',',fmt=['%.15g']*6+['%d'])
    (out/'active_paths.tsv').write_bytes((centered/'active_paths.tsv').read_bytes())
    original=(BASE/'ecal_comparison'/'check_layout.C').read_text()
    macro=original.replace('void check_layout(', 'void check_material_geometry(')
    # The original narrow sample omits tracker parents; broad rays need parent
    # silicon, including its dead border, and must omit active daughters instead.
    macro=macro.replace('bool trackerParent=part.path.find("recoil_l14_sensor_vol_")!=std::string::npos && part.path.find("active_sensor")==std::string::npos;',
                        'bool trackerParent=part.path.find("active_sensor")!=std::string::npos;')
    macro=macro.replace('if(!trackerParent && expected==((1u<<nLayers)-1))chords', 'if(!trackerParent)chords')
    (out/'check_material_geometry.C').write_text(macro)
    shift=json.loads((centered/'comparison.json').read_text())['ecal_translation_mm']
    call=f'{out}/check_material_geometry.C("detector.gdml","{out}",false,{shift[0]:.15g},{shift[1]:.15g})'
    run=subprocess.run(['root','-l','-b','-q',call],cwd=BASE/'geometry',capture_output=True,text=True)
    (out/'root_check.log').write_text(run.stdout+run.stderr)
    if run.returncode:raise RuntimeError(f'ROOT validation failed: {run.returncode}')
    rootcheck=json.loads((out/'root_ray_check.json').read_text())
    cols=collections.defaultdict(float)
    rootcumulative=np.zeros((len(check),len(model.layers)))
    for row in csv.DictReader((out/'material_chords.csv').open()):
        i=int(row['ray']);rho=model.densities[row['material']]
        length=float(row['length_mm']);entry=float(row['entry_mm'])
        cols[i]+=length*rho/10
        distance=(model.layer_zlow-check[i,2])/check[i,5]
        rootcumulative[i]+=np.clip(distance-entry,0,length)*rho/10
    actual=model.trace(check[:,:2],-check[:,3:5]/check[:,5:6],4000.)
    rootmass=np.array([cols[i] for i in range(len(check))])
    broadmassdiff=float(np.max(abs(actual['material_columns']-rootmass)))
    cumulativediff=float(np.max(abs(actual['cumulative_columns']-rootcumulative)))
    monotonic=bool(np.all(np.diff(broad['cumulative_columns'],axis=1)>-1e-9))
    hierarchy=json.loads((centered/'scene.json').read_text())['objects']
    pairs=[(o['path'],q['path']) for o in hierarchy for q in hierarchy if q['path'].startswith(o['path']+'/')]
    checks={'legacy_centered_rays':len(rays),'legacy_mask_mismatches':legacy_mismatch,
        'legacy_ROOT_material_rays':len(ids),'legacy_material_max_abs_difference_g_cm2':massdiff,
        'centered_golden_mean_vertical_column_g_cm2':float(np.mean(result['material_columns'][ids]*abs(rays[ids,5]))),
        'broad_independent_python_rays':len(xy),'broad_python_mask_mismatches':broad_mismatch,
        'broad_ROOT_check':rootcheck,'broad_ROOT_material_max_abs_difference_g_cm2':broadmassdiff,
        'broad_ROOT_cumulative_material_max_abs_difference_g_cm2':cumulativediff,
        'broad_cumulative_columns_monotonic':monotonic,'material_hierarchy_pairs':pairs,
        'max_cumulative_excess_over_full_column_g_cm2':float(np.max(broad['cumulative_columns']-broad['material_columns'][:,None])),
        'model':model.metadata,'validation_not_transport':True}
    assert legacy_mismatch==broad_mismatch==rootcheck['mismatches']==0
    assert massdiff<1e-6 and broadmassdiff<1e-6 and cumulativediff<1e-6 and monotonic
    checks['passed']=True
    (HERE/'review'/'ray_geometry_validation.json').write_text(json.dumps(checks,indent=2)+'\n')
    print(json.dumps(checks,indent=2))


if __name__=='__main__':main()
