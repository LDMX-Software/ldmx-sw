"""Histogram configuration objects

These histogram objects are passed to the HistogramPool to be
created for processors that they are grouped with.
"""

from ._parameter_set import field, parameter_set


def uniform_binning(nbins, minedge, maxedge):
    """Create a list of bin edges uniformly separated

    Parameters
    ----------
    nbins : int
        Number of bins
    minedge : float
        Lower edge of binning
    maxedge : float
        Upper edge of binning
    """

    bin_width = float(maxedge - minedge) / float(nbins)

    # range does not include upper limit, so we need to add an extra bin at end
    return [bin_width * ibin + minedge for ibin in range(nbins + 1)]


@parameter_set
class histogram:
    """Object to hold parameters for a one-dimensional root histogram

    This histogram object will be passed to the HistogramPool and created,
    so that it is available to the processor who created it.

    If the ybins member is empty, then the histogram is assumed to be a 1D
    histogram.

    Parameters
    ----------
    name : str
        pointer name of histogram (like you are defining a variable)
    xlabel : str
        title of x-axis that this histogram represents
    xbins : list of floats
        bin edges along x-axis
    ylabel : str
        title of y-axis that this histogram represents
    ybins : list of floats
        bin edges along y-axis
    weighted: bool
        whether to keep track of sum of squared weights
    """

    name: str
    xlabel: str = ""
    xbins: list[float] = []
    xcategories: list[str] = field(init=False)
    ylabel: str = ""
    ybins: list[float] = []
    ycategories: list[str] = field(init=False)
    weighted: bool = False

    def __post_init__(self):
        if len(self.xbins) == 0:
            raise ValueError("Cannot have a histogram with zero bins.")

        if isinstance(self.xbins[0], str):
            self.xcategories = self.xbins
            self.xbins = [0.0]
        else:
            self.xcategories = []

        if len(self.ybins) == 0:
            self.ycategories = []
        elif isinstance(self.ybins[0], str):
            self.ycategories = self.ybins
            self.ybins = []
        else:
            self.ycategories = []
