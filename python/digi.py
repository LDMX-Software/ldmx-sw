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

        # noise (0.02PE)
        self.noiseRMS = self.calculateVoltageHcal(0.02) # mV

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
    inputCollName : str
        Name of input collection  
    inputPassName : str
        Name of input pass 
    digiCollName : str    
        Name of digi collection                                                                                                                                                                          
    """

    def __init__(self, instance_name = 'hcalDigis') :
        super().__init__(instance_name , 'hcal::HcalDigiProducer','Hcal')

        self.hgcroc = HcalHgcrocEmulator()

        #Energy -> Volts converstion
        # energy [MeV] ( 1 MIP / energy per MIP [MeV] ) ( voltage per MIP [mV] / 1 MIP ) = voltage [mV]
        # assuming 1 PEs ~ 5mV ->  self.MeV = 72.961 mV/MeV
        self.MeV = (1./mipEnergy)*self.hgcroc.calculateVoltageHcal( nPEPerMIP )

        # attenuation length
        self.attenuationLength = 5.; # in m   

        # avg parameters
        self.avgReadoutThreshold = 4. #ADCs - noise config only
        self.avgGain = 1.2 #noise config only
        self.avgPedestal = 1. #noise config only
        
        # input and output collection name parameters
        self.inputCollName = 'HcalSimHits'
        self.inputPassName = ''
        self.digiCollName = 'HcalDigis'

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
    digiCollName : str
        Name of digi collection
    digiPassName : str
        Name of digi pass
    simHitCollName : str
        Name of simHit collection
    simHitPassName : str 
        Name of simHit pass 
    recHitCollName : str
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
        self.attenuationLength = 5.; # in m  
        
        self.digiCollName = 'HcalDigis'
        self.digiPassName = ''
        self.simHitCollName = 'HcalSimHits'
        self.simHitPassName = ''
        self.recHitCollName = 'HcalRecHits'

        # hgcroc parameters:
        self.rateUpSlope = hgcroc.rateUpSlope
        self.timeUpSlope = hgcroc.timeUpSlope
        self.rateDnSlope = hgcroc.rateDnSlope
        self.timeDnSlope = hgcroc.timeDnSlope
        self.timePeak    = hgcroc.timePeak
        self.nADCs       = hgcroc.nADCs

        # avg parameters
        self.avgToaThreshold = 1.6 # mV - correction config only
        self.avgGain = 1.2 # correction config only 
        self.avgPedestal = 1. #noise config only   
