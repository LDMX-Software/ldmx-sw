#!/usr/bin/env python3
"""Inclusive HCAL-trigger combinations in the installed, aligned ESA stand.

Units: geometry mm; column g/cm2; flux m^-2 s^-1 sr^-1; exported rates hour^-1.
The stopping calculation is a mixed-material mass-column/polystyrene proxy,
not Geant4 transport or detector response. Omitted requirements remove no matter.
"""
from pathlib import Path
import argparse, collections, csv, hashlib, itertools, json, math, sys, time
import numpy as np
from scipy.integrate import quad
from scipy.interpolate import PchipInterpolator, RegularGridInterpolator
from scipy.stats import qmc
import legacy_geometry as geo

HERE = Path(__file__).resolve().parent
UNITS = [[0,1],[14,15,16,17],[22,23,24,25],[2,3,4,5,10,11],
         [12,13],[18,19,20,21],[6],[7],[8],[9]]
UNIT_NAMES = ['H_T','H_M','H_B','S','Y','E','T1','T2','T3','T4']
MASS = .10565839

def spectrum(E, co):
    cs = np.sqrt((co*co+.102573**2-.068287*co**.958633+.0407253*co**.817285)
                 /(1+.102573**2-.068287+.0407253))
    return 1e4*.14*(E+3.64/cs**1.29)**(-2.7)*(1/(1+1.1*E*cs/115)+.054/(1+1.1*E*cs/850))

def integrated_flux(E, co, order=96):
    E, co = np.broadcast_arrays(E, co)
    nodes, weights = np.polynomial.legendre.leggauss(order)
    out = np.zeros_like(E, dtype=float)
    scale=E+4.
    for t, w in zip((nodes+1)/2, weights/2):
        out += w*spectrum(E+scale*t/(1-t), co)*scale/(1-t)**2
    return out

def ranges(name):
    rows=[]
    for line in (HERE/'inputs'/f'{name}.txt').read_text().splitlines():
        try: a=list(map(float,line.split()))
        except ValueError: continue
        if len(a)>=9 and a[0]>=10 and a[8]>0: rows.append(a)
    a=np.array(rows)
    r=PchipInterpolator(np.log(a[:,0]/1000),np.log(a[:,8]),extrapolate=False)
    k=PchipInterpolator(np.log(a[:,8]),np.log(a[:,0]/1000),extrapolate=False)
    return lambda x:np.exp(r(np.log(x))), lambda x:np.exp(k(np.log(x)))

class Flux:
    def __init__(self):
        self.norm=70/quad(lambda e:float(spectrum(e,1.)),math.sqrt(1+MASS*MASS),np.inf,
                         epsabs=1e-8,epsrel=1e-10)[0]
        self.rp,self.kp=ranges('polystyrene');self.rc,self.kc=ranges('concrete')
        self.co=np.unique(np.r_[np.geomspace(.001,.05,60),np.linspace(.05,1,240)])
        self.logx=np.linspace(0,np.log1p(2e5),360)
        co,x=np.meshgrid(self.co,np.expm1(self.logx),indexing='ij')
        self.tables=[]
        for roof in [0,91.44]:
            f=self.exact(x,co,roof)
            assert np.all(np.isfinite(f)) and np.all(f>0)
            self.tables.append(RegularGridInterpolator((self.co,self.logx),np.log(f),bounds_error=True))
        rng=np.random.default_rng(20260919)
        c=rng.uniform(.03,1,250); x=np.expm1(rng.uniform(0,np.log1p(3000),250))
        self.max_interp_relative_error=max(float(np.max(np.abs(self(x,c,r)/self.exact(x,c,[0,91.44][r])-1))) for r in [0,1])
        probe_e=np.geomspace(.116,500,20);probe_c=np.linspace(.02,1,20)
        truth=np.array([quad(lambda e:float(spectrum(e,c)),e,np.inf,epsabs=1e-9,epsrel=1e-9)[0] for e,c in zip(probe_e,probe_c)])
        self.max_quadrature_relative_error=float(np.max(abs(integrated_flux(probe_e,probe_c)/truth-1)))
        assert self.max_interp_relative_error<.005 and self.max_quadrature_relative_error<.005
    def exact(self,x,co,roof):
        k=self.kp(self.rp(.010)+x)
        if roof: k=self.kc(self.rc(k)+2.3*roof/co)
        return self.norm*integrated_flux(k+MASS,co)
    def __call__(self,x,co,roof):
        points=np.column_stack([np.ravel(co),np.log1p(np.ravel(x))])
        return np.exp(self.tables[roof](points)).reshape(np.shape(x))

def bits(required):
    return sum(1<<i for i in required)

