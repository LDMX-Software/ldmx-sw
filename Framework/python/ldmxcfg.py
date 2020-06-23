"""ldmxcfg

Basic python configuration for ldmx-sw application
"""

class EventProcessor:
    """An EventProcessor object

    This object contains the parameters that are necessary for a ldmx::EventProcessor to be configured.

    You should NOT use this class directly. Use one of the derived classes Producer or Analyzer for clarity.

    Parameters
    ----------
    instanceName : str
        Name of this copy of the producer object
    className : str
        Name (including namespace) of the C++ class that this processor should be

    Attributes
    ----------
    parameters : dict
        python dictionary that will be passed to the C++ object during the configure method
    histograms : list of histogram1D objects
        List of histogram configure objects for the HistogramPool to make for this processor

    See Also
    --------
    LDMX.Framework.ldmxcfg.Producer : Producer configuration object
    LDMX.Framework.ldmxcfg.Analyzer : Analyzer configuration object
    LDMX.Framework.histogram.histogram : histogram configuration object
    """

    def __init__(self, instanceName, className):
        self.instanceName=instanceName
        self.className=className
        self.parameters=dict()
        self.histograms=[]

    def build1DHistogram(self, name, xlabel, bins, xmin = None, xmax = None):
        """Make a 1D histogram 

        If xmin and xmax are not provided, bins is assumed to be
        the bin edges on the x-axis. If they are both provided,
        bins is assumed to be the number of bins on the x-axis.

        Parameters
        ----------
        name : str
            variable name of histogram
        xlabel : str
            title of x-axis of histogram
        bins : int OR list of floats
            Number of bins on x-axis OR bin edges on x-axis
        xmin : float
            Minimum edge of bins on x-axis
        xmax : float
            Maximum edge of bins on x-axis

        See Also
        --------
        LDMX.Framework.histogram.histogram : histogram configuration object
        """

        import LDMX.Framework.histogram as h
        theBinEdges = bins
        if xmin is not None and xmax is not None :
            theBinEdges = h.uniform_binning(bins,xmin,xmax)

        self.histograms.append(h.histogram(name, xlabel,theBinEdges))

    def build2DHistogram(self, name, 
            xlabel = 'X Axis', xbins = 1, xmin = None, xmax = None, 
            ylabel = 'Y Axis', ybins = 1, ymin = None, ymax = None) :
        """Create a 2D histogram

        If {x,y}min or {x,y}max are not provided, {x,y}bins is assumed
        to be the bin edges on the {x,y}-axis. If they are both provided,
        {x,y}-bins is assumed to be the number of bins on the {x,y}-axis.

        Parameters
        ----------
        name : str
            variable name of histogram
        xlabel : str
            title of x-axis of histogram
        xbins : int OR list of floats
            Number of bins on x-axis OR list of bin edges on x-axis
        xmin : float
            Minimum edge of bins on x-axis
        xmax : float
            Maximum edge of bins on x-axis
        ylabel : str
            title of y-axis of histogram
        ybins : int OR list of floats
            Number of bins on y-axis OR list of bin edges on y-axis
        ymin : float
            Minimum edge of bins on y-axis
        ymay : float
            Mayimum edge of bins on y-axis

        See Also
        --------
        LDMX.Framework.histogram.histogram : histogram configuration object

        Examples
        --------

        When doing all uniform binning, you can specify the arguments by position.
            myProcessor.build2DHistogram( 'dummy' ,
                'My X Axis' , 20 , 0. , 1. ,
                'My Y Axis' , 60 , 0. , 10. )

        When using variable binning, you have to use the parameter names.
            myProcessor.build2DHistogram( 'dummy2' ,
                xlabel='My X Axis', xbins=[0.,1.,2.],
                ylabel='My Y Axis', ybins=60, ymin=0., ymax=10. )
        """

        import LDMX.Framework.histogram as h
        theBinEdgesX = xbins
        if xmin is not None and xmax is not None :
            theBinEdgesX = h.uniform_binning(xbins,xmin,xmax)

        theBinEdgesY = ybins
        if ymin is not None and ymax is not None :
            theBinEdgesY = h.uniform_binning(ybins,ymin,ymax)

        self.histograms.append(
                h.histogram(name, xlabel,theBinEdgesX, ylabel,theBinEdgesY)
                )

