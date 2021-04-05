"""Configuration for HGCROC Emulator"""

class HgcrocEmulator() :
    """Configuration for HGCROC Emulator

    The parameters in this configuration class
    are **only** for system-wide settings. The
    parameters that can change on a chip-by-chip
    basis are handled using a conditions table
    given to the C++ emulator in the subsystem producer.

    Attributes
    ----------
    clockCycle : float
        Cycle of chip clock [ns]
    timingJitter : float
        Uncertainty in chip clock [ns]
    nADCs : int
        Number of voltage samples to measure for one DIGI
    iSOI : int
        Index for sample of interest within multi-sample DIGI
    noiseRMS : float
        Average noise within chip [mV]
        Calculated using the average readout pad capacitance (20pF),
        noise at zero capacitance (700 electrons), and noise increase
        per capacitance increase (25 electrons per pF).
    noise : bool
        False to turn off all noise generation
    rateUpSlope : float
        Rate of up-wards slope in pulse shape fit
    timeUpSlope : float
        Time of front edge in pulse shape fit
    rateDnSlope : float
        Rate of down-wards slope in pulse shape fit
    timeDnSlope : float
        Time of back edge in pulse shape fit
    timePeak : float
        Time of beak in pulse shape fit
    """

    def __init__(self) :

        self.rateUpSlope  = -0.345
        self.timeUpSlope  = 70.6547
        self.rateDnSlope  = 0.140068
        self.timeDnSlope  = 87.7649
        self.timePeak     = 77.732
        self.clockCycle   = 25.0 #ns
        self.timingJitter = self.clockCycle / 100. #ns - pretty arbitrarily chosen
        self.noiseRMS     = (700. + 25.*20.)*(0.162/1000.)*(1./20.) #mV
        self.nADCs        = 10 
        self.iSOI         = 2

        # turn on or off noise
        #   NOT DOCUMENTED - only meant for testing purposes
        self.noise = True

