from pathlib import Path
import hashlib,json,platform,importlib.metadata
p=Path(__file__).resolve().parent
(p/'environment.json').write_text(json.dumps({'python':platform.python_version(),'packages':{x:importlib.metadata.version(x) for x in ['numpy','scipy','matplotlib']},'geometry_git_commit':'eae30ac8f64f694fa99bbc8ed30c1a77fef05135'},indent=2)+'\n')
files=[f for f in p.rglob('*') if f.is_file() and not any(k in f.parts for k in ('.venv','__pycache__','qa')) and f.name!='SHA256SUMS' and f.suffix not in ('.log','.aux','.out')]
(p/'SHA256SUMS').write_text(''.join(hashlib.sha256(f.read_bytes()).hexdigest()+'  '+str(f.relative_to(p))+'\n' for f in sorted(files)))
