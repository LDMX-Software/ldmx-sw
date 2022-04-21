"""Configuration for standard Hcal analysis scripts"""

from LDMX.Framework import ldmxcfg

class HcalPedestalAnalyzer(ldmxcfg.Analyzer) :
    """Constructs standard pedestal text files and
    optionally produces histograms for each channel.

    Attributes
    ----------
    input_name : str
        Name of the input collection
    input_pass : str
        Name of the input pass (or empty for default)
    output_file : str
        Filename for output calibration file
    make_histos : bool
        Make individual channel histograms (default = false)
    filter_noTOA : bool
        Ignore any event for a channel where the TOA fired in any sample (default=true)
    filter_noTOT : bool
        Ignore any event for a channel where the TOT fired in any sample (default=true)
    low_cutoff : int
        Ignore any event for a channel where any sample was below this level (default=10)
    high_cutoff : int
        Ignore any event for a channel where any sample was above this level (default=300)
    comments : str
        Comments to put into the output CSV file for logging purposes


    Examples
    --------
        from LDMX.EventProc.hcal import HcalPedestalAnalyzer
        p.sequence.append( HcalPedestalAnalyzer() )
    """

    def __init__(self,name = 'hcal_ped_ana', input_name="", input_pass="", output_file="", make_histos=False,
                 filter_noTOT=True, filter_noTOA=True, low_cutoff=10, high_cutoff=300, comments="") :
        super().__init__(name,'hcal::HcalPedestalAnalyzer','Hcal')

        self.input_name = input_name
        self.input_pass= input_pass
        self.output_file = output_file
        self.make_histos = make_histos
        self.filter_noTOT = filter_noTOT
        self.filter_noTOA = filter_noTOA
        self.low_cutoff = low_cutoff
        self.high_cutoff = high_cutoff
        self.comments=comments


