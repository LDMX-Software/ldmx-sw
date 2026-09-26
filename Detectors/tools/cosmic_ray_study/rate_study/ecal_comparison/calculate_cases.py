#!/usr/bin/env python3
"""Compare ECal-only centering and physical ECal removal at original heights.

Use the report's integration, spectrum and range tables. Removing ECal drops
its four hit requirements AND its material subtree. Recompute the remaining
ROOT mass column for this case; retain the nominal column for centered ECal,
as in the earlier position-only comparison.
"""
from pathlib import Path
import collections,copy,csv,hashlib,json,subprocess,sys
import xml.etree.ElementTree as ET
import numpy as np
HERE=Path(__file__).resolve().parent
BASE=HERE.parent
sys.path.insert(0,str(BASE))
import calculate_rate as geometry
import shielding


def run_case(name,remove_ecal=False):
    folder=HERE/name
    folder.mkdir(exist_ok=True)
    original=json.loads((BASE/'geometry/root_scene.json').read_text())
    scene={'units':original['units'],'objects':[]}
    shift=json.loads((BASE/'shift_up_200mm/ecal_centered/comparison.json').read_text())['ecal_translation_mm']
    shift=np.zeros(3) if remove_ecal else np.array(shift)
    for before in original['objects']:
        if remove_ecal and before['subsystem']=='ecal':continue
        obj={k:copy.deepcopy(before[k]) for k in ('path','subsystem','material','shape','vertices_mm')}
        if obj['subsystem']=='ecal':obj['vertices_mm']=(np.array(obj['vertices_mm'])+shift).tolist()
        scene['objects'].append(obj)
    scene['variation']={'assembly_shift_mm':0,'ecal_removed':remove_ecal,
                        'ecal_translation_mm':shift.tolist(),
                        'source_scene_sha256':hashlib.sha256((BASE/'geometry/root_scene.json').read_bytes()).hexdigest()}
    path=folder/'scene.json'
    path.write_text(json.dumps(scene)+'\n')
    layers=geometry.prepare(scene,omit_ecal=remove_ecal)
    nominal_layers={tuple(sorted(l['paths'])):l for l in geometry.prepare(original)}
    for layer in layers:
        before=nominal_layers[tuple(sorted(layer['paths']))]
        assert abs(layer['z_mm']-before['z_mm'])<1e-8
        delta=shift[:2] if layer['subsystem']=='ecal' else np.zeros(2)
        assert np.allclose(layer['lo']-before['lo'],delta,rtol=0,atol=1e-8)
        assert np.allclose(layer['hi']-before['hi'],delta,rtol=0,atol=1e-8)
    geometry.HERE=folder
    sys.argv=['calculate_rate.py','--scene',str(path)] + (['--without-ecal'] if remove_ecal else [])
    geometry.main()
    rate_summary=json.loads((folder/'results.json').read_text())
    rate_summary['shielding_status']='See comparison.json for the roof comparison and material treatment.'
    (folder/'results.json').write_text(json.dumps(rate_summary,indent=2)+'\n')
    macro=f'../ecal_comparison/check_layout.C("detector.gdml","../ecal_comparison/{name}",{str(remove_ecal).lower()},{shift[0]:.15g},{shift[1]:.15g})'
    with (folder/'root_check.log').open('w') as log:
        subprocess.run(['root','-l','-b','-q',macro],cwd=BASE/'geometry',stdout=log,stderr=subprocess.STDOUT,check=True)
    check=json.loads((folder/'root_ray_check.json').read_text())
    assert check['mismatches']==0 and check['layers']==len(layers)
    reference=json.loads((BASE/'shielding_results.json').read_text())
    mass=reference['detector_vertical_mass_column_g_cm2']
    material_note='Original 32.70 g/cm2 proxy held fixed, consistent with the earlier position studies.'
    mass_std=None
    if remove_ecal:
        doc=ET.parse(BASE/'geometry/detector.gdml')
        rho={m.get('name'):float(m.find('D').get('value')) for m in doc.findall('materials/material')}
        columns=collections.defaultdict(float)
        with (folder/'material_chords.csv').open() as f:
            for row in csv.DictReader(f):
                assert 'native_ecal' not in row['path']
                columns[int(row['ray'])]+=float(row['length_mm'])*rho[row['material']]/10
        rays=np.loadtxt(folder/'root_test_rays.csv',delimiter=',')
        vertical=np.array([v*abs(rays[k,5]) for k,v in columns.items()])
        assert len(vertical)==96 and np.all(vertical>0)
        mass=float(vertical.mean());mass_std=float(vertical.std())
        material_note='ROOT-derived mean column for 96 accepted rays after excluding the entire native_ecal subtree.'
    data=np.loadtxt(folder/'angular_rate.csv',delimiter=',',skiprows=1)
    theta,rate_geo=data[data[:,1]>0].T
    co=np.cos(np.deg2rad(theta))
    rc,ic=shielding.range_table('concrete');rp,ip=shielding.range_table('polystyrene')
    def rates(detector_mass):
        out=[]
        kdet=np.exp(ip(np.log(np.exp(rp(np.log(.010)))+detector_mass/co)))
        for original_case in reference['cases']:
            cm=original_case['concrete_cm']
            surface=np.exp(ic(np.log(np.exp(rc(np.log(kdet)))+2.3*cm/co))) if cm else kdet
            flux=np.array([reference['spectrum_normalization_factor']*shielding.integral(k+shielding.MUON_MASS_GEV,c) for k,c in zip(surface,co)])
            hz=float(np.sum(rate_geo*flux/(70*co**2)))
            out.append({'label':original_case['label'],'concrete_cm':cm,'rate_per_hour':hz*3600,
                        'rate_per_day':hz*86400,'fractional_change_from_original':hz/original_case['rate_Hz']-1})
        return out
    result={**scene['variation'],'required_layers':len(layers),'cases':rates(mass),
            'detector_vertical_mass_column_g_cm2':mass,'detector_column_std_g_cm2':mass_std,
            'mass_model':material_note,'root_cross_check':check,
            'no_scattering_straggling_decay_or_detector_efficiency':True}
    if remove_ecal:
        # Separately expose the geometric gain at fixed stopping mass. This is
        # not a claim that removing only a trigger condition removes material.
        result['fixed_nominal_mass_comparison']=rates(reference['detector_vertical_mass_column_g_cm2'])
    (folder/'comparison.json').write_text(json.dumps(result,indent=2)+'\n')
    print(name,json.dumps(result,indent=2),flush=True)


if __name__=='__main__':
    run_case('centered_only')
    run_case('without_ecal',remove_ecal=True)
