#!/usr/bin/env python3
"""Rate-only +200 mm translation of all trigger, tracker and LYSO active layers.

Reuse the baseline integration and stopping model. The production GDML stays
unchanged. Keep the detector mass-column proxy fixed to isolate the acceptance
change; a translated mechanical/Geant4 model is outside this comparison.
"""
from pathlib import Path
import argparse
import copy
import hashlib
import json
import math
import subprocess
import sys

HERE = Path(__file__).resolve().parent
BASE = HERE.parent
sys.path.insert(0, str(BASE))
import numpy as np
import calculate_rate as geometry
import shielding


def main():
    global HERE
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--power', type=int, default=18)
    parser.add_argument('--replicates', type=int, default=8)
    parser.add_argument('--reuse-integration', action='store_true',
                        help='Reuse saved rays only if their scene hash and sampling settings match.')
    parser.add_argument('--center-ecal', action='store_true',
                        help='Also align the complete ECal active envelope with the LYSO axis in X/Y.')
    args = parser.parse_args()
    if args.center_ecal:
        HERE = HERE/'ecal_centered'
        HERE.mkdir(exist_ok=True)
    source = BASE / 'geometry/root_scene.json'
    scene = json.loads(source.read_text())
    original_layers = geometry.prepare(scene)
    ecal = [l for l in original_layers if l['subsystem']=='ecal']
    lyso = [l for l in original_layers if l['subsystem']=='target']
    ecal_center = (np.min([l['lo'] for l in ecal], axis=0) +
                   np.max([l['hi'] for l in ecal], axis=0))/2
    lyso_center = (np.min([l['lo'] for l in lyso], axis=0) +
                   np.max([l['hi'] for l in lyso], axis=0))/2
    ecal_delta = lyso_center-ecal_center if args.center_ecal else np.zeros(2)
    shifted = {'units': scene['units'], 'objects': []}
    moving_paths = []
    translations = []
    for original in scene['objects']:
        obj = {key: copy.deepcopy(original[key]) for key in
               ('path', 'subsystem', 'material', 'shape', 'vertices_mm')}
        delta = np.zeros(3)
        if obj['subsystem'] in ('trigger', 'tracker', 'target'):
            delta[2] = 200
        elif obj['subsystem']=='ecal':
            delta[:2] = ecal_delta
        for vertex in obj['vertices_mm']:
            for axis in range(3):
                vertex[axis] += float(delta[axis])
        if geometry.active(obj) and np.any(delta):
            moving_paths.append(obj['path'])
            translations.append(obj['path']+'\t'+'\t'.join(str(v) for v in delta))
        shifted['objects'].append(obj)
    shifted['variation'] = {
        'translation_mm': [0, 0, 200],
        'translated_subsystems': ['trigger', 'tracker', 'target (LYSO)'],
        'fixed_subsystems': ['hcal'] if args.center_ecal else ['ecal', 'hcal'],
        'ecal_translation_mm': [*ecal_delta.tolist(), 0.0],
        'ecal_centering': 'Align the midpoint of the combined active-silicon X/Y envelope with the LYSO envelope centre; preserve internal offsets and height.' if args.center_ecal else 'Original position',
        'source_scene_sha256': hashlib.sha256(source.read_bytes()).hexdigest(),
        'scope': 'rate-only active-volume comparison; no revised production GDML',
    }
    expected_translated = 113 if args.center_ecal else 109
    assert len(moving_paths) == expected_translated
    (HERE / 'shifted_scene.json').write_text(json.dumps(shifted) + '\n')
    (HERE/'active_translations.tsv').write_text('\n'.join(translations)+'\n')
    (HERE / 'shifted_active_paths.txt').write_text('\n'.join(moving_paths) + '\n')
    nominal_layers = geometry.prepare(scene)
    raised_layers = geometry.prepare(shifted)
    nominal_by_paths = {tuple(sorted(x['paths'])): x for x in nominal_layers}
    for layer in raised_layers:
        before = nominal_by_paths[tuple(sorted(layer['paths']))]
        expected = 200 if layer['subsystem'] in ('trigger', 'tracker', 'target') else 0
        assert abs(layer['z_mm'] - before['z_mm'] - expected) < 1e-8
        delta = ecal_delta if layer['subsystem']=='ecal' else np.zeros(2)
        assert np.allclose(layer['lo']-before['lo'], delta, rtol=0, atol=1e-8)
        assert np.allclose(layer['hi']-before['hi'], delta, rtol=0, atol=1e-8)
    geometry.HERE = HERE
    sys.argv = [str(__file__), '--scene', str(HERE / 'shifted_scene.json'),
                '--power', str(args.power), '--replicates', str(args.replicates)]
    if args.reuse_integration:
        saved = json.loads((HERE/'results.json').read_text())
        assert saved['source_scene_sha256'] == hashlib.sha256((HERE/'shifted_scene.json').read_bytes()).hexdigest()
        assert saved['N_per_scramble'] == 2**args.power and saved['scrambles'] == args.replicates
    else:
        geometry.main()
    geometrical = json.loads((HERE/'results.json').read_text())
    geometrical['shielding_status'] = 'See comparison.json for the two roof cases with the fixed nominal stopping proxy.'
    (HERE/'results.json').write_text(json.dumps(geometrical, indent=2)+'\n')
    # ROOT reads the original GDML solids; only their world translations change.
    with (HERE / 'root_check.log').open('w') as log:
        subprocess.run(['root', '-l', '-b', '-q',
            f'../shift_up_200mm/check_shifted_rays.C("detector.gdml","../{HERE.relative_to(BASE)}")'],
            cwd=BASE / 'geometry', stdout=log, stderr=subprocess.STDOUT, check=True)
    check = json.loads((HERE / 'root_ray_check.json').read_text())
    assert check['mismatches'] == 0 and check['active_volumes'] == 217
    assert check['translated_active_volumes'] == expected_translated and check['rays'] == 192

    nominal = json.loads((BASE / 'shielding_results.json').read_text())
    rc, ic = shielding.range_table('concrete')
    rp, ip = shielding.range_table('polystyrene')
    data = np.loadtxt(HERE / 'angular_rate.csv', delimiter=',', skiprows=1)
    theta, rate_geometry = data[data[:, 1] > 0].T
    cosine = np.cos(np.deg2rad(theta))
    mass = nominal['detector_vertical_mass_column_g_cm2']
    norm = nominal['spectrum_normalization_factor']
    cases = []
    for reference in nominal['cases']:
        kdet = np.exp(ip(np.log(np.exp(rp(np.log(.010))) + mass / cosine)))
        cm = reference['concrete_cm']
        surface = (np.exp(ic(np.log(np.exp(rc(np.log(kdet))) + 2.3*cm/cosine)))
                   if cm else kdet)
        flux = np.array([norm*shielding.integral(k+shielding.MUON_MASS_GEV, c)
                         for k, c in zip(surface, cosine)])
        bins = rate_geometry * flux / (70*cosine**2)
        hz = float(bins.sum())
        cases.append({
            'label': reference['label'], 'concrete_cm': cm,
            'nominal_rate_per_hour': reference['rate_per_hour'],
            'shifted_rate_per_hour': hz*3600, 'shifted_rate_per_day': hz*86400,
            'shifted_mean_wait_min': 1/hz/60,
            'shifted_to_nominal_ratio': hz/reference['rate_Hz'],
            'fractional_change': hz/reference['rate_Hz']-1,
            'mean_required_surface_kinetic_GeV': float(np.average(surface, weights=bins)),
        })
        name = 'angular_concrete.csv' if cm else 'angular_no_roof.csv'
        np.savetxt(HERE/name, np.column_stack([theta, bins]), delimiter=',',
                   header='theta_deg,rate_Hz_per_0.02deg_bin', comments='')
    result = {
        **shifted['variation'], 'cases': cases,
        'required_layers': 26, 'translated_trigger_layers': 6, 'translated_tracker_layers': 4,
        'translated_lyso_layers': 2,
        'detector_vertical_mass_column_g_cm2': mass,
        'detector_mass_proxy': 'Held at the nominal ROOT-derived mean; not remeasured after translation.',
        'stopping_model': 'Same polystyrene CSDA proxy, concrete CSDA and 10 MeV exit floor as baseline.',
        'omissions': ['scattering', 'straggling', 'decay', 'detector inefficiency',
                      'mechanical clearance and revised-material transport validation'],
        'root_cross_check': check,
    }
    (HERE/'comparison.json').write_text(json.dumps(result, indent=2)+'\n')
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
