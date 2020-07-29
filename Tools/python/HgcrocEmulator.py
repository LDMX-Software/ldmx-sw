"""Configuration for HGCROC Emulator"""

class HgcrocEmulator() :
    """Configuration for HGCROC Emulator

    Attributes
    ----------
    pedestal : float
    clockCycle : float
    timingJitter : float
    readoutPadCapacitance : float
    maxADCRange : float
    nADCs : float
    iSOI : float
    nElectronsPerMIP : float
    mipSiEnergy : float
    mVperMIP : float
    MeV : float
    gain : float
    nEcalLayers : int
    nModulesPerLayer : int
    nCellsPerModule : int
    ecal : bool
    noiseRMS : float
    readoutThreshold : float
    toaThreshold : float
    totThreshold : float
    """

    def __init__(self) :

        #######################################################################
        # Settings of the chip
        self.pedestal = 50. #ADC counts - baseline factor to subtract off of readout
        self.clockCycle = 25.0 #ns
        self.timingJitter = self.clockCycle / 100. #ns - pretty arbitrarily chosen
        self.readoutPadCapacitance = 0.1 #pF <- derived from hardware geometry
        self.maxADCRange = 320. #fC <- setting of HGCROC
        self.nADCs = 10 #
        self.iSOI  = 0 

        #######################################################################
        # Physical Constants for Detector Materials
        self.nElectronsPerMIP = 37000.0 #e-h pairs created per MIP <- derived from 0.5mm thick Si
        self.mipEnergy = 0.130 #MeV - corresponds to ~3.5 eV per e-h pair <- derived from 0.5mm thick Si

        #Energy -> Volts converstion
        #   energy [MeV] ( 1 MIP / energy per MIP [MeV] ) ( voltage per MIP [mV] / 1 MIP ) = voltage [mV]
        #   this leads to ~ 470 mV/MeV or ~6.8 MeV maximum hit (if 320 fC is max ADC range)
        self.mVperMeV = (1./self.mipEnergy)*self.calculateVoltage( self.nElectronsPerMIP )

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

        self.noiseRMS         = 5. #mV - useless default
        self.readoutThreshold = 0. #mV - useless default
        self.toaThreshold     = 0. #mV - useless default
        self.totThreshold     = 0. #mV - useless default

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
    
    def recalculateThresholds(self , readout = None , toa = None , tot = None ) :
        """Reset the different thresholds of the chip

        Optionally, you can provide specific threshold settings if you want to as well.

        The default calculation for the different thresholds if the following:
        - readout is 4 times the rms noise above the pedestal 
        - toa is 5 MIPs above the pedestal
        - tot is 50 MIPs above the pedestal

        These calculations depend on the following parameters;
        the user should call this function _after_ setting these parameters.
        - noiseRMS
        - pedestal
        - gain
        - readoutPadCapacitance

        Parameters
        ----------
        readout : float, optional
            Readout threshold [mV]
        toa : float, optional
            TOA threshold [mV]
        tot : float, optional
            TOT threshold [mV]
        """

        if readout is None :
            self.readoutThreshold = self.gain*self.pedestal + 4*self.noiseRMS
        else :
            self.readoutThreshold = readout

        if toa is None :
            self.toaThreshold = self.gain*self.pedestal + self.calculateVoltage( 5.*self.nElectronsPerMIP )
        else :
            self.toaThreshold = toa

        if tot is None :
            self.totThreshold = self.gain*self.pedestal + self.calculateVoltage( 50.*self.nElectronsPerMIP )
        else :
            self.totThreshold = tot
