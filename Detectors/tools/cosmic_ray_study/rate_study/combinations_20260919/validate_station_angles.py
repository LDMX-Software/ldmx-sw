#!/usr/bin/env python3
"""Independent q(c)=2c angular integration of single-station HCAL coincidences.

This gives near-horizontal directions more weight than the production q(c)=4c^3
sampler and checks its convergence under the modified-Gaisser spectrum.
"""
from pathlib import Path
import json
import math
import numpy as np
from scipy.stats import qmc
import calculate_combinations as calc
import ray_geometry

HERE=Path(__file__).resolve().parent


def main():
    model=ray_geometry.load(HERE/'inputs/aligned_scene.json',HERE.parent/'geometry/detector.gdml')
    flux=calc.Flux();records=[];power=17;nrep=4
    for h,indices in [(1,calc.UNITS[0]),(2,calc.UNITS[1]),(4,calc.UNITS[2])]:
        estimates=[]
        zref=model.layers[indices[0]]['z_mm'];required=calc.bits(indices)
        for rep in range(nrep):
            u=qmc.Sobol(4,scramble=True,seed=2026091910+h*10+rep).random_base2(power)
            co=np.sqrt(u[:,0]);phi=2*np.pi*u[:,1]
            slope=np.sqrt(1/co**2-1)[:,None]*np.column_stack([np.cos(phi),np.sin(phi)])
            lo=np.full((len(u),2),-np.inf);hi=-lo
            for i in indices:
                l=model.layers[i];half=max(p['half_z'] for p in l['prisms'])
                shift=slope*(zref-l['z_mm'])
                lo=np.maximum(lo,l['lo']-shift-abs(slope)*half)
                hi=np.minimum(hi,l['hi']-shift+abs(slope)*half)
            span=np.maximum(hi-lo,0);xy=lo+span*u[:,2:]
            trace=model.trace(xy,slope,zref);good=(trace['masks']&required)==required
            area=np.prod(span,axis=1)*1e-6
            base=70*np.pi*co**2*area*3600
            col=trace['cumulative_columns'][:,max(indices)]
            values=[np.mean(base*good)]
            for roof in [0,1]:values.append(np.mean(base*flux(col,co,roof)/(70*co**2)*good))
            estimates.append(values)
        a=np.array(estimates);mean=a.mean(0);se=a.std(0,ddof=1)/math.sqrt(nrep)
        row={'hcal_mask':h,'replicates':nrep,'power':power,'proposal':'q(c)=2c, uniform azimuth',
             'geometric_per_hour':float(mean[0]),'geometric_se_per_hour':float(se[0]),
             'no_roof_per_hour':float(mean[1]),'no_roof_se_per_hour':float(se[1]),
             'concrete_per_hour':float(mean[2]),'concrete_se_per_hour':float(se[2])}
        records.append(row);print(json.dumps(row),flush=True)
    result={'definition':'All layers in exactly the named required station; all other devices unrestricted and installed.',
            'purpose':'Independent angular-proposal convergence check, especially the near-horizontal tail.',
            'rows':records}
    (HERE/'review'/'station_angle_validation.json').write_text(json.dumps(result,indent=2)+'\n')


if __name__=='__main__':main()
