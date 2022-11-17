plots = []

class Ecal_plots :
    
    def dqm() :
        # these are just the veto feature plots. could have separate functions for these and
        # other Ecal DQM plots, and call them all from here
        plots = [
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
        return plots

    
