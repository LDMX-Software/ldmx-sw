"""Configuration for the GENIE electronuclear physics process"""

from LDMX.Framework import parameter_set


@parameter_set
class GenieNuclearPhysics:
    """Configuration for the GENIE-based electronuclear process.

    When enabled, this replaces the built-in Geant4 electronNuclear process
    with a GENIE-based implementation that fires as the electron traverses
    material, using the electron's actual energy after upstream energy loss.

    Attributes
    ----------
    enable : bool
        Enable the GENIE electronuclear process (default: False)
    targets : list[int]
        GENIE target codes in 10LZZZAAAI format
    abundances : list[float]
        Relative abundances for each target (will be normalized)
    tune : str
        Name of the GENIE tune to use
    spline_file : str
        Path to the GENIE cross-section spline XML file
    message_threshold_file : str
        Path to the GENIE message threshold configuration
    only_one_per_event : bool
        Only allow one electronuclear interaction per event (default: True)
    """

    enable: bool = False
    targets: list[int] = []
    abundances: list[float] = []
    tune: str = "default"
    spline_file: str = ""
    message_threshold_file: str = (
        "/usr/local/GENIE/Generator/config/Messenger.xml"
    )
    only_one_per_event: bool = True