class Producer(EventProcessor):
    """A producer object.

    This object contains the parameters that are necessary for a ldmx::Producer to be configured.

    See Also
    --------
    LDMX.Framwork.ldmxcfg.EventProcessor : base class
    """

    def __init__(self, instanceName, className):
        super().__init__(instanceName,className)

    def __str__(self) :
        """Stringify this Producer, creates a message with all the internal parameters.

        Returns
        -------
        str
            A message with all the parameters and member variables in a human readable format
        """

        msg = "\n  Producer(%s of class %s)"%(self.instanceName,self.className)
        if len(self.parameters)>0:
            msg += "\n   Parameters:"
            for k, v in self.parameters.items():
                msg += "\n    " + str(k) + " : " + str(v)

        if self.histograms:
            msg += "\n   Creating the following histograms:" 
            for histo in self.histograms: 
                msg += '\n    ' + str(histo)

        return msg

class Analyzer(EventProcessor):
    """A analyzer object.

    This object contains the parameters that are necessary for a ldmx::Analyzer to be configured.

    See Also
    --------
    LDMX.Framework.ldmxcfg.EventProcessor : base class
    """

    def __init__(self, instanceName, className):
        super().__init__(instanceName,className)

    def __str__(self) :
        """Stringify this Analyzer, creates a message with all the internal parameters.

        Returns
        -------
        str
            A message with all the parameters and member variables in a human readable format
        """

        msg = "\n  Analyzer(%s of class %s)"%(self.instanceName,self.className)
        if len(self.parameters)>0:
            msg += "\n   Parameters:"
            for k, v in self.parameters.items():
                msg += "\n    " + str(k) + " : " + str(v)

        if self.histograms:
            msg += "\n   Creating the following histograms:" 
            for histo in self.histograms: 
                msg += '\n    ' + str(histo)

        return msg
                
