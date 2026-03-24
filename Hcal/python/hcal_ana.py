"""Configuration for standard Hcal analysis scripts"""

from LDMX.Framework import Processor, processor


@processor("hcal::HcalPedestalAnalyzer", "Hcal")
class HcalPedestalAnalyzer(Processor):
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
    filter_no_toa : bool
        Ignore any event for a channel where the TOA fired in any sample (default=true)
    filter_no_tot : bool
        Ignore any event for a channel where the TOT fired in any sample (default=true)
    low_cutoff : int
        Ignore any event for a channel where any sample was below this level (default=10)
    high_cutoff : int
        Ignore any event for a channel where any sample was above this level (default=300)
    comments : str
        Comments to put into the output CSV file for logging purposes


    Examples
    --------
        from LDMX.Hcal.hcal_ana import HcalPedestalAnalyzer
        p.sequence.append( HcalPedestalAnalyzer(
            input_name = "HcalDigis",
            output_file = "pedestals_per_channel.txt"
        ) )
    """

    input_name: str
    output_file: str
    input_pass: str = ""
    make_histos: bool = False
    filter_no_tot: bool = True
    filter_no_toa: bool = True
    low_cutoff: int = 10
    high_cutoff: int = 300
    comments: str = ""
