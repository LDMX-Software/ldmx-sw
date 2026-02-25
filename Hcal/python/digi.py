"""Package to configure the HCal digitization pipeline

All classes are derived versions of LDMX.Framework.ldmxcfg.Producer
with helpful member functions.

Two module-wide parameters are defined.

Attributes
----------
n_pe_per_mip: float
    Number of photo-electrons (PEs) created for each MIP 
mip_energy: float
    Energy [MeV] of a single MIP 
"""

from LDMX.Framework import Processor, processor
from LDMX.Tools.hgcroc_emulator import HgcrocEmulator


n_pe_per_mip = 68.
"""PEs created per MIP"""

mip_energy = 4.66
"""energy deposited by a single MIP

measured 1.4MeV for a 6mm thick tile, so a 20mm thick bar is ~1.4*20/6
"""


def calculate_voltage(pe):
    """Calculate the voltage signal [mV] of the input number of photo-electrons (PEs)
    
    Assuming that 1 PE ~ 5mV
    This translates to (68/4.66)*5 = 73 PE/MeV
    
    Parameters
    ----------
    pe : int
        Number of photo electrons
    """
    return pe*(5/1)


energy_to_voltage_conversion = (1./mip_energy)*calculate_voltage(n_pe_per_mip)
"""conversion from energy in MeV to voltage in mV

energy [MeV] ( 1 MIP / energy per MIP [MeV] ) ( voltage per MIP [mV] / 1 MIP ) = voltage [mV]
- assuming 1 PEs ~ 5mV ->  72.961 mV/MeV
"""


def hcal_hgcroc_emulator():
    """
    Get an HGCROC emulator and configure for the HCal specifically

    This sets the pulse shape parameters to the ones from a fit
    to a test readout of an HCal module and then thresholds to the
    default construction.

    Noise RMS is calculated using the voltage of 0.02 PEs.
    """

    # the time such that with [parameter 4]=0, the pulse peaks at t=0
    return HgcrocEmulator(
        i_soi = 3,
        n_adcs = 10,
        rate_up_slope = -0.1141,
        time_up_slope = -9.897,
        rate_dn_slope = 0.0279,
        time_dn_slope = 45.037
        time_peak    = 12.698 
    )


@parameter_set
class DigiTimeSpread:
    """Possible time smearing/shifting that can be applied during digi emulation

    either per event/spill or per hit
    """

    kind: int
    parameters: list[float]

    def __post_init__(self):
        if self.kind not in [-1, 0, 1, 2]:
            raise ValueError(
                "Invalid kind of time spread, must be -1 (No spread), 0 (Gaussian), 1 (Uniform) or 2 (Constant)"
            )

    def none():
        return DigiTimeSpread(kind = -1, parameters = [0.0])

    def gaussian(mean, sigma):
        """Gaussing time spread

        Parameters
        ----------
        mean: float
            mean of the gaussian distribution
        sigma: float
            standard deviation
        """
        return DigiTimeSpread(kind = 0, parameters = [mean, sigma])

    def uniform(min_value, max_value):
        """Uniform time spread

        Parameters
        ----------
        min_value : float
            Minimum value of the uniform distribution
        max_value : float
            Maximum value of the uniform distribution
        """
        return DigiTimeSpread(kind = 1, parameters = [min_value, max_value])

    def constant(value):
        """Constant time spread

        Parameters
        ----------
        value : float
            Value of the constant time spread
        """
        return DigiTimeSpread(kind = 2, parameters = [value])


@processor("hcal::HcalDigiProducer", "Hcal")
class HcalDigiProducer(Processor):
    """Configuration for HcalDigiProducer

    Attributes
    ----------
    hgcroc : HgcrocEmulator
        Configuration for the chip emulator
    mev : float
        Conversion between energy [MeV] and voltage [mV]
    attenuation_length: float
        attenuation length of the bars in m
    avg_readout_threshold: float
        average readout threshold in ADCs (for noise generation)
    avg_gain: float
        average channel gain (for noise in empty channels)
    avg_pedestal: float
        average channel pedestal (for noise in empty channels)
    avg_noise_rms: float
        average channel noise RMS (for noise in empty channels)
    save_pulse_truth_info: bool 
        save pulse details for later study of emulator
    pulse_truth_coll_name: str
        output name for pulse details (if being saved)
    zero_suppression: bool
        if True (default), drop channels that are not above their readout threshold
    input_coll_name : str
        Name of input collection  
    input_pass_name : str
        Name of input pass 
    digi_coll_name : str    
        Name of digi collection
    flat_time_shift: float
        flat time shift to apply to all hits
    time_spread_per_hit: DigiTimeSpread
        time spread to apply to each hit within an event
    time_spread_per_spill: DigiTimeSpread
        time spread to apply uniformly within each spill
    """

    hgcroc: HgcrocEmulator = field(default_factory = hcal_hgcroc_emulator)
    mev: float = energy_to_voltage_conversion
    attenuation_length: float = 5.0
    avg_readout_threshold: float = 4.0
    avg_gain: float = 1.2
    avg_pedestal: float = 1.0
    avg_noise_rms: float = calculate_voltage(0.02)/1.2
    save_pulse_truth_info: bool = False
    zero_suppresion: bool = True
    input_coll_name = 'HcalSimHits'
    input_pass_name = ''
    digi_coll_name = 'HcalDigis'
    pulse_truth_coll_name = 'HcalPulseTruth'
    flat_time_shift: float = 0.0
    time_spread_per_hit: DigiTimeSpread = field(default_factory = DigiTimeSpread.none)
    time_spread_per_spill: DigiTimeSpread = field(default_factory = DigiTimeSpread.none)


