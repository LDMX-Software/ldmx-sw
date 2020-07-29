"""Package to configure the ECal digitization pipeline

All classes are derived versions of LDMX.Framework.ldmxcfg.Producer
with helpful member functions.
"""

from LDMX.Framework.ldmxcfg import Producer

class EcalDigiProducer(Producer) :
    """Configuration for EcalDigiProducer

    Warnings
    --------
    - Multiple samples per digi has not been implemented yet. All of the information goes into the sample of interest (SOI).
    
    """

    def __init__(self, instance_name = 'ecalDigis') :
        super().__init__(instance_name , 'ldmx::EcalDigiProducer','Ecal')

        self.nElectronsPerMIP = 37000.0 #e-h pairs created per MIP <- derived from 0.5mm thick Si
        self.mipSiEnergy = 0.130 #MeV - corresponds to ~3.5 eV per e-h pair <- derived from 0.5mm thick Si

        from LDMX.Tools import HgcrocEmulator
        self.hgcroc = HgcrocEmulator.HgcrocEmulator()

        self.hgcroc.setThresholdDefaults( self.nElectronsPerMIP )

        #Energy -> Volts converstion
        #   energy [MeV] ( 1 MIP / energy per MIP [MeV] ) ( voltage per MIP [mV] / 1 MIP ) = voltage [mV]
        #   this leads to ~ 470 mV/MeV or ~6.8 MeV maximum hit (if 320 fC is max ADC range)
        self.MeV = (1./self.mipSiEnergy)*self.hgcroc.calculateVoltage( self.nElectronsPerMIP )

        # ecal hexagon geometry parameters
        # used for putting noise into empty channels
        self.nEcalLayers      = 34
        self.nModulesPerLayer = 7
        self.nCellsPerModule  = 397

class EcalRecProducer(Producer) :
    """Configuration for the EcalRecProducer

    The layer weights and second order energy correction
    change when the ECal geometry changes, so we have setup
    various options for the different possible ECal geometries
    and their associated layer weights.
    """

    def __init__(self, instance_name = 'ecalRecon') : 
        super().__init__(instance_name , 'ldmx::EcalRecProducer','Ecal')

        self.pedestal = 50. #ADC counts - baseline factor to subtract off of readout
        self.clockCycle = 25.0 #ns
        self.readoutPadCapacitance = 0.1 #pF <- derived from hardware geometry
        self.maxADCRange = 320. #fC <- setting of HGCROC
        self.nElectronsPerMIP = 37000.0 #e-h pairs created per MIP <- derived from 0.5mm thick Si
        self.mipSiEnergy = 0.130 #MeV - corresponds to ~3.5 eV per e-h pair <- derived from 0.5mm thick Si

        #Volts -> Energy conversion
        #   voltage [mV] ( readout pad capacitance [pF] ) ( 1000 electrons / 0.162 fC ) ( 1 MIP / electrons ) ( energy / MIP ) = energy [MeV]
        self.mV  = self.readoutPadCapacitance*(1000./0.162)*(1./self.nElectronsPerMIP)*(self.mipSiEnergy)

        #ADC Counts -> Voltage conversion
        #
        # gain = maximum ADC range [fC] ( 1 / readout pad capacitance in pF ) ( 1 / 2^10 ADC Counts ) = mV / ADC counts
        self.gain = self.maxADCRange/self.readoutPadCapacitance/1024 # mV / ADC

        self.digiCollName = 'EcalDigis'
        self.digiPassName = ''

        # geometry dependent settings
        # use helper functions to set these
        self.secondOrderEnergyCorrection = 1.
        self.layerWeights = [ ]

        from LDMX.DetDescr import EcalHexReadout
        self.hexReadout = EcalHexReadout.EcalHexReadout()

        self.v12() #use v12 geometry by default

    def v2(self) :
        """These layerWeights and energy correction were calculated at least before v3 geometry.

        The second order energy correction is determined by comparing the mean of 1M single 4GeV
        electron events with 4GeV.
        """

        self.hexReadout.v9()
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

        self.hexReadout.v9()
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

        self.hexReadout.v12()
        self.secondOrderEnergyCorrection = 4000./4010.;
        self.layerWeights = [
            1.675, 2.724, 4.398, 6.039, 7.696, 9.077, 9.630, 9.630, 9.630, 9.630, 9.630,
            9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630,
            9.630, 13.497, 17.364, 17.364, 17.364, 17.364, 17.364, 17.364, 17.364, 17.364,
            17.364, 8.990
            ]

