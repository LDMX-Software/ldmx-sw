"""CLI for comparison plots within Ecal Validation"""

import argparse
import os
import re
from _differ import Differ

import matplotlib.pyplot as plt
#import mplhep
#plt.style.use(mplhep.style.ROOT)

from systems.ecal import Ecal_plots
from systems.trigscint import TrigScint_plots

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
    parser.add_argument('--systems',help='csv list of systems for which to make plots [options are ecal, trigscint]')

    arg = parser.parse_args()

    if arg.systems is None :
        print("Must specify which system's plots to make (use --systems 'ecal, ...') \nExiting")
        exit()

    print("Making plots for "+arg.systems)
    dev = arg.dev
    if dev.endswith('/') :
        dev = dev[:-1]
    print("Using data in "+dev)

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

    shower_feats = TrigScint_plots.feats()
    if "trigscint" in (syst.lower() for syst in arg.systems) :
        shower_feats += TrigScint_plots.feats()
    if "ecal" in (syst.lower() for syst in arg.systems) :
        shower_feats += Ecal_plots.feats()

    for path, name in shower_feats :
        print(path)
        print(name)
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

