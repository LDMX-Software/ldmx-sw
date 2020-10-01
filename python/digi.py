"""Package to configure the HCal digitization pipeline

All classes are derived versions of LDMX.Framework.ldmxcfg.Producer
with helpful member functions.

Two module-wide parameters are defined.

Attributes
----------
nPEPerMIP: float
    Number of PE created for each MIP for a 20 mm scintillator tile
mipSiEnergy: float
    Energy [MeV] of a single MIP on average in 20 mm scintillator
"""

from LDMX.Framework.ldmxcfg import Producer

nPEPerMIP = 68. #PEs created per MIP 
mipSiEnergy = 4.66 #MeV - measured 1.4 MeV for a 6mm thick tile, so for 20mm bar = 1.4*20/6      

def HcalHgcrocEmulator() :
    """Get an HGCROC emulator and configure for the HCal specifically

    This sets the pulse shape parameters to the ones from a fit 
    to a test readout of an HCal module and then thresholds to 
    1 PE using 68 as the number of PEs per MIP.
    """

    from LDMX.Tools import HgcrocEmulator
    hgcroc = HgcrocEmulator.HgcrocEmulator()

    hgcroc.pedestal = 50. # mV 
    hgcroc.readoutThreshold = hgcroc.calculateVoltagePE(1.) # [mV] # readout threshold 1 PE
    hgcroc.toaThreshold = 7*50 + hgcroc.calculateVoltagePE( 5.*nPEPerMIP) # toa is 5 MIPs above the pedestal?
    hgcroc.totThreshold = 1000000. # just leave tot mode out for now

    # set pulse shape parameters                                                                                                                                              
    hgcroc.rateUpSlope = -0.983
    hgcroc.timeUpSlope = 9.897
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

        #Energy -> PE converstion
        # energy [MeV] ( 1 MIP / energy per MIP [MeV] ) ( voltage per MIP [mV] / 1 MIP ) = voltage [mV]                                                                   
        # assuming 7 PEs ~ 2.5V ~ 2500 mV                                                                                                                                     
        # this leads to ~ 3.64 V/MeV  ~ 3640 mV/MeV                                                                                                                           
        # max ADC range should be larger than 320fC - maybe even pC?                                                                                                               
        self.MeV = (1./mipSiEnergy)*self.hgcroc.calculateVoltagePE(nPEPerMIP)
