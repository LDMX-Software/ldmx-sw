"""CLI for comparison plots within Ecal Validation"""

import argparse
import os
import re
from _differ import Differ

import matplotlib.pyplot as plt
import mplhep
plt.style.use(mplhep.style.ROOT)

def extract_parameters(fn) :
    l = fn.replace('.root','').split('_')
    return l[0], { l[i] : l[i+1] for i in range((len(l)-1)) if i%2 == 1 }

# guard incase someone imports this somehow
if __name__ == '__main__' :
    parser = argparse.ArgumentParser(
        """
        Make comparison plots between different geometries.

        We assume that the input files are written out in the
        format of the configs in this repository so we can
        deduce the parameters from their names.
        """
        )

    parser.add_argument('dev',help='directory of event and histogram files from new developments')
    parser.add_argument('--label',help='label for developments, defaults to dev directory name')
    parser.add_argument('--out-dir',help='directory to which to print plots. defaults to input data dev dir')

    arg = parser.parse_args()

    dev = arg.dev
    if dev.endswith('/') :
        dev = dev[:-1]
    print(dev)

    label = os.path.basename(dev)
    if arg.label is not None :
        label = arg.label

    out_dir = dev
    if arg.out_dir is not None :
        out_dir = arg.out_dir
    
    root_files = [ 
        (os.path.join(dev,f), extract_parameters(f)) 
        for f in os.listdir(dev) if f.endswith('.root') 
        ]

    histo_files = [ (fp, params['geometry']) for fp, (t, params) in root_files if t == 'histos' ]
    hd = Differ(label, *histo_files)

    shower_feats = [
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
    for path, name in shower_feats :
          hd.plot1d(path, name, 
                    file_name = re.sub(r'^.*/','',path),
                    out_dir = out_dir)

    event_files = [ (fp, params['geometry']) for fp, (t, params) in root_files if t == 'events' ]
    ed = Differ(arg.label, *event_files)
    ed.plot1d('LDMX_Events/EcalSimHits_valid/EcalSimHits_valid.edep_',
              'Sim Energy Dep [MeV]',
              bins=50, range=(0,30),
              file_name = 'edep',
              out_dir = out_dir)

