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
    targets : list[int], optional
        GENIE target codes in 10LZZZAAAI format.
        If empty (default), targets are auto-discovered from the Geant4
        geometry at runtime using isotope data from each G4Element found in
        the volume named by ``discover_volume``.
    abundances : list[float], optional
        Relative abundances for each target (will be normalized).
        Must match targets if targets are specified manually.
    discover_volume : str, optional
        Volume to auto-discover targets from when ``targets`` is empty.
        Uses the same naming convention as the ElectroNuclear bias operator
        (e.g. "target_region", "target", "ecal", "hcal", or a substring of a
        logical volume name). Discovery happens once, on the first step, and
        only builds GENIE drivers for elements in the matched volume(s) —
        avoiding the cost of initializing drivers for every traversed
        material. Required (non-empty) when ``targets`` is empty.
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
    discover_volume: str = ""
    tune: str = "default"
    spline_file: str = ""
    message_threshold_file: str = "/usr/local/GENIE/Generator/config/Messenger.xml"
    only_one_per_event: bool = True
