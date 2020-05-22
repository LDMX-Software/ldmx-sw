"""@package histogram
Histogram configuration objects

These histogram objects are passed to the HistogramPool to be
created for processors that they are grouped with.
"""

class histogram1D: 
    """Object to hold parameters for a one-dimensional root histogram

    This histogram object will be passed to the HistogramPool and created,
    so that it is available to the processor who created it

    Attributes
    ----------
    name : str
        pointer name of histogram (like you are defining a variable)
    xlabel : str
        title of x-axis that this histogram represents
    bins : int
        number of bins
    xmin : int
        minimum value for the x-axis
    xmax : int
        maximum value for the x-axis
    """
    
    def __init__(self, name, xlabel, bins, xmin, xmax):
        self.name   = name
        self.xlabel = xlabel
        self.bins   = bins
        self.xmin   = xmin
        self.xmax   = xmax

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
        
        return "\tName: %s x Label: %s Bins: %s Range: (%s, %s)" % (self.name, 
                self.xlabel, self.bins, self.xmin, self.xmax)
