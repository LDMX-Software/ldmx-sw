#!/usr/bin/env python3
"""Record portable study inputs and deliverables, excluding transient build files."""
from pathlib import Path
import hashlib,json,platform,sys
import numpy,scipy,matplotlib
HERE=Path(__file__).resolve().parent;BASE=HERE.parent
def include(p):
    r=p.relative_to(BASE)
    return (p.is_file() and not p.is_symlink() and
            not any(v in {'__pycache__','checkpoints','pilot','pdf_pages'} for v in r.parts) and
            p.suffix not in {'.pyc','.dylib','.so','.aux','.out','.zip','.log'} and
            p.name not in {'SHA256SUMS','delivery_manifest.json','.DS_Store'} and
            not p.name.startswith('contact_'))
def main():
    results=json.loads((HERE/'results/results.json').read_text())
    files={str(p.relative_to(BASE)):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(BASE.rglob('*')) if include(p)}
    (BASE/'SHA256SUMS').write_text(''.join(f'{digest}  {name}\n' for name,digest in files.items()))
    manifest={'title':'Cosmic-muon trigger and coincidence rates in the aligned ESA test stand',
      'date':'2026-09-20','rate_calculation_date':'2026-09-19',
      'revision':'Geometry overview, combined figures and reader-facing context; rates unchanged',
      'authors':['Emrys Peets','Matthew Gignac','Takumi Britt'],
      'report':'cosmic_rate_report.pdf','sha256':files['cosmic_rate_report.pdf'],
      'definition':'Inclusive omitted-hit requirements; all devices remain installed; ECAL aligned at original heights',
      'scientific_status':'Conditional straight-ray, sea-level spectrum and mixed-material CSDA proxy; no detector efficiency or scattering',
      'counts':{'specified_equipment_subsets':896,'inclusive_multiplicity_combinations':280,'exclusive_tracker_count_combinations':280,'HCAL_trigger_and_single_layer_rows':21},
      'runtime':{'python':platform.python_version(),'executable':sys.executable,'numpy':numpy.__version__,'scipy':scipy.__version__,'matplotlib':matplotlib.__version__},
      'calculation_fingerprint':results['calculation_fingerprint'],'files':files}
    (BASE/'delivery_manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    print(f'Manifest: {len(files)} files; PDF SHA256 {manifest["sha256"]}')
if __name__=='__main__':main()
