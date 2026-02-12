"""Configuration for HGCROC Emulator"""

class HgcrocEmulator :
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

        self.rate_up_slope  = -0.345
        self.time_up_slope  = 70.6547
        self.rate_dn_slope  = 0.140068
        self.time_dn_slope  = 87.7649
        self.time_peak     = 77.732
        self.clock_cycle   = 25.0 #ns
        self.timing_jitter = self.clock_cycle / 100. #ns - pretty arbitrarily chosen
        self.n_adcs        = 10
        self.i_soi         = 2

        # turn on or off noise
        #   NOT DOCUMENTED - only meant for testing purposes
        self.noise = True

