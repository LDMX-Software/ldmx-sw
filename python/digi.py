"""Package to configure the ECal digitization pipeline

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

def EcalHgcrocEmulator() :
    """Get an HGCROC emulator and configure for the ECal specifically

    This sets the pulse shape parameters to the ones from a fit
    to a test readout of an ECal module and then thresholds to the
    default construction using 37k electrons as the number of
    electrons per MIP.
    """

    from LDMX.Tools import HgcrocEmulator
    hgcroc = HgcrocEmulator.HgcrocEmulator()

    # readout capacitance of chip is ~20pF
    hgcroc.readoutPadCapacitance = 20. #pF

    # set defaults with 37k electrons per MIP
    hgcroc.setThresholdDefaults( nElectronsPerMIP )

    # set pulse shape parameters
    hgcroc.rateUpSlope =  -0.345
    hgcroc.timeUpSlope = 70.6547
    hgcroc.rateDnSlope = 0.140068
    hgcroc.timeDnSlope = 87.7649
    hgcroc.timePeak    = 77.732

    return hgcroc

class EcalDigiProducer(Producer) :
    """Configuration for EcalDigiProducer

    Attributes
    ----------
    hgcroc : HgcrocEmulator
        Configuration for the chip emulator
    MeV : float
        Conversion between energy [MeV] and voltage [mV]
    nEcalLayers : int
        Number of Si layer in ECal, needed to generate noise ID
    nModulesPerLayer : int
        Number of modules in each layer, needed to generate noise ID
    nCellsPerModule : int
        Number of cells in each module, needed to generate noise ID
    """

    def __init__(self, instance_name = 'ecalDigis') :
        super().__init__(instance_name , 'ldmx::EcalDigiProducer','Ecal')

        self.hgcroc = EcalHgcrocEmulator()

        #Energy -> Volts converstion
        #   energy [MeV] ( 1 MIP / energy [MeV] ) ( voltage [mV] / 1 MIP ) = voltage [mV]
        #   this leads to ~ 470 mV/MeV or ~6.8 MeV maximum hit (if 320 fC is max ADC range)
        self.MeV = (1./mipSiEnergy)*self.hgcroc.calculateVoltage( nElectronsPerMIP )

        # input and output collection name parameters
        self.inputCollName = 'EcalSimHits'
        self.inputPassName = ''
        self.digiCollName = 'EcalDigis'


class EcalRecProducer(Producer) :
    """Configuration for the EcalRecProducer

    The layer weights and second order energy correction
    change when the ECal geometry changes, so we have setup
    various options for the different possible ECal geometries
    and their associated layer weights.

    Attributes
    ----------
    MeV_per_mV : float
        Conversion from voltage [mV] to energy [MeV]
    mip_si_energy : float
        Copied from module-wide mipSiEnergy [MeV]
    clock_cycle : float
        Time for one DAQ clock cycle to pass [ns]
    digiCollName : str
        Name of digi collection
    digiPassName : str
        Name of digi pass
    secondOrderEnergyCorrection : float
        Correction to weighted energy
    layerWeights : list of floats
        Weighting factors depending on layer index
    """

    def __init__(self, instance_name = 'ecalRecon') : 
        super().__init__(instance_name , 'ldmx::EcalRecProducer','Ecal')

        self.mip_si_energy = mipSiEnergy #MeV / MIP
        self.charge_per_mip = nElectronsPerMIP * 0.162/1000 #fC / MIP
        self.clock_cycle = 25. #ns - needs to match the setting on the chip

        self.digiCollName = 'EcalDigis'
        self.digiPassName = ''
        self.simHitCollName = 'EcalSimHits'
        self.simHitPassName = ''
        self.recHitCollName = 'EcalRecHits'
        
        # geometry dependent settings
        # use helper functions to set these
        self.secondOrderEnergyCorrection = 1.
        self.layerWeights = [ ]
        self.v12()

    def v2(self) :
        """These layerWeights and energy correction were calculated at least before v3 geometry.

        The second order energy correction is determined by comparing the mean of 1M single 4GeV
        electron events with 4GeV.
        """

        self.secondOrderEnergyCorrection = 0.948;
        self.layerWeights = [
            1.641, 3.526, 5.184, 6.841,
            8.222, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775,
            8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 12.642, 16.51,
            16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 8.45
            ]

    def v9(self) :
        """These layerWeights and energy correction were calculated for the v9 geometry.

        The second order energy correction is determined by comparing the mean of 1M single 4GeV
        electron events with 4GeV.
        """

        self.secondOrderEnergyCorrection = 4000. / 4012.;
        self.layerWeights = [
            1.019, 1.707, 3.381, 5.022, 6.679, 8.060, 8.613, 8.613, 8.613, 8.613, 8.613,
            8.613, 8.613, 8.613, 8.613, 8.613, 8.613, 8.613, 8.613, 8.613, 8.613, 8.613,
            8.613, 12.480, 16.347, 16.347, 16.347, 16.347, 16.347, 16.347, 16.347, 16.347,
            16.347, 8.334
            ]

    def v12(self) :
        """These layerWeights and energy correction were calculated for the v12 geometry.

        The second order energy correction is determined by comparing the mean of 1M single 4GeV
        electron events with 4GeV.
        """

        self.secondOrderEnergyCorrection = 4000. / 4007.;
        self.layerWeights = [
            1.675, 2.724, 4.398, 6.039, 7.696, 9.077, 9.630, 9.630, 9.630, 9.630, 9.630,
            9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630,
            9.630, 13.497, 17.364, 17.364, 17.364, 17.364, 17.364, 17.364, 17.364, 17.364,
            17.364, 8.990
            ]