class Process:
    """Process configuration object

    The python object that stores the necessary parameters for configuring
    a Process for ldmx-app to execute.

    Upon construction, the class-wide reference lastProcess is set
    and the rest of the attributes are set to sensible defaults.

    Parameters
    ----------
    passName : str
        Short reference name for this run of the process

    Attributes
    ----------
    lastProcess : Process
        Class-wide reference to the last Process object to be constructed
    maxEvents : int
        Maximum number events to process
    run : int
        Run number for this process
    inputFiles : list of strings
        Input files to read in event data from and process
    outputFiles : list of strings
        Output files to write out event data to after processing
    sequence : list of Producers and Analyzers
        List of event processors to pass the event bus objects to
    keep : list of strings
        List of rules to keep or drop objects from the event bus
    libraries : list of strings
        List of libraries to load before attempting to build any processors
    skimDefaultIsKeep : bool
        Flag to say whether to process should by default keep the event or not
    skimRules : list of strings
        List of skimming rules for which processors the process should listen to when deciding whether to keep an event
    logFrequency : int
        Print the event number whenever its modulus with this frequency is zero

    See Also
    --------
    Producer : one type of event processor
    Analyzer : the other type of event processor
    """

    lastProcess=None
    
    def __init__(self, passName):
        self.passName=passName
        self.maxEvents=-1
        self.run=-1
        self.inputFiles=[]
        self.outputFiles=[]
        self.sequence=[]
        self.keep=[]
        self.libraries=[]
        self.skimDefaultIsKeep=True
        self.skimRules=[]
        self.logFrequency=-1
        self.compressionSetting=9
        Process.lastProcess=self

    def skimDefaultIsSave(self):
        """Configure the process to by default keep every event."""

        self.skimDefaultIsKeep=True
        
    def skimDefaultIsDrop(self):
        """Configure the process to by default drop (not save) every event."""

        self.skimDefaultIsKeep=False

    def skimConsider(self,namePat):
        """Configure the process to listen to processors matching input.

        The list of skim rules has a rather complicated form, so it
        is better to use this helper function.

        Parameters
        ----------
        namePat : str
            Pattern for the processor instanceNames to match for the Process to listen

        Example
        -------
        >>> ecalVeto = ldmxcfg.Producer( 'ecalVeto' , 'EcalVetoProcessor' )
        >>> # Setup of other parameters for the veto
        >>> p.skimConsider( 'ecalVeto' )

        See Also
        --------
        skimConsiderLabelled

        """

        self.skimRules.append(namePat)
        self.skimRules.append("")

    def skimConsiderLabelled(self,namePat,labelPat):
        """Configure the process to listen to processors matching input.

        The list of skim rules has a rather complicated form, so it
        is better to use this helper function.

        Some uses of ``setStorageHint`` in processors include a "reason"
        for the given storage hint. The input label pattern is check
        on matching this "reason" for the storage hint.

        Parameters
        ----------
        namePat : str
            Pattern for the processor instanceNames to match for the Process to listen
        labelPat : str
            Pattern for the storage hint reason to match for the Process to listen

        See Also
        --------
        skimConsider

        """
        self.skimRules.append(namePat)
        self.skimRules.append(labelPat)

    def setCompression(self,algorithm,level=9):
        """set the compression settings for any output files in this process

        We combine the compression settings here in the same way that ROOT
        does. This allows the compression settings to be passed along as
        one integer rather than two without any loss of generality.

        Look at ROOT's documentation for TFile to learn more
        about the different compression algorithms and levels available
        (as well as what integers to use). There is a summary table
        below.

        Algorithm           | int | root version
        ------------------- | --- | ------------
        ROOT global default |  0  | root-6.16
        ZLIB                |  1  | root-6.16
        LZMA                |  2  | root-6.16
        Old (ROOT 5)        |  3  | root-6.16
        LZ4                 |  4  | root-6.16
        ZSTD                |  5  | root-6.20

        Level 0 : no compression is applied
        Level 9 : maximum amount of compression available from algorithm

        Parameters
        ----------
        algorithm : int
            flag for the algorithm to use
        level : int
            flag for the level of compression to use
        """

        self.compressionSetting = algorithm*100 + level

    def __str__(self):
        """Stringify this object into a human readable, helpful form.

        This function creates a very large, multi-line string that reports (almost) all of the important
        details of this configured process.

        Returns
        -------
        str
            A human-readable, multi-line description of this process object
        """

        msg = "Process with pass name '%s'"%(self.passName)
        if (self.run>0): msg += "\n using run number %d"%(self.run)
        if (self.maxEvents>0): msg += "\n Maximum events to process: %d"%(self.maxEvents)
        else: msg += "\n No limit on maximum events to process"
        msg += "\n Processor sequence:"
        for proc in self.sequence:
            msg += str(proc)
        if len(self.inputFiles) > 0:
            if len(self.outputFiles)==len(self.inputFiles):
                msg += "\n Files:"
                for i in range(0,len(self.inputFiles)):
                    msg += "\n  '%s' -> '%s'"%(self.inputFiles[i],self.outputFiles[i])
            else:
                msg += "\n Input files:"
                for afile in self.inputFiles:
                    msg += '\n  ' + afile
                if len(self.outputFiles) > 0:
                    msg += "\n Output file: " + self.outputFiles[0]
        elif len(self.outputFiles) > 0:
            msg += "\n Output file: " + self.outputFiles[0]
        msg += "\n Skim rules:"
        if self.skimDefaultIsKeep: msg += "\n  Default: keep the event"
        else: msg += "\n  Default: drop the event"
        for i in range(0,len(self.skimRules)-1,2):
            if self.skimRules[i+1]=="": 
                msg += "\n  Listen to hints from processors with names matching '%s'"%(self.skimRules[i])
            else:
                msg += "\n  Listen to hints with labels matching '%s' from processors with names matching '%s'"%(self.skimRules[i+1],self.skimRules[i])
        if len(self.keep) > 0:
            msg += "\n Rules for keeping previous products:"
            for arule in self.keep:
                msg += '\n  ' + arule
        if len(self.libraries) > 0:
            msg += "\n Shared libraries to load:"
            for afile in self.libraries:
                msg += '\n  ' + afile

        return msg

    
