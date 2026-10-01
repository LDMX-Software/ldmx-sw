#!/usr/bin/env python3
"""Independent numerical checks for published coincidence and HCAL trigger rates."""
from pathlib import Path
import itertools
import json
import math
import numpy as np
from scipy.stats import qmc
import calculate_combinations as calc
import calculate_hcal_triggers as triggers
import ray_geometry

HERE=Path(__file__).resolve().parent
METRICS=['geometric','no_roof','concrete']


def compare(row,values):
    a=np.array(values);mean=a.mean(0);se=a.std(0,ddof=1)/math.sqrt(len(a));result={}
    for i,name in enumerate(METRICS):
        difference=mean[i]-row[name+'_per_hour'];combined=np.hypot(se[i],row[name+'_se_per_hour'])
        result[name]={'independent_rate_per_hour':float(mean[i]),'independent_se_per_hour':float(se[i]),
          'production_rate_per_hour':row[name+'_per_hour'],'difference_combined_se':float(difference/combined)}
    return result


def flux_at(flux,x,c,r):
    answer=np.empty_like(x);inside=(c>=flux.co[0])&(c<=flux.co[-1])&(np.log1p(x)<=flux.logx[-1])
    answer[inside]=flux(x[inside],c[inside],r)
    if np.any(~inside):answer[~inside]=flux.exact(x[~inside],c[~inside],[0,91.44][r])
    assert np.all(np.isfinite(answer))
    return answer


def main():
    data=json.loads((HERE/'results/results.json').read_text());specified={r['id']:r for r in data['specified']}
    incl={tuple(r[k] for k in ['hcal_mask','S','Y','E','tracker_k']):r for r in data['inclusive']}
    audit={'rate_monotonicity':{},'independent_IID':{},'HCAL_OR_logic_review':'Any layer in each required station; last required hit is the earliest hit layer of the lowest required station. All other hits are unrestricted.',
           'HCAL_OR_angle_checks':{}}
    for metric in METRICS:
        key=metric+'_per_hour';edges=0;bad=[];threshold_bad=[];negative=[];partition_residual=[]
        for i,r in specified.items():
            for k in range(10):
                j=i|(1<<k)
                if j==i or j not in specified:continue
                edges+=1
                if specified[j][key]>r[key]+1e-9:bad.append([i,j,specified[j][key]-r[key]])
        for k,r in incl.items():
            if k[-1]<4 and incl[k[:-1]+(k[-1]+1,)][key]>r[key]+1e-9:threshold_bad.append(k)
        for r in data['exclusive_tracker']:
            if r[key]<-1e-9:negative.append(r)
        for h,s,y,e in itertools.product(range(1,8),range(2),range(2),range(2)):
            exact=sum(r[key] for r in data['exclusive_tracker'] if (r['hcal_mask'],r['S'],r['Y'],r['E'])==(h,s,y,e))
            partition_residual.append(exact-incl[(h,s,y,e,0)][key])
        audit['rate_monotonicity'][metric]={'specified_subset_edges_checked':edges,'subset_violations':bad,
             'at_least_k_violations':threshold_bad,'negative_exclusive_rows':negative,
             'max_exclusive_partition_residual_per_hour':float(max(abs(x) for x in partition_residual))}
        assert not bad and not threshold_bad and not negative

    model=ray_geometry.load(HERE/'inputs/aligned_scene.json',HERE.parent/'geometry/detector.gdml')
    flux=calc.Flux();records,groups=calc.definitions(model.layers);definition={r['id']:r for r in records}
    for identifier,name in [(1023,'golden'),(7,'all_HCAL_layers')]:
        record=definition[identifier];values=[]
        for rep in range(4):
            u=np.random.default_rng(2026091950+identifier*10+rep).random((2**16,4))
            xy,sl,zref,c,w=calc.sample(u,record['anchor'],model.layers);trace=model.trace(xy,sl,zref)
            good=(trace['masks']&record['layer_bits'])==record['layer_bits'];col=trace['cumulative_columns'][:,record['last_layer']]
            values.append([np.mean(w*good)]+[np.mean(w*flux(col,c,r)/(70*c*c)*good) for r in [0,1]])
        audit['independent_IID'][name]={'power':16,'replicates':4,'comparison':compare(specified[identifier],values)}

    triggerdata=json.loads((HERE/'results/hcal_any_layer_triggers.json').read_text())
    triggerrows={r['trigger']:r for r in triggerdata['rows']}
    for ci,(name,stationgroups) in enumerate([('any_layer_stations_7',calc.UNITS[:3]),('any_station',[sum(calc.UNITS[:3],[])])]):
        values=[]
        for rep in range(4):
            u=qmc.Sobol(4,scramble=True,seed=2026091980+ci*10+rep).random_base2(17)
            u[:,0]**=2 # generate() maps u0^(1/4), hence c=sqrt(original u0).
            xy,sl,zref,c,w=triggers.generate(u,stationgroups,model.layers);w*=2*c*c # q(c)=2c Jacobian.
            trace=model.trace(xy,sl,zref);valid=np.ones(len(u),bool);last=np.zeros(len(u),int)
            for station in stationgroups:
                first=np.full(len(u),26,int)
                for layer in sorted(station,reverse=True):first=np.where((trace['masks']&(1<<layer))!=0,layer,first)
                valid &= first<26;last=np.maximum(last,np.minimum(first,25))
            col=trace['cumulative_columns'][np.arange(len(u)),last]
            values.append([np.mean(w*valid)]+[float(np.sum(w[valid]*flux_at(flux,col[valid],c[valid],r)/(70*c[valid]**2)))/len(u) for r in [0,1]])
        audit['HCAL_OR_angle_checks'][name]={'power':17,'replicates':4,'proposal':'q(c)=2c; any_station checked by direct union of all ten layers',
                                         'comparison':compare(triggerrows[name],values)}

    # Deterministic proof that the finite TS/LYSO anchor envelope is contained
    # in all four tracker planes, independently of sampled-ray statistics.
    A,B=model.layers[2],model.layers[13];D=A['z_mm']-B['z_mm']
    ha=max(p['half_z'] for p in A['prisms']);hb=max(p['half_z'] for p in B['prisms'])
    sm=np.maximum(abs(B['hi']-A['lo']),abs(B['lo']-A['hi']))/(D-ha-hb)
    alo=A['lo']-ha*sm-1e-7;ahi=A['hi']+ha*sm+1e-7;blo=B['lo']-hb*sm-1e-7;bhi=B['hi']+hb*sm+1e-7
    margins=[]
    for i in [6,7,8,9]:
        l=model.layers[i];f=(A['z_mm']-l['z_mm'])/D
        lo=(1-f)*alo+f*blo;hi=(1-f)*ahi+f*bhi
        corners=np.array([[x,y] for x in [lo[0],hi[0]] for y in [lo[1],hi[1]]]);eq=l['prisms'][0]['eq']
        margin=float(-np.max(corners@eq[:,:2].T+eq[:,2]));assert margin>0
        margins.append({'layer':i,'minimum_inside_margin_mm':margin})
    audit['tracker_plateau_proof']={'method':'All corners of the expanded TS1-top/LYSO-bottom anchor envelope at each tracker midplane lie strictly inside its convex active polygon.',
          'margins':margins,'consequence':'With TS and LYSO required, dropping tracker hit requirements does not enlarge straight-ray geometric acceptance.'}
    audit['all_checks_completed']=True
    (HERE/'review/numerical_audit.json').write_text(json.dumps(audit,indent=2)+'\n')
    print(json.dumps(audit,indent=2))


if __name__=='__main__':main()
