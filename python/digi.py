"""Package to configure the HCal digitization pipeline

All classes are derived versions of LDMX.Framework.ldmxcfg.Producer
with helpful member functions.

Two module-wide parameters are defined.

Attributes
----------
nPEPerMIP: float
    Number of PE created for each MIP 
mipEnergy: float
    Energy [MeV] of a single MIP 
"""

from LDMX.Framework.ldmxcfg import Producer

nPEPerMIP = 68. #PEs created per MIP 
mipEnergy = 4.66 #MeV - measured 1.4 MeV for a 6mm thick tile, so for 20mm bar = 1.4*20/6      

gain = 0.5

def HcalHgcrocEmulator() :
    """Get an HGCROC emulator and configure for the HCal specifically

    This sets the pulse shape parameters to the ones from a fit
    to a test readout of an HCal module and then thresholds to the
    default construction using 37k electrons as the number of
    electrons per MIP.
    """

    from LDMX.Tools import HgcrocEmulator
    hgcroc = HgcrocEmulator.HgcrocEmulator()

    # readout capacitance of chip is ~20pF
    hgcroc.readoutPadCapacitance = 20. #pF

    # set defaults (should swith to maxADCrange)
    hgcroc.setThresholdDefaultsHcal( gain)

    # set pulse shape parameters
    hgcroc.rateUpSlope = -0.1141
    hgcroc.timeUpSlope = -9.897
    hgcroc.rateDnSlope = 0.0279
    hgcroc.timeDnSlope = 45.037
    hgcroc.timePeak    = 9.747

    return hgcroc

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
        # assuming 1 PEs ~ 5mV ->  self.MeV = 72.961 mV/MeV (current)
        self.MeV = (1./mipEnergy)*self.hgcroc.calculateVoltageHcal( nPEPerMIP )

        # attenuation length
        self.attenuationLength = 5.; # in m   
        
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

        self.voltage_per_mip = (5/1)*(nPEPerMIP) # 5*68 mV/ MIP
        self.mip_energy = mipEnergy #MeV / MIP
        self.clock_cycle = 25. #ns - needs to match the setting on the chip   
        self.pe_per_mip = nPEPerMIP
        
        # TODO: do these need to be different for TOT/ADC modes?
        self.gain = gain
        self.pedestal = 1.

	# attenuation length
        self.attenuationLength = 5.; # in m  
        
        self.digiCollName = 'HcalDigis'
        self.digiPassName = ''
        self.simHitCollName = 'HcalSimHits'
        self.simHitPassName = ''
        self.recHitCollName = 'HcalRecHits'
