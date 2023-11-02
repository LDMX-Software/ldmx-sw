"""CLI for comparison plots within LDMX Validation"""

# standard
import argparse
import os
import re
import logging

# external
import matplotlib
# this line allows us to run without an X server connected
#    basically telling MPL that it will not open a window
matplotlib.use('Agg')

import matplotlib.pyplot as plt
import mplhep
plt.style.use(mplhep.style.ROOT)

# us
from ._differ import Differ
from ._file import File
from ._plotter import plotter

# guard incase someone imports this somehow
if __name__ == '__main__' :
    parser = argparse.ArgumentParser("python3 -m Validation",
        description="""
        Make comparison plots between different files within a directory.

        The labels of different plots within the directory is controlled by
        the parameter you choose. The parameters of a file are extracted from
        the file name by splitting the filename into key-val pairs separated 
        by underscores (i.e. key1_val1_key2_val2_..._keyN_valN.root). If no
        parameter is provided, then the first key/val is used.
        """
        )

    parser.add_argument('data',help='directory of event and histogram files')
    parser.add_argument('--log',help='logging level',choices=['info','debug','warn','error'], default='warn')
    parser.add_argument('--label',help='label for grouping of data, defaults to data directory name')
    parser.add_argument('--out-dir',help='directory to which to print plots. defaults to input data directory')
    parser.add_argument('--systems',required=True, choices=plotter.__registry__.keys(), nargs='+',
        help='list of plotters to run')
    parser.add_argument('--output-type', help='File format to use to produce figures', default='pdf')
    parser.add_argument('--param',
                        nargs='+',
                        help='parameter(s) in filename to use as file labels')
    parser.add_argument('--input-files', nargs='+', type=str,
                        help="""Specify the name of the root files directly (either
                        full/relative path or name of files in the input data directory)""")

    arg = parser.parse_args()

    numeric_level = getattr(logging, arg.log.upper(), None)
    if not isinstance(numeric_level, int) :
        raise ValueError(f'Invalid log level: {arg.log}')
    logging.basicConfig(level=numeric_level)

    logging.getLogger('matplotlib').setLevel(logging.ERROR)

    logging.debug(f'Parsed Arguments: {arg}')

    data = arg.data
    if data.endswith('/') :
        data = data[:-1]

    label = os.path.basename(data)
    if arg.label is not None :
        # update the raw string literal to intrepret new lines
        label = arg.label.replace(r'\n','\n')

    out_dir = data
    if arg.out_dir is not None :
        out_dir = arg.out_dir

    if arg.output_type is not None:
        output_type = arg.output_type
        if output_type[0] != '.':
            output_type = '.' + output_type
    else:
        output_type = '.pdf'

    logging.debug(f'Deduced Args: label = {label} out_dir = {out_dir}')

    if arg.input_files:
        input_files = [os.path.join(data, f)
                       if not f.startswith(data)
                       else f
                       for f in arg.input_files
                       ]
        root_files = [File.from_path(f, legendlabel_parameter=arg.param)
                      for f in input_files]
    else:
        root_files = [ File.from_path(os.path.join(data,f), legendlabel_parameter = arg.param)
                       for f in os.listdir(data) if f.endswith('.root') ]

    logging.debug(f'ROOT Files: {root_files}')

    hd = Differ(label, output_type, *[f for f in root_files if not f.is_events()])
    ed = Differ(label, output_type, *[f for f in root_files if f.is_events()])

    logging.debug(f'histogram differ = {hd}')
    logging.debug(f'event differ = {ed}')

    for syst in arg.systems :
        logging.info(f'running {syst}')
        h, e, plot = plotter.__registry__[syst]
        if h and e :
            logging.debug('both hist and event plotter')
            plot(hd, ed, out_dir = out_dir)
        elif h :
            logging.debug('both hist-only plotter')
            plot(hd, out_dir = out_dir)
        elif e :
            logging.debug('both event-only plotter')
            plot(ed, out_dir = out_dir)
        else :
            logging.warn(f'Not running {syst} since it was not registered properly.')

