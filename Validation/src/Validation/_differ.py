"""Hold the two or more files we are comparing together in one class"""

# standard modules
import os

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
    args : list of 2-tuples
        Each entry is a 2-tuple (file_path, name) where file_path
        specifies the file to open and name is what should appear
        in plot legends
    kwargs : dict
        These keyword arguments are passed to plt.figure for creationg
        of the figure we draw on.

    Example
    -------
    Opening a differ is pretty quick and lightweight.
    We do open the files with `uproot` and access the event tree.

        d = Differ('v3.2.0-alpha',('path/to/v12.root','v12'),('path/to/v14.root','v14'))

    Without any other options, the plotting with show the plot as if
    we are in an interactive notebook.

        d.plot1d('EcalSimHits_valid/EcalSimHits_valid.edep_')
    """

    def __init__(self, grp_name, *args) :
        def open_file(arg) :
            if isinstance(arg, (list,tuple)) :
                return File(*arg)
            elif isinstance(arg, File) :
                return arg
            else :
                raise KeyError(f'Argument provided {arg} is not a differ.File or a tuple of arguments for its constructor')
                
        self.grp_name = grp_name
        self.files = list(map(open_file, args))

    def plot1d(self, column, xlabel, 
              ylabel = 'Count',
              yscale = 'log',
              ylim = (None,None),
              out_dir = None, file_name = None,
              **hist_kwargs) :
        fig = plt.figure('differ',figsize=(11,8))
        ax = fig.subplots()

        for f in self.files :
            f.plot1d(ax, column, **hist_kwargs)

        ax.set_xlabel(xlabel)
        ax.set_ylabel(ylabel)
        ax.set_yscale(yscale)
        ax.set_ylim(*ylim)
        ax.legend(title=self.grp_name)

        if out_dir is None :
            plt.show()
        else :
            fn = column
            if file_name is not None :
                fn = file_name
            fig.savefig(os.path.join(out_dir,fn)+'.pdf', bbox_inches='tight')
            fig.clf()

    def load(self, **kwargs) :
        """Load all of the event data frames into memory
        
        The key-word arguments are used in each FileEntry's events call
        to specify which branches (if not all of them) should be loaded
        into memory and what manipulation (if any) to do.
        """
        for f in self.files :
            f.load(**kwargs)
            
    def manipulate(self, manipulation) :
        for f in self.files :
            f.manipulate(manipulation)
