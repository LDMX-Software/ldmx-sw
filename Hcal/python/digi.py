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

hcal_pulse_rate_up_slope = -0.1141
hcal_pulse_time_up_slope = -9.897
hcal_pulse_rate_dn_slope = 0.0279
hcal_pulse_time_dn_slope = 45.037
hcal_pulse_time_peak = 12.698

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
        rate_up_slope = hcal_pulse_rate_up_slope,
        time_up_slope = hcal_pulse_time_up_slope,
        rate_dn_slope = hcal_pulse_rate_dn_slope,
        time_dn_slope = hcal_pulse_time_dn_slope,
        time_peak     = hcal_pulse_time_peak
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
    pe_per_mip: float
        average number of PE per MIP
    attenuation_length: float
        attenuation length of bar [m]
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
    rate_up_slope: float
        use to implement amplitude and time walk correction
    time_up_slope: float
        use to implement amplitude and time walk correction
    rate_dn_slope: float
        use to implement amplitude and time walk correction
    time_dn_slope: float
        use to implement amplitude and time walk correction
    time_peak: float
        use to implement amplitude and time walk correction
    n_adcs: int
        use to implement amplitude and time walk correction
    avg_toa_threshold: float
        use to implement amplitude and time walk correction
    avg_gain: float
        use to implement amplitude and time walk correction
    avg_pedestal: float
        use to implement amplitude and time walk correction
    """

    voltage_per_mip: float = (5/1)*n_pe_per_mip
    mip_energy: float = mip_energy
    clock_cycle: float = 25.0
    pe_per_mip: float = n_pe_per_mip
    attenuation_length: float = 5.0
    input_coll_name: str = 'HcalDigis'
    input_pass_name: str = ''
    sim_hit_coll_name: str = 'HcalSimHits'
    sim_hit_pass_name: str = ''
    rec_hit_coll_name: str = 'HcalRecHits'
    rate_up_slope: float = hcal_pulse_rate_up_slope
    time_up_slope: float = hcal_pulse_time_up_slope
    rate_dn_slope: float = hcal_pulse_rate_dn_slope
    time_dn_slope: float = hcal_pulse_time_dn_slope
    time_peak: float = hcal_pulse_time_peak
    n_adcs: int = 10
    avg_toa_threshold: float = 1.6 # mV - correction config only
    avg_gain: float = 1.2 # correction config only
    avg_pedestal: float = 1. #noise config only


@processor("hcal::HcalSingleEndRecProducer", "Hcal")
class HcalSingleEndRecProducer(Processor):
    """ Configuration for the single ended Hcal Rec Producer

    Attributes
    ----------
    mip_energy : float
       Copied from module-wide mip_energy [MeV]
    clock_cycle : float
       Time for one DAQ clock cycle to pass [ns]
    pe_per_mip: float
       number of photo-electrons per MIP
    pass_name: str
       Name of digi pass
    coll_name: str
       Name of digi collection
    rec_coll_name: str
       Name of rechit collection
    """

    mip_energy: float = mip_energy
    clock_cycle: float = 25.0
    pe_per_mip: float = pe_per_mip
    coll_name: str = "HcalDigis"
    pass_name: str = ""
    rec_coll_name: str = "HcalRecHits"


@processor("hcal::HcalDoubleEndRecProducer", "Hcal")
class HcalDoubleEndRecProducer(Processor):
    """ Configuration for the double ended Hcal Rec Producer
    
    Attributes
    ----------
    mip_energy : float
       Copied from module-wide mip_energy [MeV]
    clock_cycle : float
       Time for one DAQ clock cycle to pass [ns]
    pe_per_mip: float
       number of photo-electrons per MIP
    pass_name: str
       Name of digi pass
    coll_name: str
       Name of digi collection
    rec_coll_name: str
       Name of rechit collection
    """

    mip_energy: float = mip_energy
    clock_cycle: float = 25.0
    pe_per_mip: float = pe_per_mip
    coll_name: str = "HcalRecHits"
    pass_name: str = ""
    rec_coll_name: str = "HcalDoubleEndRecHits"
    rec_pass_name: str = ""


@processor("hcal::HcalSimpleDigiAndRecProducer", "Hcal")
class HcalSimpleDigiAndRecProducer(Processor):
    """Configuration for Digitization producer in the HCal

    Legacy digi emulation and reconstruction producer which creates rec hits
    without as detailed of a digi emulation or realistic reconstruction procedure.
    """

    input_coll_name: str = 'HcalSimHits'
    input_pass_name: str = ''
    output_coll_name: str = 'HcalRecHits'
    mean_noise: float = 0.02
    readout_threshold: int = 1
    strips_side_lr_per_layer: int = 12
    num_side_lr_hcal_layers: int = 26
    strips_side_tb_per_layer: int = 12
    num_side_tb_hcal_layers: int = 28
    strips_back_per_layer: int = 60 # n strips correspond to 5 cm wide bars
    num_back_hcal_layers: int = 96
    super_strip_size: int = 1 # 1 = 5 cm readout, 2 = 10 cm readout, ...
    mev_per_mip: float = 4.66  # measured 1.4 MeV for a 6mm thick tile, so for 20mm bar = 1.4*20/6
    pe_per_mip: float = 68. # PEs per MIP at 1m (assume 80% attentuation of 1m)
    attenuation_length: float = 5. # this is in m
    position_resolution: float = 150. # this is in mm
    sim_hit_pass_name: str = '' #use any pass available
