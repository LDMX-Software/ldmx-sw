"""Hold the two or more files we are comparing together in one class"""

# standard modules
import os
import re

# external dependencies
import matplotlib.pyplot as plt

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

    def __init__(self, grp_name, *args) :
        def open_file(arg) :
            if isinstance(arg, (list,tuple)) :
                return File(*arg)
            elif isinstance(arg, File) :
                return arg
            else :
                raise KeyError(f'Argument provided {arg} is not a Validation.File or a tuple of arguments for its constructor')
                
        self.grp_name = grp_name
        self.files = list(map(open_file, args))

    def __repr__(self) :
        """Short form representation of a Differ"""
        return f'Differ ({self.grp_name}) {self.files}'

    def plot1d(self, column, xlabel, 
              ylabel = 'Count',
              yscale = 'log',
              ylim = (None,None),
              out_dir = None, file_name = None,
              legend_kw = dict(),
              **hist_kwargs) :
        """Plot a 1D histogram, overlaying the File entries

        We overlay the same 'column' of data of each File onto
        the same figure, generating a legend with the title defined by
        grp_name from the constructor.

        If out_dir is not provided, we assume we are in a notebook and
        simply `plt.show()` the figure. If out_dir is not None (i.e. it
        was defined), we assume we are in a non-interactive script and
        write the figure to a PDF in the output file and then clear
        the figure.

        Parameters
        ----------
        column : str or Callable
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
        file_name : str
            Name of file, no extension (default: column name with directory separators removed)
        hist_kwargs : dict
            All other key-word arguments are passed into each File.plot1d
        """
        fig = plt.figure('differ',figsize=(11,8))
        ax = fig.subplots()

        for f in self.files :
            f.plot1d(ax, column, **hist_kwargs)

        ax.set_xlabel(xlabel)
        ax.set_ylabel(ylabel)
        ax.set_yscale(yscale)
        ax.set_ylim(*ylim)

        if 'title' not in legend_kw :
            legend_kw['title'] = self.grp_name

        ax.legend(**legend_kw)

        if out_dir is None :
            plt.show()
        else :
            if file_name is None :
                if isinstance(column, str) :
                    file_name = re.sub(r'^.*/','',column)
                else :
                    # assume column is a function meaning the '__name__'
                    #   parameter is defined by Python for us
                    file_name = column.__name__
            fig.savefig(os.path.join(out_dir,file_name)+'.pdf', bbox_inches='tight')
            fig.clf()

    def load(self, **kwargs) :
        """Load all of the event data frames into memory
        
        The key-word arguments are used in each File's events call
        to specify which branches (if not all of them) should be loaded
        into memory and what manipulation (if any) to do.
        """
        for f in self.files :
            f.load(**kwargs)
            
    def manipulate(self, manipulation) :
        """Manipulate all of the File data frames

        Parameters
        ----------
        manipulation : Callable (e.g. a function)
            Function operating on the data frame to manipuate it **in place**
        """
        for f in self.files :
            f.manipulate(manipulation)