def definitions(layers):
    records=[];groups=collections.defaultdict(list)
    for h in range(1,8):
        for other in range(128):
            unitmask=h|(other<<3)
            required=sorted(i for k,inds in enumerate(UNITS) if (unitmask>>k)&1 for i in inds)
            if other==0 and h in [1,2,4]: anchor=('direction',h)
            else:
                candidates=[]
                for a,b in itertools.combinations(required,2):
                    A,B=layers[a],layers[b];D=A['z_mm']-B['z_mm']
                    candidates.append((np.prod(A['hi']-A['lo'])*np.prod(B['hi']-B['lo'])/D**2,a,b))
                _,a,b=min(candidates);anchor=('pair',a,b)
            rec={'id':unitmask,'hcal_mask':h,'S':(unitmask>>3)&1,'Y':(unitmask>>4)&1,
                 'E':(unitmask>>5)&1,'tracker_mask':unitmask>>6,'required_layers':required,
                 'layer_bits':bits(required),'last_layer':max(required),'anchor':anchor,
                 'requirements':'+'.join(n for k,n in enumerate(UNIT_NAMES) if (unitmask>>k)&1)}
            records.append(rec);groups[anchor].append(rec)
    return records,groups

def sample(u,anchor,layers):
    if anchor[0]=='pair':
        A,B=[layers[i] for i in anchor[1:]];D=A['z_mm']-B['z_mm']
        ha=max(p['half_z'] for p in A['prisms']);hb=max(p['half_z'] for p in B['prisms'])
        smax=np.maximum(abs(B['hi']-A['lo']),abs(B['lo']-A['hi']))/(D-ha-hb)
        alo=A['lo']-ha*smax-1e-7;ahi=A['hi']+ha*smax+1e-7
        blo=B['lo']-hb*smax-1e-7;bhi=B['hi']+hb*smax+1e-7
        xy=alo+(ahi-alo)*u[:,:2];b=blo+(bhi-blo)*u[:,2:]
        slope=(b-xy)/D;co=1/np.sqrt(1+(slope*slope).sum(1))
        w=70*np.prod(ahi-alo)*np.prod(bhi-blo)/D**2*1e-6*co**6*3600
        return xy,slope,A['z_mm'],co,w
    required=UNITS[{1:0,2:1,4:2}[anchor[1]]];zref=layers[min(required)]['z_mm']
    co=np.maximum(u[:,0],1e-16)**.25;phi=2*np.pi*u[:,1]
    slope=np.sqrt(1/co**2-1)[:,None]*np.column_stack([np.cos(phi),np.sin(phi)])
    lo=np.full((len(u),2),-np.inf);hi=-lo
    for i in required:
        l=layers[i];h=max(p['half_z'] for p in l['prisms']);shift=slope*(zref-l['z_mm'])
        lo=np.maximum(lo,l['lo']-shift-abs(slope)*h)
        hi=np.minimum(hi,l['hi']-shift+abs(slope)*h)
    span=np.maximum(hi-lo,0);xy=lo+span*u[:,2:]
    w=70*np.pi/2*np.prod(span,axis=1)*1e-6*3600
    return xy,slope,zref,co,w

METRICS=['geometric','no_roof','concrete','half_column_concrete','double_column_concrete','column_weighted','angle_weighted']

def export(records,raw,out,metadata):
    nrep=raw.shape[1];idx={r['id']:i for i,r in enumerate(records)}
    def stats(v):
        m=v.mean(0);se=v.std(0,ddof=1)/math.sqrt(nrep)
        return {f'{name}_per_hour':float(m[k]) for k,name in enumerate(METRICS[:5])}|{
            f'{name}_se_per_hour':float(se[k]) for k,name in enumerate(METRICS[:5])}
    detailed=[]
    for i,r in enumerate(records):
        row={k:r[k] for k in ['id','hcal_mask','S','Y','E','tracker_mask','requirements']};row.update(stats(raw[i]))
        row['mean_column_g_cm2']=float(raw[i,:,5].mean()/raw[i,:,1].mean())
        row['mean_zenith_deg']=float(raw[i,:,6].mean()/raw[i,:,1].mean())
        detailed.append(row)
    inclusive=[];exclusive=[]
    for h in range(1,8):
        for s,y,e in itertools.product([0,1],repeat=3):
            sums=np.zeros((5,nrep,len(METRICS)))
            for t in range(16):
                sums[bin(t).count('1')]+=raw[idx[h+(s<<3)+(y<<4)+(e<<5)+(t<<6)]]
            for k in range(5):
                v=sums[0] if k==0 else sum((-1)**(j-k)*math.comb(j-1,k-1)*sums[j] for j in range(k,5))
                x=sum((-1)**(j-k)*math.comb(j,k)*sums[j] for j in range(k,5))
                common={'hcal_mask':h,'S':s,'Y':y,'E':e,'tracker_k':k}
                inclusive.append(common|stats(v));exclusive.append(common|stats(x))
    for name,rows in [('specified_subsets',detailed),('inclusive_combinations',inclusive),('exclusive_tracker_counts',exclusive)]:
        with (out/f'{name}.csv').open('w',newline='') as f:
            writer=csv.DictWriter(f,fieldnames=list(rows[0]));writer.writeheader();writer.writerows(rows)
    (out/'results.json').write_text(json.dumps(metadata|{'specified':detailed,'inclusive':inclusive,'exclusive_tracker':exclusive},indent=2)+'\n')
    np.savez_compressed(out/'replicate_rates.npz',rates=raw,ids=np.array([r['id'] for r in records]),metrics=METRICS)

