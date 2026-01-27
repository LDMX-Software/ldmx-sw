"""Package to configure the HCal digitization pipeline

All classes are derived versions of LDMX.Framework.ldmxcfg.Producer
with helpful member functions.

Two module-wide parameters are defined.

Attributes
----------
nPEPerMIP: float
    Number of photo-electrons (PEs) created for each MIP 
mipEnergy: float
    Energy [MeV] of a single MIP 
"""

from LDMX.Framework.ldmxcfg import Producer

from LDMX.Tools.HgcrocEmulator import HgcrocEmulator

nPEPerMIP = 68. #PEs created per MIP 
mipEnergy = 4.66 #MeV - measured 1.4 MeV for a 6mm thick tile, so for 20mm bar = 1.4*20/6      

class HcalHgcrocEmulator(HgcrocEmulator) :
    """
    Get an HGCROC emulator and configure for the HCal specifically
    This sets the pulse shape parameters to the ones from a fit
    to a test readout of an HCal module and then thresholds to the
    default construction.
    Noise RMS is calculated using the voltage of 0.02 PEs.
    """

    def __init__(self) :
        super().__init__()

        # SOI
        # Sample of interest (will have double of samples (6) after pulse peak)
        self.iSOI = 3

        # nADCs
        self.nADCs = 10

        # set pulse shape parameters
        self.rateUpSlope = -0.1141
        self.timeUpSlope = -9.897
        self.rateDnSlope = 0.0279
        self.timeDnSlope = 45.037
        self.timePeak    = 12.698 # the time such that with [parameter 4]=0, the pulse peaks at t=0

    def calculateVoltageHcal(self, PE) :
        """Calculate the voltage signal [mV] of the input number of photo-electrons (PEs)
        Assuming that 1 PE ~ 5mV
        This translates to (68/4.66)*5 = 73 PE/MeV
        Parameters
        ----------
        PE : int
             Number of photo electrons
        """
        return PE*(5/1)
    
class HcalDigiProducer(Producer) :
    """Configuration for HcalDigiProducer

    Attributes
    ----------
    hgcroc : HgcrocEmulator
        Configuration for the chip emulator
    MeV : float
        Conversion between energy [MeV] and voltage [mV]
    input_coll_name : str
        Name of input collection  
    input_pass_name : str
        Name of input pass 
    digi_coll_name : str    
        Name of digi collection                                                                                                                                                                          
    """

    def __init__(self, instance_name = 'hcalDigis') :
        super().__init__(instance_name , 'hcal::HcalDigiProducer','Hcal')

        self.hgcroc = HcalHgcrocEmulator()

        #Energy -> Volts converstion
        # energy [MeV] ( 1 MIP / energy per MIP [MeV] ) ( voltage per MIP [mV] / 1 MIP ) = voltage [mV]
        # assuming 1 PEs ~ 5mV ->  self.MeV = 72.961 mV/MeV
        self.mev = (1./mipEnergy)*self.hgcroc.calculateVoltageHcal( nPEPerMIP )

        # attenuation length
        self.attenuation_length = 5.; # in m   
        # avg parameters
        self.avg_readout_threshold = 4. #ADCs - noise config only
        self.avg_gain = 1.2 #noise config only
        self.avg_pedestal = 1. #noise config only
        # avg noise set to 0.02PE
        self.avg_noise_rms = self.hgcroc.calculateVoltageHcal(0.02)/self.avg_gain
        self.save_pulse_truth_info = False

        # If false, digitize every channel, by not dropping any pulses
        # to e.g. simulate a pedestal measurement
        self.zero_suppression = True
        # input and output collection name parameters
        self.input_coll_name = 'HcalSimHits'
        self.input_pass_name = ''
        self.digi_coll_name = 'HcalDigis'
        self.pulse_truth_coll_name = 'HcalPulseTruth'

class HcalRecProducer(Producer) :
    """Configuration for the HcalRecProducer

    Attributes
    ----------
    voltage_per_mip: float
        Conversion from voltage [mV] to number of MIPs
    mip_energy : float
        Copied from module-wide mipEnergy [MeV]
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
        Name of recHit collection
    """

    def __init__(self, instance_name = 'hcalRecon') : 
        super().__init__(instance_name , 'hcal::HcalRecProducer','Hcal')

        hgcroc = HcalHgcrocEmulator()

        self.voltage_per_mip = (5/1)*(nPEPerMIP) # 5*68 mV/ MIP
        self.mip_energy = mipEnergy #MeV / MIP
        self.clock_cycle = 25. #ns - needs to match the setting on the chip   
        self.pe_per_mip = nPEPerMIP
        
	    # attenuation length
        self.attenuation_length = 5.; # in m  
        
        self.input_coll_name = 'HcalDigis'
        self.input_pass_name = ''
        self.sim_hit_coll_name = 'HcalSimHits'
        self.sim_hit_pass_name = ''
        self.rec_hit_coll_name = 'HcalRecHits'

        # hgcroc parameters:
        self.rate_up_slope = hgcroc.rateUpSlope
        self.time_up_slope = hgcroc.timeUpSlope
        self.rate_dn_slope = hgcroc.rateDnSlope
        self.time_dn_slope = hgcroc.timeDnSlope
        self.time_peak    = hgcroc.timePeak
        self.n_adcs       = hgcroc.nADCs

        # avg parameters
        self.avg_toa_threshold = 1.6 # mV - correction config only
        self.avg_gain = 1.2 # correction config only 
        self.avg_pedestal = 1. #noise config only   

class HcalSingleEndRecProducer(Producer) :
    """ Configuration for the single ended Hcal Rec Producer

    Attributes
    ----------
    -  mip_energy : float
       Copied from module-wide mipEnergy [MeV]
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

        self.mip_energy = mipEnergy
        self.clock_cycle = 25.
        self.pe_per_mip = nPEPerMIP
        
        self.coll_name = coll_name
        self.pass_name = pass_name
        self.rec_coll_name = rec_coll_name
        self.rec_pass_name = rec_pass_name

class HcalDoubleEndRecProducer(Producer) :
    """ Configuration for the double ended Hcal Rec Producer
    
    Attributes
    ----------
    -  mip_energy : float
       Copied from module-wide mipEnergy [MeV]
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

        self.mip_energy = mipEnergy
        self.clock_cycle = 25.
        self.pe_per_mip = nPEPerMIP

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
