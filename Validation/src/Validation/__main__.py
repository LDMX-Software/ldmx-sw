"""CLI for comparison plots within Ecal Validation"""

# standard
import argparse
import os
import re

# external
import matplotlib.pyplot as plt
#import mplhep
#plt.style.use(mplhep.style.ROOT)

# us
from ._differ import Differ

def extract_parameters(fn) :
    l = fn.replace('.root','').split('_')
    return l[0], { l[i] : l[i+1] for i in range((len(l)-1)) if i%2 == 1 }

def make_system_dqm_plots(plotter) :
    dqmPlotList=plotter.dqm()

    for path, name in dqmPlotList :
        print(path+", "+name)
        hd.plot1d(path, name, 
                    file_name = re.sub(r'^.*/','',path),
                    out_dir = out_dir)

    return 

def deduce_modules() :
    import inspect
    import sys

    def filter_members(obj) :
        if not inspect.ismodule(obj) :
            return False
        return not obj.__name__.strip('Validation.').startswith('_')

    return dict(inspect.getmembers(sys.modules['Validation'], filter_members))

# guard incase someone imports this somehow
if __name__ == '__main__' :
    runnable_modules = deduce_modules()

    parser = argparse.ArgumentParser("python3 -m Validation",
        description="""
        Make comparison plots between different geometries.

        We assume that the input files are written out in the
        format of the configs in this repository so we can
        deduce the parameters from their names.
        """
        )

    parser.add_argument('dev',help='directory of event and histogram files from new developments')
    parser.add_argument('--label',help='label for developments, defaults to dev directory name')
    parser.add_argument('--out-dir',help='directory to which to print plots. defaults to input data dev dir')
    parser.add_argument('--systems',required=True, choices=runnable_modules.keys(), nargs='+',
        help='list of systems for which to make plots')

    arg = parser.parse_args()

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
        File(os.path.join(dev,f), extract_parameters(f)) 
        for f in os.listdir(dev) if f.endswith('.root') 
        ]

    hd = Differ(label, *[f for f in root_files if not f.is_events()])
    ed = Differ(label, *[f for f in root_files if f.is_events()])

    for syst in arg.systems :
        runnable_modules[syst].plot(hd,ed, out_dir = out_dir)

