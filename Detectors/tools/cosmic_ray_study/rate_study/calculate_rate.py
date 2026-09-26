#!/usr/bin/env python3
"""Geometrical sea-level muon coincidence rate for the exact active GDML prisms.

Flux is per area perpendicular to the ray. This is not a transport or efficiency
calculation. All lengths entering the geometry are mm, and 1 mm^2 = 1e-6 m^2.
"""
import argparse, collections, hashlib, json, math, re, sys
from pathlib import Path
import numpy as np
from scipy.spatial import ConvexHull
from scipy.stats import qmc

HERE=Path(__file__).resolve().parent

def active(o):
    s,m,p=o['subsystem'],o['material'].lower(),o['path'].lower()
    return ((s=='hcal' and m=='scintillator') or
            (s=='trigger' and m=='polyvinyltoluene') or
            (s=='tracker' and 'active_sensor' in p) or
            (s=='ecal' and m=='silicon') or (s=='target' and m=='lyso'))

def prepare(scene,omit_ecal=False):
    grouped=collections.defaultdict(list)
    for o in scene['objects']:
        if not active(o) or (omit_ecal and o['subsystem']=='ecal'): continue
        v=np.array(o['vertices_mm']);z=float((v[:,2].min()+v[:,2].max())/2)
        grouped[o['subsystem'],round(z,4)].append(o)
    layers=[]
    for (s,z),objs in sorted(grouped.items(),key=lambda x:-x[0][1]):
        prisms=[]
        for o in objs:
            v=np.array(o['vertices_mm']);xy=np.unique(v[:,:2],axis=0)
            hull=ConvexHull(xy);poly=xy[hull.vertices];h=np.ptp(v[:,2])/2
            # All accepted solids are convex boxes or filled hexagonal prisms.
            assert o['shape'] in ('TGeoBBox','TGeoPgon')
            prisms.append({'poly':poly,'eq':hull.equations,'half_z':float(h)})
        vv=np.concatenate([np.array(o['vertices_mm']) for o in objs])
        z=float((vv[:,2].min()+vv[:,2].max())/2)
        lo,hi=vv[:,:2].min(0),vv[:,:2].max(0)
        if s=='hcal':
            # Adjacent bars tile a rectangle; verify equality before merging.
            areas=[ConvexHull(p['poly']).volume for p in prisms]
            assert np.isclose(sum(areas),np.prod(hi-lo),rtol=1e-8)
            poly=np.array([[lo[0],lo[1]],[hi[0],lo[1]],[hi[0],hi[1]],[lo[0],hi[1]]])
            prisms=[{'poly':poly,'eq':ConvexHull(poly).equations,'half_z':prisms[0]['half_z']}]
        layers.append({'subsystem':s,'z_mm':z,'lo':lo,'hi':hi,'prisms':prisms,
                       'physical_volumes':len(objs),'paths':[o['path'] for o in objs]})
    expected=dict(hcal=10,trigger=6,tracker=4,target=2)
    if not omit_ecal: expected['ecal']=4
    assert len(layers)==sum(expected.values())
    assert collections.Counter(x['subsystem'] for x in layers)==expected
    return layers

def crosses(layer,xy,slope,zref,mode='volume'):
    q=xy+slope*(zref-layer['z_mm'])
    hit=np.zeros(len(q),bool)
    for p in layer['prisms']:
        if mode=='midplane':
            ok=np.ones(len(q),bool)
            for nx,ny,b in p['eq']: ok &= nx*q[:,0]+ny*q[:,1]+b<=1e-8
        else:
            enter=np.full(len(q),-p['half_z']);leave=-enter
            ok=np.ones(len(q),bool)
            for nx,ny,b in p['eq']:
                val=nx*q[:,0]+ny*q[:,1]+b
                speed=nx*slope[:,0]+ny*slope[:,1]
                parallel=np.abs(speed)<1e-14
                ok &= ~parallel | (val<=1e-8)
                t=np.divide(-val,speed,out=np.zeros_like(val),where=~parallel)
                enter=np.where(speed < -1e-14,np.maximum(enter,t),enter)
                leave=np.where(speed > 1e-14,np.minimum(leave,t),leave)
            ok &= leave>enter+1e-10
        hit |= ok
    return hit

