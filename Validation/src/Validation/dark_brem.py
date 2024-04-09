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
    ]
    for h, name in features :
        log.info(f'plotting {h}')
        d.plot1d(f'db_kinematics/db_kinematics_{h}', name, out_dir = out_dir, density=True, ylabel='Weighted Fraction')

    log.info('plotting dark_brem_element')
    d.plot1d(
        'db_kinematics/db_kinematics_dark_brem_element', 
        'Element in which Dark Brem Occurred',
        out_dir = out_dir,
        tick_labels = [
            "did not happen",
            "H 1",
            "C 6",
            "O 8",
            "Na 11",
            "Si 14",
            "Ca 20",
            "Cu 29",
            "W 74",
            "unlisted",
        ],
        density=True,
        ylabel='Weighted Fraction'
    )

    log.info('plotting dark_brem_material')
    d.plot1d(
        'db_kinematics/db_kinematics_dark_brem_material',
        'Material in which Dark Brem Occurred',
        out_dir = out_dir,
        tick_labels = [
            "Unknown",
            "C",
            "PCB",
            "Glue",
            "Si",
            "Al",
            "W",
            "PVT"
        ],
        density=True,
        ylabel='Weighted Fraction'
    )
