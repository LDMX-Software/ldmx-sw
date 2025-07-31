"""Hold the two or more files we are comparing together in one class"""

# standard modules
import os
import re

# external dependencies
import matplotlib
import uproot
import numpy as np
import hist.intervals

# us
from ._file import File

class Differ :
    """Differ allowing easy comparison of "similar" files

    The basic requirement of all files passed is that the columns
    of data in the 'LDMX_Events' TTree are named exactly the same.
    This is an easy requirement if the files are generated using
    the same configuration script and/or the same installation of
    ldmx-sw.

    Parameters
    ----------
    grp_name : str
        Name to include in legend title to differentiate
        this group of plots from another
    output_type : str
        The extension for the filetype that figures should be produced with in
        non-interactive mode
    args : list of tuples or Files
        Each entry is a tuple (file_path, name, *args) where file_path
        specifies the file to open and name is what should appear
        in plot legends.
        Alternatively, each entry can just be the already constructed File

    Example
    -------
    Opening a differ is pretty quick and lightweight.
    We do open the files with `uproot`.

        d = Differ('v3.2.0-alpha',('path/to/v12.root','v12'),('path/to/v14.root','v14'))

    Without any other options, the plotting with show the plot as if
    we are in an interactive notebook.

        d.plot1d('EcalSimHits_valid/EcalSimHits_valid.edep_', 'Sim E Dep [MeV]')

    """

    def __init__(self, grp_name, output_type,  *args) :
        def open_file(arg) :
            if isinstance(arg, (list,tuple)) :
                return File(*arg)
            elif isinstance(arg, File) :
                return arg
            else :
                raise KeyError(f'Argument provided {arg} is not a Validate.File or a tuple of arguments for its constructor')
                
        self.grp_name = grp_name
        self.files = list(map(open_file, args))
        self.output_type = output_type

    def __repr__(self) :
        """Short form representation of a Differ"""
        return f'Differ ({self.grp_name}) {self.files}'

    def plot1d(
        self,
        histname,
        xlabel,
        ylabel = 'Count',
        yscale = 'log',
        ylim = (None,None),
        out_dir = None,
        file_name = None,
        tick_labels = None,
        legend_kw = dict(),
        rebin = 1,
        **hist_kwargs
    ):
        """Plot a 1D histogram, overlaying the File entries

        We overlay the same 'column' of data of each File onto
        the same figure, generating a legend with the title defined by
        grp_name from the constructor.

        If out_dir is not provided, we assume we are in a notebook and
        simply `matplotlib.pyplot.show()` the figure. If out_dir is not None (i.e. it
        was defined), we assume we are in a non-interactive script and
        write the figure to a PDF in the output file and then clear
        the figure.

        Parameters
        ----------
        histname : str or Callable
            Determines the array of data from each File to histogram and plot
        xlabel : str
            Label of X axis
        ylabel : str, optional
            Label for Y axis (default: Count)
        yscale : {'linear', 'log', 'symlog', 'logit', ...}, optional
            Scale to use for y-axis (default: 'log')
        ylim : 2-tuple
            Limits to set for the y-axis (default: deduced by matplotlib)
        out_dir : str
            Directory in which to write the plotting file
        tick_labels: list, optional
            Tick labels for the x-axis
        file_name : str
            Name of file, no extension (default: histname name with directory separators removed)
        hist_kwargs : dict
            All other key-word arguments are passed into each File.plot1d
        """
        fig = matplotlib.pyplot.figure('differ',figsize=(11,8))
        raw_ax, ratio_ax = fig.subplots(
            nrows = 2,
            sharex = 'col',
            height_ratios = [2, 1],
            gridspec_kw = dict(
                hspace = 0.05
            )
        )

        raw_histograms = []
        for f in self.files :
            try:
                h = f.get(histname)
                art = h[hist.rebin(rebin)].plot1d(ax=raw_ax, **f.hist_kwargs, **hist_kwargs)
                raw_histograms.append((h, art))
            except uproot.KeyInFileError:
                f.log.warn(f"Key {histname} doesn't exist in {self}, skipping")
                continue

        raw_ax.set_xlabel(None)
        raw_ax.set_ylabel(ylabel)
        raw_ax.set_yscale(yscale)
        raw_ax.set_ylim(*ylim)

        if 'title' not in legend_kw :
            legend_kw['title'] = self.grp_name

        raw_ax.legend(**legend_kw)

        den_h, _den_art = raw_histograms[0]
        for num_h, num_art in raw_histograms[1:]:
            (den_h/num_h).plot1d(
                ax = ratio_ax,
                yerr = hist.intervals.ratio_uncertainty(
                    num = num_h.values(),
                    denom = den_h.values()
                ),
                histtype='errorbar',
                color = num_art[0].stairs.get_edgecolor()
            )

        ratio_ax.set_ylabel('Ratio')
        ratio_ax.set_xlabel(xlabel)
        if tick_labels is not None:
            ratio_ax.set_xticks((bins[1:]+bins[:-1])/2)
            ratio_ax.set_xticklabels(tick_labels)
            ratio_ax.tick_params(axis='x', rotation=90)

        if out_dir is None :
            matplotlib.pyplot.show()
        else :
            if file_name is None :
                file_name = re.sub(r'^.*/','',histname)
            fig.savefig(os.path.join(out_dir,file_name)+ self.output_type, bbox_inches='tight')
            fig.clf()
