"""Package to help configure the simulation

Defines a derived class from ldmxcfg.Producer
with several helpful member functions.
"""

from LDMX.Framework import Processor, field, parameter_set, processor

from .bias_operators import XsecBiasingOperator
from .dark_brem import DarkBrem
from .fcp_physics import FCPPhysics
from .generators import PrimaryGenerator
from .genie_physics import GenieNuclearPhysics
from .kaon_physics import KaonPhysics
from .photonuclear_models import BertiniModel, PhotoNuclearModel
from .sensitive_detectors import SensitiveDetector
from .user_actions import UserAction


@parameter_set
class _EventToReSim:
    """A class to hold the information identifying a specific event we wish to
    re-simulate

    This is an internal class used by simulator.resimulate in order to pass the event
    identification to the ReSimulator class

    Attributes
    ----------
    event: int
        event number to re-sim, required
    run: int
        run number of the event to re-sim, -1 if we don't care about the run
    """

    event: int
    run: int = -1


@processor("simcore::ReSimulator", "SimCore")
class ReSimulator(Processor):
    """Configuration for resimulating events from an existing file.

    Created via Simulator.resimulate() to carry the same configuration
    as the original simulation plus the re-simulation-specific parameters.

    Attributes
    ----------
    resimulate_all_events : bool
        If True, every event in the input file is re-simulated.
    care_about_run : bool
        If True, match both run *and* event number when selecting events.
    events_to_resimulate : list of _EventToReSim
        Specific events (and optionally runs) to re-simulate when
        resimulate_all_events is False.
    """

    generators: list[PrimaryGenerator] = []
    detector: str = ""
    sensitive_detectors: list[SensitiveDetector] = []
    description: str = ""
    scoring_planes: str = ""
    time_shift_primaries: bool = True
    pre_init_commands: list[str] = []
    post_init_commands: list[str] = []
    actions: list[UserAction] = []
    biasing_operators: list[XsecBiasingOperator] = []
    logging_prefix: str = "GEANT4"
    root_primary_gen_use_seed: bool = False
    validate_detector: bool = False
    verbosity: int = 0
    dark_brem: DarkBrem = field(default_factory=DarkBrem)
    photonuclear_model: PhotoNuclearModel = field(default_factory=BertiniModel)
    kaon_parameters: KaonPhysics = field(default_factory=KaonPhysics)
    fcp_physics: FCPPhysics = field(default_factory=FCPPhysics)
    genie_nuclear: GenieNuclearPhysics = field(default_factory=GenieNuclearPhysics)
    resimulate_all_events: bool = True
    care_about_run: bool = False
    events_to_resimulate: list[_EventToReSim] = []