@processor("hcal::HcalRecProducer", "Hcal")
class HcalRecProducer(Processor):
    """Configuration for the HcalRecProducer

    Attributes
    ----------
    voltage_per_mip: float
        Conversion from voltage [mV] to number of MIPs
    mip_energy : float
        Copied from module-wide mip_energy [MeV]
    clock_cycle : float
        Time for one DAQ clock cycle to pass [ns]
    input_coll_name : str
        Name of digi collection
    input_pass_name : str
        Name of digi pass
    sim_hit_coll_name : str
        Name of simHit collection
    sim_hit_pass_name : str 
        Name of simHit pass 
    rec_hit_coll_name : str
        Name of rec_hit collection
    """

    def __init__(self, instance_name = 'hcalRecon') :
        super().__init__(instance_name , 'hcal::HcalRecProducer','Hcal')

        hgcroc = HcalHgcrocEmulator()

        self.voltage_per_mip = (5/1)*(n_pe_per_mip) # 5*68 mV/ MIP
        self.mip_energy = mip_energy #MeV / MIP
        self.clock_cycle = 25. #ns - needs to match the setting on the chip
        self.pe_per_mip = n_pe_per_mip

	    # attenuation length
        self.attenuation_length = 5. # in m

        self.input_coll_name = 'HcalDigis'
        self.input_pass_name = ''
        self.sim_hit_coll_name = 'HcalSimHits'
        self.sim_hit_pass_name = ''
        self.rec_hit_coll_name = 'HcalRecHits'

        # hgcroc parameters:
        self.rate_up_slope = hgcroc.rate_up_slope
        self.time_up_slope = hgcroc.time_up_slope
        self.rate_dn_slope = hgcroc.rate_dn_slope
        self.time_dn_slope = hgcroc.time_dn_slope
        self.time_peak    = hgcroc.time_peak
        self.n_adcs       = hgcroc.n_adcs

        # avg parameters
        self.avg_toa_threshold = 1.6 # mV - correction config only
        self.avg_gain = 1.2 # correction config only
        self.avg_pedestal = 1. #noise config only

class HcalSingleEndRecProducer(Producer) :
    """ Configuration for the single ended Hcal Rec Producer

    Attributes
    ----------
    -  mip_energy : float
       Copied from module-wide mip_energy [MeV]
    -  clock_cycle : float
       Time for one DAQ clock cycle to pass [ns]
    -  pe_per_mip: float
       number of photo-electrons per MIP
    -  pass_name: str
       Name of digi pass
    -  coll_name: str
       Name of digi collection
    -  rec_coll_name: str
       Name of rechit collection
    """

    def __init__(self, instance_name = 'hcalRecon', pass_name = '', coll_name = 'HcalDigis', rec_coll_name = 'HcalRecHits', rec_pass_name = '') :
        super().__init__(instance_name , 'hcal::HcalSingleEndRecProducer','Hcal')

        self.mip_energy = mip_energy
        self.clock_cycle = 25.
        self.pe_per_mip = n_pe_per_mip

        self.coll_name = coll_name
        self.pass_name = pass_name
        self.rec_coll_name = rec_coll_name
        self.rec_pass_name = rec_pass_name

class HcalDoubleEndRecProducer(Producer) :
    """ Configuration for the double ended Hcal Rec Producer
    
    Attributes
    ----------
    -  mip_energy : float
       Copied from module-wide mip_energy [MeV]
    -  clock_cycle : float
       Time for one DAQ clock cycle to pass [ns]
    -  pe_per_mip: float
       number of photo-electrons per MIP
    -  pass_name: str
       Name of digi pass
    -  coll_name: str
       Name of digi collection
    -  rec_coll_name: str
       Name of rechit collection
    """

    def __init__(self, instance_name = 'hcalDoubleRecon', pass_name = '', coll_name = 'HcalRecHits', rec_coll_name = 'HcalDoubleEndRecHits', rec_pass_name = '') :
        super().__init__(instance_name , 'hcal::HcalDoubleEndRecProducer','Hcal')

        self.mip_energy = mip_energy
        self.clock_cycle = 25.
        self.pe_per_mip = n_pe_per_mip

        self.coll_name = coll_name
        self.pass_name = pass_name
        self.rec_coll_name = rec_coll_name
        self.rec_pass_name = rec_pass_name

class HcalSimpleDigiAndRecProducer(Producer) :
    """Configuration for Digitization producer in the HCal
        Sets all parameters to reasonable defaults.
    Examples
    --------
        from LDMX.EventProc.hcal import HcalDigiProducer
        p.sequence.append( HcalDigiProducer() )
    """

    def __init__(self,name = 'hcalSimpleDigiAndRec') :
        super().__init__(name,'hcal::HcalSimpleDigiAndRecProducer','Hcal')
        self.input_coll_name = 'HcalSimHits'
        self.input_pass_name = ''
        self.output_coll_name = 'HcalRecHits'

        self.mean_noise = 0.02
        self.readout_threshold= 1
        self.strips_side_lr_per_layer = 12
        self.num_side_lr_hcal_layers = 26
        self.strips_side_tb_per_layer = 12
        self.num_side_tb_hcal_layers = 28
        self.strips_back_per_layer = 60 # n strips correspond to 5 cm wide bars
        self.num_back_hcal_layers = 96
        self.super_strip_size = 1 # 1 = 5 cm readout, 2 = 10 cm readout, ...
        self.mev_per_mip = 4.66  # measured 1.4 MeV for a 6mm thick tile, so for 20mm bar = 1.4*20/6
        self.pe_per_mip = 68. # PEs per MIP at 1m (assume 80% attentuation of 1m)
        self.attenuation_length = 5. # this is in m
        self.position_resolution = 150. # this is in mm
        self.sim_hit_pass_name = '' #use any pass available
