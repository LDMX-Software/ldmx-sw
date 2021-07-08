"""Package to configure the ECal digitization pipeline

All classes are derived versions of LDMX.Framework.ldmxcfg.Producer
with helpful member functions.

Two module-wide parameters are defined.

Attributes
----------
n_kelectrons_per_mip : float
    Number of thousand e-h pairs created for each MIP in 0.5mm thick Si
mip_si_energy : float
    Energy [MeV] of a single MIP on average in 0.5mm thick Si
"""

from LDMX.Framework.ldmxcfg import Producer

n_kelectrons_per_mip = 37.0 #thousand e-h pairs created per MIP <- derived from 0.5mm thick Si
charge_per_mip = n_kelectrons_per_mip*0.1602 #fC
mip_si_energy = 0.130 #MeV - corresponds to ~3.5 eV per e-h pair <- derived from 0.5mm thick Si

def EcalHgcrocEmulator() :
    """Get an HGCROC emulator and configure for the ECal specifically

    This sets the pulse shape parameters to the ones from a fit
    to a test readout of an ECal module and then thresholds to the
    default construction using 37k electrons as the number of
    electrons per MIP.

    Noise RMS is calculated using the average readout pad capacitance (20pF),
    noise at zero capacitance (700 electrons), and noise increase
    per capacitance increase (25 electrons per pF).
    """

    from LDMX.Tools import HgcrocEmulator
    hgcroc = HgcrocEmulator.HgcrocEmulator()

    # set pulse shape parameters
    hgcroc.rateUpSlope =  -0.345
    hgcroc.timeUpSlope = 70.6547
    hgcroc.rateDnSlope = 0.140068
    hgcroc.timeDnSlope = 87.7649
    hgcroc.timePeak    = 77.732

    hgcroc.noiseRMS     = (700. + 25.*20.)*(0.1602/1000.)*(1./20.) #mV
    hgcroc.nADCs        = 10 
    hgcroc.iSOI         = 2

    return hgcroc

class EcalDigiProducer(Producer) :
    """Configuration for EcalDigiProducer

    Attributes
    ----------
    hgcroc : HgcrocEmulator
        Configuration for the chip emulator
    MeV : float
        Conversion between energy [MeV] and voltage [mV]
    avgReadoutThreshold : float
        Average readout threshold for all channels [mV], for noise emulation
    avgPedestal : float
        Average pedestal for all channels [mV], for noise emulation
    zero_suppresion : bool
        Should we suppress pure noise "hits" below readout threshold?
    inputCollName : str
        Name of simulated ecal hits to digitize
    inputPassName : str
        Name of pass to digitize
    digiCollName : str
        Output name of digis put into event bus
    """

    def __init__(self, instance_name = 'ecalDigis') :
        super().__init__(instance_name , 'ecal::EcalDigiProducer','Ecal')

        self.hgcroc = EcalHgcrocEmulator()

        #Energy -> Volts converstion
        #   energy [MeV] (thousand electrons per MIP) (charge per thousand electrons fC) 
        #        (avg pad capacitance pF) ( 1 MIP / energy [MeV] ) = voltage [mV]
        #   this leads to ~ 470 mV/MeV or ~6.8 MeV maximum hit (if 320 fC is max ADC range)
        self.MeV = charge_per_mip/20./mip_si_energy

        # these averages are for configuring the noise generator
        #   _only_ and are not meant to be propated to a chip-by-chip basis
        avgGain = 0.3125/20.
        self.avgReadoutThreshold = 53.*avgGain
        self.avgPedestal = 50.*avgGain

        # Should we suppress noise "hits" below readout threshold?
        self.zero_suppression = True

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
        Copied from module-wide mip_si_energy [MeV]
    clock_cycle : float
        Time for one DAQ clock cycle to pass [ns]
    digiCollName : str
        Name of digi collection
    digiPassName : str
        Name of digi pass
    simHitCollName : str
        Name of sim collection to check for pure noise hits
    simHitPassName : str
        Name of sim pass
    recHitCollName : str
        Name of output collection 
    secondOrderEnergyCorrection : float
        Correction to weighted energy
    layerWeights : list of floats
        Weighting factors depending on layer index
    """

    def __init__(self, instance_name = 'ecalRecon') : 
        super().__init__(instance_name , 'ecal::EcalRecProducer','Ecal')

        self.mip_si_energy = mip_si_energy #MeV / MIP
        self.charge_per_mip = charge_per_mip #fC / MIP
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

