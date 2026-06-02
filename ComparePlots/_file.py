"""Wrap an uproot file for some extra help plotting"""

import logging
import os

import hist
import mplhep
import uproot


class File:
    """File entry in Differ object holding histogram objects

    Parameters
    ----------
    filepath : str or pathlib.Path
        path specifying ROOT file to open for reading
    hist_kwargs : dict
        dictionary providing extra detail for the hist plotting call
        helpful for specifying style options and other defaults for hists from this file
    open_kwargs : dict
        all other key-word arguments are passed to uproot.open
    """

    log = logging.getLogger("File")

    def __init__(self, filepath, hist_kwargs=None, **open_kwargs):
        if hist_kwargs is None:
            hist_kwargs = {}
        self.root_file = uproot.open(filepath, **open_kwargs)
        self.hist_kwargs = hist_kwargs

    def __repr__(self):
        """Represent this File in a short form"""
        return "File { Histograms labeled " + self.hist_kwargs["label"] + " }"

    @staticmethod
    def from_path(filepath, legendlabel_parameter=None):
        """Extract the legend-label for histograms from this file using the filepath

        This is separated from the contsructor because

        1. It makes assumptions on how the file name is formatted
        2. It does not allow for modification of the keyword args in the constructor

        The parameters of a file are extracted from its name.
        In general, we assume that the filename is of the form

        <key1>_<val1>_<key2>_<val2>_...<keyN>_<valN>.root

        i.e. a mapping where the key/val pairs are separated by underscores.

        If the 'legendlabel_parameter' is not provided, we simply use the first key-val
        pair in the file name as the legend-label.

        Parameters
        ----------
        filepath : str
            Full path to the file to open
        legendlabel_parameter : str, optional
            key-name to use in legend-label for this File
        """
        fn = os.path.basename(filepath).replace(".root", "")
        parts = fn.split("_")
        if len(parts) % 2 != 0:
            raise ValueError(
                f"The filename provided {fn} cannot be"
                " split into key_val pairs."
                "\n\tWorking example: hist_new.root"
            )
        file_params = {
            parts[i]: parts[i + 1] for i in range(len(parts) - 1) if i % 2 == 0
        }
        File.log.debug(f"Deduced File Parameters: {file_params}")

        if legendlabel_parameter is None:
            legendlabel_parameter = [next(iter(file_params))]
        ll = [file_params[key] for key in legendlabel_parameter if key in file_params]
        if len(ll) < len(legendlabel_parameter):
            missing_params = set(legendlabel_parameter) - set(file_params.keys())
            raise KeyError(
                f"{', '.join(missing_params)} not in"
                f" deduced file parameters:\n{file_params}"
            )
        ll = "_".join(ll)

        File.log.debug(f"Deduced File Label: {ll}")
        return File(filepath, hist_kwargs={"label": ll})

    def keys(self, *args, **kwargs):
        """Callback into uproot keys

        Helpful for exploring the file when trying to decide
        what to plot within a notebook
        """
        return self.root_file.keys(*args, **kwargs)

    @property
    def path(self):
        """Retrun the path to the file on disk"""
        return self.root_file.file_path

    def get(self, obj):
        """Get the input ROOT histogram by name and return it as a hist.Hist"""
        return self.root_file[obj].to_hist()
