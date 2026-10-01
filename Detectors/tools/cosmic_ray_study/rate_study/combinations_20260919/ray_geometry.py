#!/usr/bin/env python3
"""Finite-volume hits and exact installed-material chords for straight rays.

API: model=load(scene_path,gdml_path); model.trace(xy,slope,zref) returns
uint32 ``masks``, full ``material_columns`` and ``cumulative_columns[:,layer]``
in g/cm^2 through each layer's lower surface. Layers use calculate_rate.prepare
ordering. xy is the ray point at zref; xy(z)=xy+slope*(zref-z). Rays start above
the entire geometry, even when zref is an interior reference plane.

Four tracker active daughters have zero stopping density because their same-
material parents already contain them. All installed ECal objects remain.
HCal bars are merged only after calculate_rate.prepare verifies area tiling.
The exported scene's convex prisms are exact for this frozen geometry; this is
neither a general GDML navigator nor a scattering/energy-loss transport model.
"""
from pathlib import Path
import ctypes
import hashlib
import importlib.util
import json
import os
import platform
import subprocess
import xml.etree.ElementTree as ET
import numpy as np
from scipy.spatial import ConvexHull

HERE=Path(__file__).resolve().parent


def _library():
    source=HERE/'ray_geometry.cpp'
    suffix='.dylib' if platform.system()=='Darwin' else '.so'
    digest=hashlib.sha256(source.read_bytes()).hexdigest()[:12]
    output=HERE/f'_ray_geometry_{digest}{suffix}'
    if not output.exists():
        command=[os.environ.get('CXX','c++'),'-O3','-std=c++17','-fPIC']
        command+=['-dynamiclib' if platform.system()=='Darwin' else '-shared',str(source),'-o',str(output)]
        subprocess.run(command,check=True,capture_output=True,text=True)
    lib=ctypes.CDLL(str(output))
    dptr=np.ctypeslib.ndpointer(dtype=np.float64,flags='C_CONTIGUOUS')
    uptr=np.ctypeslib.ndpointer(dtype=np.uint32,flags='C_CONTIGUOUS')
    lib.trace_prisms.argtypes=[ctypes.c_int64,dptr,dptr,ctypes.c_double,ctypes.c_int,dptr,dptr,
                              ctypes.c_int,dptr,uptr,dptr,dptr]
    lib.trace_prisms.restype=None
    return lib


class Model:
    def __init__(self,scene_path,gdml_path):
        self.scene_path=Path(scene_path).resolve()
        self.gdml_path=Path(gdml_path).resolve()
        scene=json.loads(self.scene_path.read_text())
        module_path=HERE/'legacy_geometry.py'
        spec=importlib.util.spec_from_file_location('legacy_cosmic_geometry',module_path)
        geometry=importlib.util.module_from_spec(spec);spec.loader.exec_module(geometry)
        self.layers=geometry.prepare(scene)
        lookup={(l['subsystem'],round(l['z_mm'],4)):i for i,l in enumerate(self.layers)}
        self.layer_zlow=np.ascontiguousarray([l['z_mm']-max(p['half_z'] for p in l['prisms']) for l in self.layers],dtype=np.float64)
        assert len(self.layers)<=32 and np.all(np.diff(self.layer_zlow)<=0)
        doc=ET.parse(self.gdml_path)
        rho={m.get('name'):float(m.find('D').get('value')) for m in doc.findall('materials/material')}
        self.densities=rho
        prisms=[];planes=[]

        def add(v,density,layer):
            v=np.asarray(v,dtype=float)
            zlo,zhi=float(v[:,2].min()),float(v[:,2].max())
            assert zhi>zlo
            assert np.all(np.minimum(abs(v[:,2]-zlo),abs(v[:,2]-zhi))<1e-6), 'Nonvertical prism is unsupported'
            xy=np.unique(v[:,:2],axis=0);hull=ConvexHull(xy)
            # Top and bottom must have the same transverse convex hull.
            top=v[np.abs(v[:,2]-zhi)<1e-6,:2];bottom=v[np.abs(v[:,2]-zlo)<1e-6,:2]
            assert np.isclose(ConvexHull(top).volume,hull.volume,rtol=1e-9)
            assert np.isclose(ConvexHull(bottom).volume,hull.volume,rtol=1e-9)
            lo=xy.min(0);hi=xy.max(0)
            eq=hull.equations
            # Rectangle clipping already supplies all four planes for axis boxes.
            if np.all((abs(eq[:,0])<1e-12)|(abs(eq[:,1])<1e-12)):eq=np.empty((0,3))
            first=len(planes);planes.extend(eq.tolist())
            prisms.append([lo[0],hi[0],lo[1],hi[1],zlo,zhi,density,layer,first,len(eq)])

        for o in scene['objects']:
            if o['subsystem']=='hcal' and o['material']=='Scintillator':continue
            v=np.asarray(o['vertices_mm']);z=float((v[:,2].min()+v[:,2].max())/2)
            layer=lookup[(o['subsystem'],round(z,4))] if geometry.active(o) else -1
            density=rho[o['material']]
            if o['subsystem']=='tracker' and 'active_sensor' in o['path']:density=0
            add(v,density,layer)
        for i,l in enumerate(self.layers):
            if l['subsystem']!='hcal':continue
            assert len(l['prisms'])==1
            p=l['prisms'][0];v=np.array([[x,y,l['z_mm']+sign*p['half_z']] for sign in [-1,1] for x,y in p['poly']])
            add(v,rho['Scintillator'],i)
        self.prisms=np.ascontiguousarray(prisms,dtype=np.float64)
        self.planes=np.ascontiguousarray(planes,dtype=np.float64).reshape(-1,3)
        self._lib=_library()
        self.metadata={'geometry':'exact convex vertical-prism chords in exported frozen scene',
            'material_objects_original':len(scene['objects']),'calculation_prisms':len(prisms),
            'tracker_material_policy':'Full silicon parents retained; four same-material active daughters assigned zero additional density.',
            'hcal_material_policy':'Adjacent same-material bars merged after the original area-tiling assertion.',
            'units':'Positions mm; density g/cm3; columns g/cm2.',
            'stop_planes':'Lower surface of each active layer, in descending z order.',
            'scene_sha256':hashlib.sha256(self.scene_path.read_bytes()).hexdigest(),
            'gdml_sha256':hashlib.sha256(self.gdml_path.read_bytes()).hexdigest()}

    def trace(self,xy,slope,zref):
        xy=np.ascontiguousarray(xy,dtype=np.float64)
        slope=np.ascontiguousarray(slope,dtype=np.float64)
        if xy.ndim!=2 or xy.shape[1]!=2 or slope.shape!=xy.shape:raise ValueError('xy and slope must both have shape (N,2)')
        if not np.all(np.isfinite(xy)) or not np.all(np.isfinite(slope)) or not np.isfinite(zref):raise ValueError('finite inputs required')
        n=len(xy);masks=np.empty(n,dtype=np.uint32);columns=np.empty(n,dtype=np.float64)
        cumulative=np.empty((n,len(self.layers)),dtype=np.float64)
        self._lib.trace_prisms(n,xy,slope,float(zref),len(self.prisms),self.prisms,self.planes,
                              len(self.layers),self.layer_zlow,masks,columns,cumulative)
        return {'masks':masks,'material_columns':columns,'cumulative_columns':cumulative}


def load(scene_path,gdml_path):
    return Model(scene_path,gdml_path)