@processor("simcore::Simulator", "SimCore")
class Simulator(Processor):
    """A instance of the simulation configuration

    This class is derived from ldmxcfg.Producer and is mainly
    focused on providing helper functions that can be used instead
    of accessing the parameters member directly.

    The parameters that are lists are initialized as empty lists
    so that we can append to them later.

    Parameters
    ----------
    instance_name : str
        Name of this instance of the Simulator

    Attributes
    ----------
    generators : list of PrimaryGenerator
        Generators to use to make primaries
    detector : str
        Full path to detector description gdml (suggested to use set_detector)
    validate_detector : bool, optional
        Should we have Geant4 validate that the gdml is correctly formatted?
    sensitive_detectors : list[SensitiveDetector]
        List of sensitive detectors to load
    description : str
        Describe this run in a human-readable way
    scoringPlanes : str, optional
        Full path to the scoring planes gdml (suggested to use set_detector)
    time_shift_primaries : bool
        Should we shift the times of primaries so that z=0mm corresponds to t=0ns?
    pre_init_commands : list of str, optional
        Geant4 commands to run before the run is initialized
    post_init_commands : list of str, optional
        Geant4 commands to run after run is initialized (but before run starts)
    actions : list of UserAction, optional
        Special User-defined actions to take during the simulation
    biasing_operators : list of XsecBiasingOperators, optional
        Operators for biasing specific particles to undergo specific processes
    dark_brem : DarkBrem
        Configuration options for dark brem process
    logging_prefix : str, optional
        Prefix to prepend any Geant4 logging files
    rootPrimaryGenUseSeed : bool, optional
        Use the seed stored in the EventHeader for random generation
    verbosity : int, optional
        Verbosity level to print
    """

    generators: list[PrimaryGenerator] = []
    detector: str = ""
    sensitive_detectors: list[SensitiveDetector] = []
    description: str = ""
    scoring_planes: str = ""
    time_shift_primaries: bool = True
    pre_init_commands: list[str] = []
    post_init_commands: list[str] = []
    actions: list[UserAction] = []
    biasing_operators: list[XsecBiasingOperator] = []
    logging_prefix: str = "GEANT4"
    root_primary_gen_use_seed: bool = False
    validate_detector: bool = False
    verbosity: int = 0
    dark_brem: DarkBrem = field(default_factory=DarkBrem)
    photonuclear_model: PhotoNuclearModel = field(default_factory=BertiniModel)
    kaon_parameters: KaonPhysics = field(default_factory=KaonPhysics)
    fcp_physics: FCPPhysics = field(default_factory=FCPPhysics)
    genie_nuclear: GenieNuclearPhysics = field(default_factory=GenieNuclearPhysics)

    def set_detector(
        self,
        det_name,
        include_scoring_planes_others=False,
        include_scoring_planes_minimal=False,
    ):
        """Set the detector description with the option to include the scoring planes

        Parameters
        ----------
        det_name : str
            name of a detector in the Detectors module
        include_scoring_planes_minimal : bool
            True if you want to import only target and ecal scoring planes
        include_scoring_planes_others : bool
            True if you want to import the remaining other scoring planes

        See Also
        --------
        LDMX.Detectors.makePath for definitions of the path making functions.
        sensitive_detectors for configuring the SDs
        """

        from LDMX.Detectors import make_path

        from . import sensitive_detectors as sds

        self.detector = make_path.make_detector_path(det_name)
        if "v12" in det_name:
            trigscint = [
                sds.TrigScintSD.up(),
                sds.TrigScintSD.tag(),
                sds.TrigScintSD.down(),
            ]
        elif "hcal-prototype" in det_name:
            trigscint = [sds.TrigScintSD.testbeam()]
        else:
            trigscint = [
                sds.TrigScintSD.pad1(),
                sds.TrigScintSD.pad2(),
                sds.TrigScintSD.pad3(),
            ]
        self.sensitive_detectors = [
            sds.TrackerSD.tagger(),
            sds.TrackerSD.recoil(),
            sds.HcalSD(),
            sds.EcalSD(),
            sds.TrigScintSD.target(),
            *trigscint,
        ]
        if include_scoring_planes_minimal:
            self.scoring_planes = make_path.make_scoring_planes_path(det_name)
            self.sensitive_detectors.extend(
                [
                    sds.ScoringPlaneSD.target(),
                    sds.ScoringPlaneSD.ecal(),
                ]
            )
        if include_scoring_planes_others:
            self.scoring_planes = make_path.make_scoring_planes_path(det_name)
            self.sensitive_detectors.extend(
                [
                    sds.ScoringPlaneSD.hcal(),
                    sds.ScoringPlaneSD.trigscint(),
                    sds.ScoringPlaneSD.tracker(),
                    sds.ScoringPlaneSD.magnet(),
                ]
            )

    def resimulate(self, which_events=None, which_runs=None):
        """Create a resimulator based on the simulator configuration.

        This is intended to ensure that a resimulator has the same configuration
        as the simulation that was used to generate the input file. If you
        require any changes to the simulation configuration, such as loading a
        modified geometry, you can make those changes after creating the
        resiulator.


        Parameters
        ----------
        which_events : list of event numbers, optional
            Which events from the input files to resimulate. If None,
            resimulate all events.

            Events that are not present in any of the input files will be
            ignored.

            For multiple input files, if an event number is present within more
            than one input file all versions will be resimulated unless the which_runs
            parameters is used to distinguish them.

        which_runs : list of run numbers, optional
            Which runs from the input files to resimulate, ignored if no
            events are listed. Runs not present in the input files will be
            ignored.

            If not provided, all runs will be resimulated (i.e. the run number
            check is ignored). If only one value is provided, all events requested
            are also required to have that value for their run number to be resimulated.
            If more than one value is provided, it must be the same length as the
            number of events requested so that the event/run number pair can be
            required.

        """
        resimulator = ReSimulator(self.instance_name)
        # Copy all Simulator fields into the ReSimulator
        _skip = {"class_name", "module_name", "instance_name", "histograms"}
        for fname in self.__dataclass_fields__:
            if fname not in _skip:
                resimulator.__dict__[fname] = self.__dict__[fname]

        if which_events is None:
            resimulator.resimulate_all_events = True
            resimulator.care_about_run = False
            resimulator.events_to_resimulate = []
        elif isinstance(which_events, list):
            resimulator.resimulate_all_events = False
            if len(which_events) == 0:
                raise ValueError(
                    "which_events must contain at least one element if provided"
                )
            if which_runs is None:
                resimulator.care_about_run = False
                resimulator.events_to_resimulate = [
                    _EventToReSim(event) for event in which_events
                ]
            elif isinstance(which_runs, int):
                resimulator.care_about_run = True
                resimulator.events_to_resimulate = [
                    _EventToReSim(event, which_runs) for event in which_events
                ]
            elif isinstance(which_runs, list):
                if len(which_runs) == 0:
                    raise ValueError(
                        "which_runs must have at least one value if provided as a list"
                    )
                if len(which_runs) != len(which_events):
                    raise ValueError(
                        "which_runs must have the same number of entries as "
                        "which_events if more than one run is provided"
                    )
                resimulator.care_about_run = True
                resimulator.events_to_resimulate = [
                    _EventToReSim(event, run)
                    for event, run in zip(which_events, which_runs, strict=True)
                ]
            else:
                raise ValueError(
                    "which_runs must be an int or a list of ints if provided"
                )
        else:
            raise ValueError("which_events must be a list if provided")
        return resimulator
