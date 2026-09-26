#!/usr/bin/env python3
"""Draw the complete installed rate geometry from the frozen aligned scene.

Display only: no rays are regenerated and no rate inputs are changed. The
separate historical ray panels are clipped directly from geometry_acceptance.pdf.
"""
from pathlib import Path
import hashlib,json
import numpy as np
from scipy.spatial import ConvexHull
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.collections import PolyCollection
from matplotlib.patches import Patch
from matplotlib.colors import to_rgb
import legacy_geometry as geometry

HERE=Path(__file__).resolve().parent
COLORS={'hcal':'#257f78','trigger':'#b78011','tracker':'#286cb0','target':'#7951a4','ecal':'#b3456d'}

def make_overview():
    source=HERE/'inputs/aligned_scene.json';scene=json.loads(source.read_text())
    origin=np.array([5027.519993117254,-155.750230102,0.])
    eye=np.array([1.7,-2.3,1.45]);eye/=np.linalg.norm(eye)
    right=np.cross([0,0,1],eye);right/=np.linalg.norm(right);up=np.cross(eye,right)
    def draw(ax,objects):
        faces=[];colors=[];depth=[]
        for o in objects:
            v=(np.array(o['vertices_mm'])-origin)/1000
            active=geometry.active(o);color=np.array(to_rgb(COLORS[o['subsystem']] if active else '#b9bfc3'))
            xy=np.unique(v[:,:2],axis=0);poly=xy[ConvexHull(xy).vertices]
            lo=np.column_stack([poly,np.full(len(poly),v[:,2].min())]);hi=lo.copy();hi[:,2]=v[:,2].max()
            surfaces=[lo,hi]+[np.array([lo[i],lo[(i+1)%len(lo)],hi[(i+1)%len(lo)],hi[i]]) for i in range(len(lo))]
            for t in surfaces:
                faces.append(np.column_stack([t@right,t@up]));depth.append(t.mean(0)@eye+(1e-7 if active else 0))
                normal=np.cross(t[1]-t[0],t[2]-t[0]);normal/=np.linalg.norm(normal)
                shade=.78+.22*abs(normal@np.array([.3,-.4,.866]));colors.append(np.r_[color*shade,1. if active else .16])
        order=np.argsort(depth)
        ax.add_collection(PolyCollection([faces[i] for i in order],facecolors=np.array(colors)[order],edgecolors='#ffffff44',linewidths=.14))
        ax.update_datalim(np.concatenate(faces));ax.autoscale_view();ax.set_aspect('equal');ax.axis('off')
    with plt.rc_context({'font.family':'serif','font.size':12,'pdf.fonttype':42}):
        fig=plt.figure(figsize=(5.0,8.2))
        ax=fig.add_axes([.01,.42,.98,.57]);draw(ax,scene['objects'])
        # The full view retains physical dimensions; a separate inset makes
        # the narrow central components visible without enlarging their solids.
        central=[o for o in scene['objects'] if o['subsystem'] in ('trigger','tracker','target')]
        inset=fig.add_axes([.02,.03,.43,.36]);draw(inset,central)
        inset.set_title('Central assembly\n(enlarged)',fontsize=11,pad=3)
        labels={'hcal':'HCAL','trigger':'TS counters','tracker':'Tracker silicon','target':'LYSO','ecal':'ECAL silicon'}
        handles=[Patch(facecolor=COLORS[k],label=labels[k]) for k in COLORS]
        handles.append(Patch(facecolor='#b9bfc3',label='Passive material'))
        fig.legend(handles=handles,loc='lower left',bbox_to_anchor=(.45,.15),frameon=False,fontsize=11,labelspacing=.8)
        fig.text(.49,.08,'356 placed objects\n217 active volumes\n26 active layers',fontsize=10,linespacing=1.5)
        fig.savefig(HERE/'figures/full_installed_geometry.pdf');fig.savefig(HERE/'figures/full_installed_geometry.png',dpi=180);plt.close(fig)
    provenance={'scene_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'placed_objects':len(scene['objects']),
      'active_volumes':sum(geometry.active(o) for o in scene['objects']),'left':'All installed objects from the aligned rate-model snapshot; central inset is a separate enlarged view.',
      'right_source':'../geometry_acceptance.pdf','right_sha256':hashlib.sha256((HERE.parent/'geometry_acceptance.pdf').read_bytes()).hexdigest(),
      'right_layout':'Original offset ECAL; original32 accepted straight rays. Original vector projections clipped and stacked by LaTeX.',
      'display_note':'Passive material is translucent; coincident active faces drawn after their same-material silicon parents.',
      'CAD_support_material_included':False,'new_transport_or_rate_calculation':False}
    (HERE/'figures/overview_provenance.json').write_text(json.dumps(provenance,indent=2)+'\n')

if __name__=='__main__':make_overview()