def evaluate(u,layers,anchor_a,anchor_b,bounds,mode='volume'):
    alo,ahi,blo,bhi=bounds;D=anchor_a['z_mm']-anchor_b['z_mm']
    a=alo+(ahi-alo)*u[:,:2];b=blo+(bhi-blo)*u[:,2:]
    slope=(b-a)/D;cosine=1/np.sqrt(1+(slope*slope).sum(1))
    base=np.prod(ahi-alo)*np.prod(bhi-blo)/D**2*1e-6
    weight=70.0*base*cosine**6
    masks=np.column_stack([crosses(l,a,slope,anchor_a['z_mm'],mode) for l in layers])
    selection=np.all(masks,axis=1)
    def mask(subs): return masks[:,[i for i,l in enumerate(layers) if l['subsystem'] in subs]].all(axis=1)
    cuts=[mask(set(x)) for x in [('trigger','target'),('trigger','target','tracker'),('trigger','target','tracker','ecal'),('trigger','target','tracker','ecal','hcal')]]
    rates=[float(np.mean(weight*m)) for m in cuts]
    return rates,selection,weight,cosine,a,slope,masks

def main():
    p=argparse.ArgumentParser();p.add_argument('--scene',type=Path,required=True);p.add_argument('--power',type=int,default=18);p.add_argument('--replicates',type=int,default=8);p.add_argument('--without-ecal',action='store_true',help='Require only the remaining 22 layers; use the companion study for material removal.');args=p.parse_args()
    scene=json.loads(args.scene.read_text());layers=prepare(scene,omit_ecal=args.without_ecal)
    A=next(l for l in layers if l['subsystem']=='trigger');B=[l for l in layers if l['subsystem']=='target'][-1]
    D=A['z_mm']-B['z_mm'];ha=A['prisms'][0]['half_z'];hb=B['prisms'][0]['half_z']
    maxs=np.maximum(abs(B['hi']-A['lo']),abs(B['lo']-A['hi']))/(D-ha-hb)
    bounds=(A['lo']-ha*maxs-1e-7,A['hi']+ha*maxs+1e-7,B['lo']-hb*maxs-1e-7,B['hi']+hb*maxs+1e-7)
    rates=[];mid=[];angles=[];weights=[];rays=[];checkrays=[]
    for seed in range(args.replicates):
        u=qmc.Sobol(4,scramble=True,seed=918260+seed).random_base2(args.power)
        rr,good,w,co,xy,sl,masks=evaluate(u,layers,A,B,bounds);rates.append(rr)
        mid.append(evaluate(u,layers,A,B,bounds,mode='midplane')[0][-1])
        angles.append(np.degrees(np.arccos(co[good])));weights.append(w[good]);rays.append(np.column_stack([xy[good],sl[good]]))
        if seed==0:
            ids=np.r_[np.flatnonzero(good)[:96],np.flatnonzero(~good)[:96]]
            for i in ids:
                origin=np.r_[xy[i]+sl[i]*(A['z_mm']-4000),4000.0]
                direction=np.r_[sl[i],-1.0];direction/=np.linalg.norm(direction)
                bits=sum(int(v)<<j for j,v in enumerate(masks[i]))
                checkrays.append([*origin,*direction,bits])
        print(f'replicate {seed+1}: {rr[-1]*3600:.6f} /hour',flush=True)
    rates=np.array(rates);mean=rates.mean(0);err=rates.std(0,ddof=1)/np.sqrt(len(rates))
    angle=np.concatenate(angles);weight=np.concatenate(weights);ray=np.concatenate(rays)
    bins=np.linspace(0,10,501)
    hist,_=np.histogram(angle,bins,weights=weight)
    np.savetxt(HERE/'angular_rate.csv',np.column_stack([(bins[:-1]+bins[1:])/2,hist/(args.replicates*2**args.power)]),delimiter=',',header='theta_deg,rate_Hz',comments='')
    order=np.argsort(angle);quantiles=np.interp([.5,.9,.95,.99],np.cumsum(weight[order])/weight.sum(),angle[order])
    # Independent pseudorandom integration cross-check with its ordinary MC error.
    rr,g,w,c,x,s,m=evaluate(np.random.default_rng(9182666).random((2**args.power,4)),layers,A,B,bounds)
    mcerr=float(np.std(w*g,ddof=1)/np.sqrt(len(w)))
    summary={'definition':f'intersect a nonzero path length in at least one active volume in every one of {len(layers)} layers','flux':{'I0_m2_s_sr':70,'angular_power':2,'momentum_threshold_GeV_c':1,'model':'PDG 2022 sea-level reference; azimuth symmetric; no shield or survival correction'},'N_per_scramble':2**args.power,'scrambles':args.replicates,'seed_start':918260,'rate_Hz':float(mean[-1]),'numerical_SE_Hz':float(err[-1]),'rate_per_hour':float(mean[-1]*3600),'numerical_SE_per_hour':float(err[-1]*3600),'rate_per_day':float(mean[-1]*86400),'mean_wait_minutes':float(1/mean[-1]/60),'geometrical_factor_m2_sr':float(mean[-1]/70),'angular_quantiles_deg':dict(zip(['50','90','95','99'],map(float,quantiles))),'max_sampled_zenith_deg':float(angle.max()),'midplane_rate_per_hour':float(np.mean(mid)*3600),'cutflow_labels':['6 trigger + 2 LYSO layers','plus 4 tracker sensors','ECal omitted' if args.without_ecal else 'plus 4 ECal silicon layers','plus all 10 HCal layers'],'cutflow_Hz':mean.tolist(),'cutflow_numerical_SE_Hz':err.tolist(),'independent_MC_Hz':rr[-1],'independent_MC_SE_Hz':mcerr,'source_scene_sha256':hashlib.sha256(args.scene.read_bytes()).hexdigest(),'anchor_z_mm':[A['z_mm'],B['z_mm']],'anchor_distance_mm':D,'anchor_bounds_mm':[x.tolist() for x in bounds],'shielding_status':'This geometric reference is unshielded; see shielding_results.json for the 3 ft comparison','geant4_transport_or_detector_efficiency_included':False}
    (HERE/'results.json').write_text(json.dumps(summary,indent=2)+'\n')
    # Keep a compact weighted trajectory sample for figures, not every trial.
    ids=np.linspace(0,len(ray)-1,min(20000,len(ray)),dtype=int)
    np.savez_compressed(HERE/'accepted_rays.npz',ray=ray[ids],weights=weight[ids],theta=angle[ids],zref=A['z_mm'])
    ledger=[]
    for i,l in enumerate(layers):
        ledger.append({'layer':i,'subsystem':l['subsystem'],'z_mm':l['z_mm'],'xy_bbox_mm':[l['lo'].tolist(),l['hi'].tolist()],'physical_volumes':l['physical_volumes'],'polygons':[p['poly'].tolist() for p in l['prisms']]})
    (HERE/'layer_inventory.json').write_text(json.dumps(ledger,indent=2)+'\n')
    with (HERE/'root_test_rays.csv').open('w') as f:
        for r in checkrays: f.write(','.join(str(v) for v in r)+'\n')
    with (HERE/'active_paths.tsv').open('w') as f:
        for i,l in enumerate(layers):
            for path in l['paths']: f.write(f'{i}\t{path}\n')
    print(json.dumps(summary,indent=2))

if __name__=='__main__': main()
