"""Package to help configure the ECal digitization pipeline

All classes are derived versions of LDMX.Framework.ldmxcfg.Producer
with helpful member functions.
"""

from LDMX.Framework import ldmxcfg

class EcalDigiProducer(ldmxcfg.Producer) :
    """Configuration for EcalDigiProducer

    Warnings
    --------
    - Multiple samples per digi has not been implemented yet. All of the information goes into the sample of interest (SOI).
    
    """

    def __init__(self, instance_name = 'ecalDigis') :
        super().__init__(instance_name , 'ldmx::EcalDigiProducer','Ecal')

        self.gain = 2000.
        self.pedestal =1100.
        self.noiseIntercept = 700. 
        self.noiseSlope = 25.
        self.padCapacitance = 0.1 
        self.readoutThreshold = 4. 
        self.makeConfigHists = False 
        self.nADCs = 10
        self.iSOI  = 0

        import time
        self.randomSeed = int(time.time())

class EcalRecProducer(ldmxcfg.Producer) :
    """Configuration for the EcalRecProducer

    The layer weights and second order energy correction
    change when the ECal geometry changes, so we have setup
    various options for the different possible ECal geometries
    and their associated layer weights.
    """

    def __init__(self, instance_name = 'ecalRecon') : 
        super().__init__(instance_name , 'ldmx::EcalRecProducer','Ecal')

        self.digiCollName = 'EcalDigis'
        self.digiPassName = ''
        self.secondOrderEnergyCorrection = 1.
        self.layerWeights = [ ]

        from LDMX.Ecal import EcalGeometry
        EcalGeometry.EcalGeometryProvider.getInstance()

        self.v12() #use v12 geometry by default

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
        """These layerWeights and energy correction were calculated at least before v3 geometry.

        The second order energy correction is determined by comparing the mean of 1M single 4GeV
        electron events with 4GeV.
        """

        self.secondOrderEnergyCorrection = 4000./4010.;
        self.layerWeights = [
            1.675, 2.724, 4.398, 6.039, 7.696, 9.077, 9.630, 9.630, 9.630, 9.630, 9.630,
            9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630,
            9.630, 13.497, 17.364, 17.364, 17.364, 17.364, 17.364, 17.364, 17.364, 17.364,
            17.364, 8.990
            ]