def main():
    p=argparse.ArgumentParser();p.add_argument('--power',type=int,default=16);p.add_argument('--replicates',type=int,default=6)
    p.add_argument('--output',type=Path,default=HERE/'results');p.add_argument('--only-anchor',type=int);p.add_argument('--iid',action='store_true')
    args=p.parse_args();args.output.mkdir(exist_ok=True,parents=True)
    import ray_geometry
    scene=HERE/'inputs/aligned_scene.json';gdml=HERE.parent/'geometry/detector.gdml'
    model=ray_geometry.load(scene,gdml);layers=geo.prepare(json.loads(scene.read_text()))
    identity_files=[Path(__file__),HERE/'ray_geometry.py',HERE/'ray_geometry.cpp',HERE/'legacy_geometry.py',scene,gdml,HERE/'inputs/concrete.txt',HERE/'inputs/polystyrene.txt']
    fingerprint=hashlib.sha256(b''.join(hashlib.sha256(f.read_bytes()).digest() for f in identity_files)).hexdigest()
    records,groups=definitions(layers);indices={r['id']:i for i,r in enumerate(records)}
    flux=Flux();raw=np.zeros((len(records),args.replicates,len(METRICS)));start=time.time()
    cache=args.output/'checkpoints';cache.mkdir(exist_ok=True)
    checkpoints=[]
    for ai,(anchor,recs) in enumerate(groups.items()):
        if args.only_anchor is not None and ai!=args.only_anchor: continue
        fn=cache/f'anchor_{ai:02d}_p{args.power}_r{args.replicates}_{"iid" if args.iid else "sobol"}_{fingerprint[:16]}.npz'
        if fn.exists():
            saved=np.load(fn);values=saved['values'];assert np.array_equal(saved['ids'],[r['id'] for r in recs])
        else:
            values=np.zeros((len(recs),args.replicates,len(METRICS)))
            for rep in range(args.replicates):
                seed=202609190+ai*100+rep
                u=np.random.default_rng(seed).random((2**args.power,4)) if args.iid else qmc.Sobol(4,scramble=True,seed=seed).random_base2(args.power)
                xy,sl,zref,co,w=sample(u,anchor,layers)
                traced=model.trace(xy,sl,zref);masks=traced['masks'];columns=traced['cumulative_columns']
                for last in sorted({r['last_layer'] for r in recs}):
                    col=columns[:,last]
                    w0=w*flux(col,co,0)/(70*co**2);wc=w*flux(col,co,1)/(70*co**2)
                    wh=w*flux(col*.5,co,1)/(70*co**2);wd=w*flux(col*2,co,1)/(70*co**2)
                    weight=np.column_stack([w,w0,wc,wh,wd,w0*col,w0*np.degrees(np.arccos(co))])
                    for ri,r in enumerate(recs):
                        if r['last_layer']!=last:continue
                        good=(masks&r['layer_bits'])==r['layer_bits']
                        values[ri,rep]=np.sum(weight[good],axis=0)/len(u)
                if ai in [0,len(groups)-1]:print(f'anchor {ai+1}/{len(groups)} rep {rep+1}: {time.time()-start:.1f}s',flush=True)
            np.savez_compressed(fn,ids=[r['id'] for r in recs],values=values)
        for i,r in enumerate(recs):raw[indices[r['id']]]=values[i]
        checkpoints.append({'index':ai,'anchor':anchor,'rows':len(recs)})
        print(f'anchor {ai+1}/{len(groups)} {anchor}: {len(recs)} selections; elapsed {time.time()-start:.1f}s',flush=True)
    if args.only_anchor is not None:return
    metadata={'model':'Straight rays, installed aligned geometry, per-ray mass column as polystyrene CSDA; no efficiency/scattering/decay',
              'power':args.power,'replicates':args.replicates,'points_per_anchor':2**args.power*args.replicates,
              'anchor_count':len(groups),'total_rays':len(groups)*2**args.power*args.replicates,'iid':args.iid,
              'seed_formula':'202609190 + 100 * anchor_index + replicate_index','elapsed_seconds':time.time()-start,
              'scene_sha256':hashlib.sha256(scene.read_bytes()).hexdigest(),'calculation_fingerprint':fingerprint,'flux_normalization':flux.norm,
              'max_flux_interpolation_relative_error':flux.max_interp_relative_error,
              'max_flux_quadrature_relative_error':flux.max_quadrature_relative_error,
              'units':UNIT_NAMES,'unit_layers':UNITS,'anchors':checkpoints,'roof_cm':91.44,'roof_density_g_cm3':2.30,
              'exit_kinetic_energy_floor_GeV':.010,'table_semantics':'S/Y/E and specified tracker subsets are required hits, all other hits unrestricted; equipment remains installed'}
    export(records,raw,args.output,metadata)
    print(json.dumps({k:v for k,v in metadata.items() if k not in ['anchors','unit_layers']},indent=2))

if __name__=='__main__':main()
