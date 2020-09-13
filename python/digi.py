"""Package to configure the HCal digitization pipeline

All classes are derived versions of LDMX.Framework.ldmxcfg.Producer
with helpful member functions.

Two module-wide parameters are defined.

"""

from LDMX.Framework.ldmxcfg import Producer

nPEPerMIP = 68. #PEs created per MIP 
mipSiEnergy = 4.66 #MeV - measured 1.4 MeV for a 6mm thick tile, so for 20mm bar = 1.4*20/6      

def HcalHgcrocEmulator() :
    """Get an HGCROC emulator and configure for the HCal specifically

    This sets the pulse shape parameters to the ones from a fit
    to a test readout of an HCal module 
    """

    from LDMX.Tools import HgcrocEmulator
    hgcroc = HgcrocEmulator.HgcrocEmulator()

    '''
    The default calculation for the different thresholds if the following:                                                                                                       
    - readout is 4 times the rms noise above the pedestal                                                                                                                        
    - toa is 5 MIPs above the pedestal                                                                                                                                           
    - tot is 50 MIPs above the pedestal 
    '''
    hgcroc.gain = 1. # need to change this?
    hgcroc.readoutThreshold = 1. # PE                                                                                                                                             
    hgcroc.toaThreshold = 1.
    hgcroc.totThreshold = 100000.
    hgcroc.pedestal = 1.

    # set pulse shape parameters                                                                                                                                              
    # "[0]/(1.0+exp([1]*(x-[2]+[3]-[4])))/(1.0+exp([5]*(x-[6]+[3]-[4])))",                                                                                                       
    # /(-0.969(x+58.641-61.5518) /0.0288779*(x+23.3814-61.5518)                                                                                                         
    hgcroc.rateUpSlope = -0.969
    hgcroc.timeUpSlope = -58.641
    hgcroc.rateDnSlope = 0.0288779
    hgcroc.timeDnSlope = -23.3814
    hgcroc.timePeak    = -61.5518

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
        # energy [MeV] ( 1 MIP / energy per MIP [MeV] ) ( nPE / 1 MIP) * 4 times = PE  
        #   energy [MeV] ( 1 MIP / energy per MIP [MeV] ) ( voltage per MIP [mV] / 1 MIP ) = voltage [mV]
        #   this leads to ~ 470 mV/MeV or ~6.8 MeV maximum hit (if 320 fC is max ADC range)
        self.MeV = (1./mipSiEnergy)*nPEPerMIP*4
        print('MeV ',self.MeV)

        import time
        self.randomSeed = int(time.time())

        # hcal hexagon geometry parameters
        # used for putting noise into empty channels
        self.nHcalLayers      = 34
        self.nModulesPerLayer = 7
        self.nCellsPerModule  = 397
