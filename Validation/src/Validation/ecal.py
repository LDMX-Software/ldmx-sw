"""Plotting of ECal-related validation plots"""

from ._differ import Differ

def plot_hists(d : Differ, out_dir = None) :
    """Plot ECal-related validation plots

    Parameters
    ----------
    d : Differ
        Differ containing files that are not event files (presumably histogram files)
    """

    features = [
        ('EcalShowerFeatures/EcalShowerFeatures_deepest_layer_hit', 'Deepest Layer Hit'),
        ('EcalShowerFeatures/EcalShowerFeatures_num_readout_hits', 'N Readout Hits'),
        ('EcalShowerFeatures/EcalShowerFeatures_summed_det', 'Total Rec Energy [MeV]'),
        ('EcalShowerFeatures/EcalShowerFeatures_summed_iso', 'Total Isolated Energy [MeV]'),
        ('EcalShowerFeatures/EcalShowerFeatures_summed_back', 'Total Back Energy [MeV]'),
        ('EcalShowerFeatures/EcalShowerFeatures_max_cell_dep', 'Max Cell Dep [MeV]'),
        ('EcalShowerFeatures/EcalShowerFeatures_shower_rms', 'Shower RMS [mm]'),
        ('EcalShowerFeatures/EcalShowerFeatures_x_std', 'X Standard Deviation [mm]'),
        ('EcalShowerFeatures/EcalShowerFeatures_y_std', 'Y Standard Deviation [mm]'),
        ('EcalShowerFeatures/EcalShowerFeatures_avg_layer_hit', 'Avg Layer Hit'),
        ('EcalShowerFeatures/EcalShowerFeatures_std_layer_hit', 'Std Dev Layer Hit')
    ]
    for col, name in features :
        d.plot1d(col, name, out_dir = out_dir)

def plot_events(d : Differ, out_dir = None) :
    """Plot ECal-related validation plots

    Parameters
    ----------
    d : Differ
        Differ containing files that are event files
    """

    branches = [
        ('EcalSimHits_valid/EcalSimHits_valid.edep_', 'Sim Energy Dep [MeV]')
        ]
    for col, name in branches :
        d.plot1d(f'LDMX_Events/{col}', name, out_dir = out_dir)
