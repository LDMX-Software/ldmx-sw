"""Histogram configuration objects

These histogram objects are passed to the HistogramPool to be
created for processors that they are grouped with.
"""

def uniform_binning( nbins , minedge , maxedge ) :
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

    bin_width = float(maxedge-minedge)/float(nbins)

    #range does not include upper limit, so we need to add an extra bin at end
    return [ bin_width*ibin+minedge for ibin in range(nbins+1) ]

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
    """
    
    def __init__(self, name, xlabel, xbins, ylabel='', ybins=[]) :
        self.name   = name
        self.xlabel = xlabel
        self.xbins  = xbins
        self.ylabel = ylabel
        self.ybins  = ybins

    def __repr__(self):
        """Represent this object to the human user

        Returns
        -------
        A string representation of the histogram displaying its properties.
        """

        if len(self.ybins) > 0 :
            return "Name: %s x Label: %s y Label: %s" % (self.name, 
                self.xlabel, self.ylabel)
        else :
            return "Name: %s x Label: %s" % (self.name,self.xlabel)     

    def __str__(self):
        """Stringify this object. 
        
        Helpful for printing it in python to make sure the passed variables are what you want.

        Returns
        -------
        A string representation of the histogram displaying its properties.

        Example
        -------
        This function allows you to do things like:
            print(myHistogram)
        or
            "The histogram in my processor is %s" % ( myHistogram )
        """
        
        return self.__repr__()
