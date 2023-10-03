"""Plotting of ECal-related validation plots"""

from ._differ import Differ
from ._plotter import plotter
import logging

log = logging.getLogger('dark_brem')

@plotter(hist=True,event=False)
def kinematics(d : Differ, out_dir = None) :
    """Plot Dark Brem interaction histograms

    Parameters
    ----------
    d : Differ
        Differ containing files that are not event files (presumably histogram files)
    """

    features = [
        ('aprime_energy', 'Dark Photon Energy [MeV]'),
        ('aprime_pt', 'Dark Photon $p_T$ [MeV]'),
        ('recoil_energy', 'Recoil Energy [MeV]'),
        ('recoil_pt', 'Recoil $p_T$ [MeV]'),
        ('incident_energy', 'Incident Energy [MeV]'),
        ('incident_pt', 'Incident $p_T$ [MeV]'),
        ('dark_brem_z', 'Dark Brem Z Location [mm]'),
        ('dark_brem_element', 'Element in which Dark Brem Occurred'),
        ('dark_brem_material', 'Material in which Dark Brem Occurred')
    ]
    for h, name in features :
        log.info(f'plotting {h}')
        d.plot1d(f'db_kinematics/db_kinematics_{h}', name, out_dir = out_dir)
