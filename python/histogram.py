
class histogram1D: 
    
    def __init__(self, name, xlabel, bins, xmin, xmax):
        self.name   = name
        self.xlabel = xlabel
        self.bins   = bins
        self.xmin   = xmin
        self.xmax   = xmax

    def __str__(self):
        
        return "\tName: %s x Label: %s Bins: %s Range: (%s, %s)" % (self.name, 
                self.xlabel, self.bins, self.xmin, self.xmax)
