"""Plotting of ECal-related validation plots"""

from ._differ import Differ

def plot(hd : Differ, ed : Differ, out_dir = None) :
    """Plot ECal-related validation plots

    Parameters
    ----------
    hd : Differ
        Differ containing files that are not event files (presumably histogram files)
    ed : Differ
        Differ containing files that are event files
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
        hd.plot1d(col, name,
                  file_name = re.sub(r'^.*/','',path),
                  out_dir = out_dir)

    
