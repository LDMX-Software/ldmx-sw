"""Configuration for HGCROC Emulator"""

class HgcrocEmulator() :
    """Configuration for HGCROC Emulator

    Parameters
    ----------
    TBD
    """

    def __init__(self) :

        self.pedestal = 50. #ADC counts - baseline factor to subtract off of readout
        self.clockCycle = 25.0 #ns
        self.timingJitter = self.clockCycle / 100. #ns - pretty arbitrarily chosen
        self.readoutPadCapacitance = 0.1 #pF <- derived from hardware geometry
        self.maxADCRange = 320. #fC <- setting of HGCROC
        self.nADCs = 10 #
        self.iSOI  = 0 
        self.nElectronsPerMIP = 37000.0 #e-h pairs created per MIP <- derived from 0.5mm thick Si
        self.mipSiEnergy = 0.130 #MeV - corresponds to ~3.5 eV per e-h pair <- derived from 0.5mm thick Si

        self.mVperMIP = self.calculateVoltage( self.nElectronsPerMIP )

        #Energy -> Volts converstion
        #   energy [MeV] ( 1 MIP / energy per MIP [MeV] ) ( voltage per MIP [mV] / 1 MIP ) = voltage [mV]
        #   this leads to ~ 470 mV/MeV or ~6.8 MeV maximum hit (if 320 fC is max ADC range)
        self.MeV = (1./self.mipSiEnergy)*self.mVperMIP

        #Voltage -> ADC Counts conversion
        # voltage [mV] / gain = ADC Counts
        #
        # gain = maximum ADC range [fC] ( 1 / readout pad capacitance in pF ) ( 1 / 2^10 ADC Counts ) = mV / ADC counts
        self.gain = self.maxADCRange/self.readoutPadCapacitance/1024 # mV / ADC

        # ecal hexagon geometry parameters
        # used for putting noise into empty channels
        self.nEcalLayers      = 34
        self.nModulesPerLayer = 7
        self.nCellsPerModule  = 397
        self.ecal = True

        self.noiseRMS = 5. #mV - useless default

        self.readoutThreshold = self.gain*self.pedestal + 4.*self.noiseRMS #mV readout threshold is 4sigma higher than noise average
        self.toaThreshold     = self.gain*self.pedestal + self.calculateVoltage( 5.*self.nElectronsPerMIP  ) #mV TOA Threshold is 5 MIPs
        self.totThreshold     = self.gain*self.pedestal + self.calculateVoltage( 50.*self.nElectronsPerMIP ) #mV TOT Threshold is 50 MIPs

    def calculateVoltage(self, electrons) :
        """Calculate the voltage signal [mV] of the input number of electrons

        Uses the charge of 1000 electrons in fC and the capacitance of the readout pads.

        electrons ( 0.162 fC / 1000 electrons ) ( 1 / capacitance in pF ) = voltage [mV]

        Parameters
        ----------
        electrons : int
            Number of electrons (or e-h pairs) produced
        """

        return electrons*(0.162/1000.)*(1./self.readoutPadCapacitance)

