"""Configuration for HGCROC Emulator"""

from LDMX.Framework import parameter_set


@parameter_set
class HgcrocEmulator:
    """Configuration for HGCROC Emulator

    The parameters in this configuration class
    are **only** for system-wide settings. The
    parameters that can change on a chip-by-chip
    basis are handled using a conditions table
    given to the C++ emulator in the subsystem producer.

    Attributes
    ----------
    clock_cycle : float
        Cycle of chip clock [ns]
    timing_jitter : float
        Uncertainty in chip clock [ns]
    n_adcs : int
        Number of voltage samples to measure for one DIGI
    i_soi : int
        Index for sample of interest within multi-sample DIGI
    noise : bool
        False to turn off all noise generation
    rate_up_slope : float
        Rate of up-wards slope in pulse shape fit
    time_up_slope : float
        Time of front edge in pulse shape fit
    rate_dn_slope : float
        Rate of down-wards slope in pulse shape fit
    time_dn_slope : float
        Time of back edge in pulse shape fit
    time_peak : float
        Time of beak in pulse shape fit
    """

    rate_up_slope: float = -0.345
    time_up_slope: float = 70.6547
    rate_dn_slope: float = 0.140068
    time_dn_slope: float = 87.7649
    time_peak: float = 77.732
    clock_cycle: float = 25.0  # ns
    timing_jitter: float = 25.0 / 100.0  # ns - pretty arbitrarily chosen
    n_adcs: int = 10
    i_soi: int = 2
    noise: bool = True
