"""Package to configure the HCal digitization pipeline

All classes are derived versions of LDMX.Framework.ldmxcfg.Producer
with helpful member functions.

Two module-wide parameters are defined.

Attributes
----------
nElectronsPerMIP : float
    Number of e-h pairs created for each MIP in 0.5mm thick Si
mipSiEnergy : float
    Energy [MeV] of a single MIP on average in 0.5mm thick Si
"""

from LDMX.Framework.ldmxcfg import Producer

nElectronsPerMIP = 37000.0 #e-h pairs created per MIP <- derived from 0.5mm thick Si
mipSiEnergy = 0.130 #MeV - corresponds to ~3.5 eV per e-h pair <- derived from 0.5mm thick Si

def HcalHgcrocEmulator() :
    """Get an HGCROC emulator and configure for the HCal specifically

    This sets the pulse shape parameters to the ones from a fit
    to a test readout of an HCal module and then thresholds to the
    default construction using 37k electrons as the number of
    electrons per MIP.
    """

    from LDMX.Tools import HgcrocEmulator
    hgcroc = HgcrocEmulator.HgcrocEmulator()

    # set defaults with 37k electrons per MIP
    hgcroc.setThresholdDefaults( nElectronsPerMIP )

    # set pulse shape parameters
    hgcroc.rateUpSlope =  -0.345
    hgcroc.timeUpSlope = 70.6547
    hgcroc.rateDnSlope = 0.140068
    hgcroc.timeDnSlope = 87.7649
    hgcroc.timePeak    = 77.732

    return hgcroc

class HcalDigiProducer(Producer) :
    """Configuration for HcalDigiProducer

    Attributes
    ----------
    hgcroc : HgcrocEmulator
        Configuration for the chip emulator
    MeV : float
        Conversion between energy [MeV] and voltage [mV]
    nHcalLayers : int
        Number of Si layer in HCal, needed to generate noise ID
    nModulesPerLayer : int
        Number of modules in each layer, needed to generate noise ID
    nCellsPerModule : int
        Number of cells in each module, needed to generate noise ID
    """

    def __init__(self, instance_name = 'hcalDigis') :
        super().__init__(instance_name , 'ldmx::HcalDigiProducer','Hcal')

        self.hgcroc = HcalHgcrocEmulator()

        #Energy -> Volts converstion
        #   energy [MeV] ( 1 MIP / energy per MIP [MeV] ) ( voltage per MIP [mV] / 1 MIP ) = voltage [mV]
        #   this leads to ~ 470 mV/MeV or ~6.8 MeV maximum hit (if 320 fC is max ADC range)
        self.MeV = (1./mipSiEnergy)*self.hgcroc.calculateVoltage( nElectronsPerMIP )

        import time
        self.randomSeed = int(time.time())

        # hcal hexagon geometry parameters
        # used for putting noise into empty channels
        self.nHcalLayers      = 34
        self.nModulesPerLayer = 7
        self.nCellsPerModule  = 397
