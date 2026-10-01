#!/usr/bin/env python3
"""HCAL triggers accepting any layer per station, and individual-layer singles.

Station ORs count one muon once. All installed material remains. A hit means a
positive geometrical chord, with the same 10 MeV residual energy convention.
"""
from pathlib import Path
import argparse,csv,json,math,time
import numpy as np
from scipy.stats import qmc
import calculate_combinations as calc
import ray_geometry
HERE=Path(__file__).resolve().parent

def generate(u,groups,layers):
    co=np.maximum(u[:,0],1e-16)**.25;phi=2*np.pi*u[:,1]
    sl=np.sqrt(1/co**2-1)[:,None]*np.column_stack([np.cos(phi),np.sin(phi)])
    zref=max(layers[min(g)]['z_mm'] for g in groups)
    lo=np.full((len(u),2),-np.inf);hi=-lo
    for group in groups:
        # Bound the UNION of projected layer envelopes in this station.
        glo=np.full_like(lo,np.inf);ghi=-glo
        for i in group:
            l=layers[i];h=max(p['half_z'] for p in l['prisms']);shift=sl*(zref-l['z_mm'])
            glo=np.minimum(glo,l['lo']-shift-abs(sl)*h)
            ghi=np.maximum(ghi,l['hi']-shift+abs(sl)*h)
        lo=np.maximum(lo,glo);hi=np.minimum(hi,ghi)
    span=np.maximum(hi-lo,0)
    return lo+span*u[:,2:],sl,zref,co,70*np.pi/2*np.prod(span,axis=1)*1e-6*3600

def main():
    p=argparse.ArgumentParser();p.add_argument('--power',type=int,default=17);p.add_argument('--replicates',type=int,default=8);args=p.parse_args()
    out=HERE/'results';out.mkdir(exist_ok=True)
    model=ray_geometry.load(HERE/'inputs/aligned_scene.json',HERE.parent/'geometry/detector.gdml')
    flux=calc.Flux();rows=[];raw=[];start=time.time()
    cases=[(f'any_layer_stations_{h}',[calc.UNITS[k] for k in range(3) if (h>>k)&1]) for h in range(1,8)]
    cases +=[(f'layer_{i}',[[i]]) for i in calc.UNITS[0]+calc.UNITS[1]+calc.UNITS[2]]
    for ci,(name,groups) in enumerate(cases):
        reps=[]
        for rep in range(args.replicates):
            u=qmc.Sobol(4,scramble=True,seed=202609590+ci*100+rep).random_base2(args.power)
            xy,sl,zref,co,w=generate(u,groups,model.layers);tr=model.trace(xy,sl,zref)
            valid=np.ones(len(u),bool);last=np.zeros(len(u),int)
            for group in groups:
                first=np.full(len(u),26,int)
                for i in sorted(group,reverse=True):first=np.where((tr['masks']&(1<<i))!=0,i,first)
                valid &= first<26;last=np.maximum(last,np.minimum(first,25))
            col=tr['cumulative_columns'][np.arange(len(u)),last]
            reps.append([float(np.sum(w[valid]))/len(u)]+[float(np.sum(w[valid]*flux(col[valid],co[valid],r)/(70*co[valid]**2)))/len(u) for r in [0,1]])
        raw.append(reps)
        print(name,np.mean(reps,axis=0).tolist(),f'{time.time()-start:.1f}s',flush=True)
    raw=np.array(raw)
    # Inclusive union rates from measured station intersections. Intersections
    # include the appropriate last-hit energy condition, so inclusion-exclusion applies.
    union=raw[0]+raw[1]+raw[3]-raw[2]-raw[4]-raw[5]+raw[6]
    two=raw[2]+raw[4]+raw[5]-2*raw[6]
    names=[x[0] for x in cases]+['any_station','at_least_two_stations']
    raw=np.concatenate([raw,union[None],two[None]],axis=0)
    strict=np.load(out/'replicate_rates.npz')
    byid={int(i):strict['rates'][k,:,:3] for k,i in enumerate(strict['ids'])}
    strict_union=byid[1]+byid[2]+byid[4]-byid[3]-byid[5]-byid[6]+byid[7]
    strict_two=byid[3]+byid[5]+byid[6]-2*byid[7]
    assert strict_union.shape[0]==args.replicates, 'Use the same scramble count for equipment and trigger studies.'
    names+=['strict_any_station','strict_at_least_two_stations']
    raw=np.concatenate([raw,strict_union[None],strict_two[None]],axis=0)
    for name,reps in zip(names,raw):
        mean=reps.mean(0);err=reps.std(0,ddof=1)/math.sqrt(args.replicates)
        row={'trigger':name}
        for k,metric in enumerate(['geometric','no_roof','concrete']):row[metric+'_per_hour']=float(mean[k]);row[metric+'_se_per_hour']=float(err[k])
        rows.append(row)
    with (out/'hcal_any_layer_triggers.csv').open('w',newline='') as f:
        w=csv.DictWriter(f,fieldnames=list(rows[0]));w.writeheader();w.writerows(rows)
    (out/'hcal_any_layer_triggers.json').write_text(json.dumps({'power':args.power,'replicates':args.replicates,'rows':rows,'seed_formula':'202609590 + 100*case_index + replicate','elapsed_seconds':time.time()-start},indent=2)+'\n')
    np.savez_compressed(out/'hcal_trigger_replicates.npz',rates=raw,names=names)

if __name__=='__main__':main()
