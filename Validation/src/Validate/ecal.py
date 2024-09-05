"""Plotting of ECal-related validation plots"""

from ._differ import Differ
from ._plotter import plotter
import logging

log = logging.getLogger('ecal')

@plotter(hist=True,event=False)
def digi_verify(d : Differ, out_dir = None) :
    """Plot ECal digi verify variables from the already created DQM histograms

    Parameters
    ----------
    d : Differ
        Differ containing files that are not event files (presumably histogram files)
    """

    col, name = 'EcalDigiVerify/EcalDigiVerify_num_sim_hits_per_cell', 'Number of SimHits per ECal Cell (excluding empty rec cells)'
    log.info(f'plotting {col}')
    d.plot1d(col, name, out_dir = out_dir, legend_kw = dict(loc='upper left'))

    features = [
        ('EcalDigiVerify/EcalDigiVerify_num_rec_hits', 'Number of RecHits'),
        ('EcalDigiVerify/EcalDigiVerify_num_noise_hits', 'Number of noisy RecHits'),
        ('EcalDigiVerify/EcalDigiVerify_total_rec_energy', 'Total Reconstructed Energy in ECal [MeV]'),
    ]
    for col, name in features :
        log.info(f'plotting {col}')
        d.plot1d(col, name, out_dir = out_dir)

@plotter(hist=True,event=False)
def shower_feats(d : Differ, out_dir = None) :
    """Plot ECal shower features from the already created DQM histograms

    Parameters
    ----------
    d : Differ
        Differ containing files that are not event files (presumably histogram files)
    """

    col, name = 'EcalShowerFeatures/EcalShowerFeatures_deepest_layer_hit', 'Deepest Layer Hit'
    log.info(f'plotting {col}')
    d.plot1d(col, name, out_dir = out_dir, legend_kw = dict(loc='upper left'))

    features = [
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
        log.info(f'plotting {col}')
        d.plot1d(col, name, out_dir = out_dir)

@plotter(hist=False,event=True)
def sim_hits(d : Differ, out_dir = None) :
    """Plot ECal-related validation plots

    Parameters
    ----------
    d : Differ
        Differ containing files that are event files
    """

    # loading just the IDs into memory so that we can calculate the layer
    #   the hits are in and count how many sim hits there were per event
    def rename_columns_and_calc_layer(data) :
        data.rename(inplace=True,columns=lambda cn : cn.replace('EcalSimHits_valid.','').replace('_',''))
        data['layer'] = (data['id'].values >> 17) & 0x3f
    d.load(manipulation = rename_columns_and_calc_layer, filter_name = 'EcalSimHits_valid/EcalSimHits_valid.id_')

    # plot number of sim hits
    log.info('plotting num sim hits')
    d.plot1d(lambda data : data.reset_index(level=1).index.value_counts(), 'Num Sim Hits', 
            ylabel='Events', range=(0,200), file_name = 'ecal_num_sim_hits', out_dir = out_dir)
    branches = [
        ('layer', 'Sim Layer Hit', 
            dict(bins=34, density=True)),
        ('LDMX_Events/EcalSimHits_valid/EcalSimHits_valid.edep_', 'Sim Energy Dep [MeV]',
            dict(range=(0,50),bins=50, density=True)),
        ('LDMX_Events/EcalSimHits_valid/EcalSimHits_valid.time_', 'Sim Hit Time [ns]',
            dict(range=(0,100000),bins=100,density=True)),
        ('LDMX_Events/EcalSimHits_valid/EcalSimHits_valid.x_', 'Sim Hit x [mm]',
            dict(range=(-300,300),bins=60, density=True)),
        ('LDMX_Events/EcalSimHits_valid/EcalSimHits_valid.y_', 'Sim Hit y [mm]',
            dict(range=(-300,300),bins=60, density=True)),
        ]
    for col, name, kw in branches :
        log.info(f'plotting {col}')
        d.plot1d(col, name, ylabel = 'Fraction Sim Hits', out_dir = out_dir, **kw)
