"""Configuration classes for sensitive detectors"""

from LDMX.Framework import field, parameter_set, _register


class SensitiveDetector:
    """base class for sensitive detectors

    here for type-checking the simulator parameters
    """
    pass


def sensitive_detector(class_name: str, module_name: str = 'SimCore_SDs'):
    """Configuration for a sensitive detector we want to load

    Parameters
    ----------
    instance_name : str
        Unique name for this particular instance of a PrimaryGenerator
    class_name : str
        Name of C++ class that this PrimaryGenerator should be
    module_name : str
        Name of C++ library that this primary generator is compiled into
    """
    return parameter_set(
        class_name = class_name,
        instance_name = class_name,
        module_name = module_name,
        post_init = lambda self: _register.library(self.module_name),
        required_base = SensitiveDetector
    )



@sensitive_detector("simcore::ScoringPlaneSD")
class ScoringPlaneSD(SensitiveDetector) :
    """Scoring plane SD

    Simply collecting tracker-hit equivalent for scoring planes that
    enclose different subsystems

    Parameters
    ----------
    subsystem : str
        Name of subsystem to store scoring plane hits for
        Names must match what is in gdml for sp_<subsystem>
    """

    subsystem: str
    collection_name: str = field(init = False)
    match_substr: str = field(init = False)

    def __post_init__(self) :
        self.instance_name = f'{self.subsystem}_sp'
        # we don't use the Python built-in str.capitalize since
        #  that function changes all characters after the first one to lowercase
        self.collection_name = f'{self.subsystem[0].upper()+self.subsystem[1:]}ScoringPlaneHits'
        self.match_substr = f'sp_{self.subsystem}' #depends on gdml

    def ecal() :
        return ScoringPlaneSD('ecal')

    def hcal() :
        return ScoringPlaneSD('hcal')

    def target() :
        return ScoringPlaneSD('target')

    def trigscint() :
        return ScoringPlaneSD('trigScint')

    def magnet() :
        return ScoringPlaneSD('magnet')

    def tracker() :
        sp = ScoringPlaneSD('tracker')
        sp.match_substr = 'sp_recoil'
        return sp


@sensitive_detector("simcore::TrackerSD")
class TrackerSD(SensitiveDetector):
    """SD for the recoil and tagging trackers

    Parameters
    ----------
    subsystem : str
        Recoil or Tagger
    subdet_id : int
        ID number for the subsystem
    """

    subsystem: str
    subdet_id: int
    collection_name: str = field(init=False)

    def __post_init__(self):
        self.instance_name = f'{self.subsystem}_TrackerSD'
        self.collection_name = f'{self.subsystem}SimHits'

    def tagger() :
        return TrackerSD('Tagger',1)

    def recoil() :
        return TrackerSD('Recoil',4)


@sensitive_detector("simcore::HcalSD")
class HcalSD(SensitiveDetector):
    """SD for the HCal

    Separate from the other calorimeters since it includes a Birks law
    estimate.

    Parameters
    ----------
    gdml_identifiers : list[str]

        A list of strings used to determine which volumes in the Hcal are
        considered sensitive. Any volume name containing at least one of these
        identifiers will have a sensitive detector attached.

        The current defaults match the mainline LDMX Hcal and
        prototype Hcal (scint_box) scintillator geometries.

    enable_hit_contribs : bool, optional
        Should the simulation save contributions to Hcal sim hits?
    compress_hit_contribs : bool, optional
        Should the simulation compress contributions to Hcal sim hits by PDG ID?
    """

    gdml_identifiers: list[str] = [
            'scintYVolume', 'scintXVolume', 'scintX_0Volume',
            'scintX_1Volume', 'scintX_2Volume', 'scintX_3Volume',
            'scintY_0Volume', 'scintY_1Volume', 'scintY_2Volume',
            'scintY_3Volume', 'scintZXVolume', 'scintZYVolume',
            'ScintBox', 'scint_box'
    ]
    enable_hit_contribs: bool = True
    compress_hit_contribs: bool = True
    max_origin_track_id: int = 6

    def __post_init__(self):
        self.instance_name = 'hcal_sd'


@sensitive_detector("simcore::EcalSD")
class EcalSD(SensitiveDetector):
    """SD for the ECal

    The two configurable parameters are inherited from a legacy method of
    merging simulated hit contribs. We have plans to update this hit merging
    in the future.

    Parameters
    ----------
    enable_hit_contribs : bool, optional
        Should the simulation save contributions to Ecal sim hits?
    compress_hit_contribs : bool, optional
        Should the simulation compress contributions to Ecal sim hits by PDG ID?
    """

    enable_hit_contribs: bool = True
    compress_hit_contribs: bool = True
    max_origin_track_id: int = 6

    def __post_init__(self):
        self.instance_name = 'ecal_sd'


@sensitive_detector("simcore::TrigScintSD")
class TrigScintSD(SensitiveDetector):
    """Trigger Scintillaotr Sensitive Detector

    used for both the trigger pad modules as well as collecting hits
    within the target itself

    Parameters
    ----------
    module : int
        ID number for the module we are collecting hits from
    name : str
        Short name to be used in building collection name
    vol : str
        Name of logical volume(s) that this SD should be attached to
        DEPENDS ON GDML
    use_birks_law : bool, optional
        Should the simulation use Birks law to estimate energy deposition?
        Defaults to False.
    birks_const_one : float, optional
        First Birks constant, defaults to 1.29e-2
    birks_const_two : float, optional
        Second Birks constant, defaults to 9.59e-6
    """

    module_id: int
    name: str
    volume_name: str
    use_birks_law: bool = False
    birks_const_one: float = 1.29e-2
    birks_const_two: float = 9.59e-6
    collection_name: str = field(init = False)


    def __post_init__(self):
        self.instance_name = f'trig_scint_{self.name}_sd'

        coll = self.name+'SimHits'
        if self.name != 'Target' :
            coll = 'Trigger'+coll

        self.collection_name = coll


    def testbeam() :
        return TrigScintSD(1, 'PadUp', 'TS_plastic_bar_volume')

    def up() :
        return TrigScintSD(3, 'PadUp', 'trigger_pad_up_bar_volume')

    def tag() :
        return TrigScintSD(1, 'PadTag', 'trigger_pad_tag_bar_volume')

    def down() :
        return TrigScintSD(2, 'PadDn', 'trigger_pad_dn_bar_volume')

    def pad3() :
        return TrigScintSD(3,'Pad3','trigger_pad3_bar_volume')

    def pad2() :
        return TrigScintSD(1,'Pad2','trigger_pad2_bar_volume')

    def pad1() :
        return TrigScintSD(2,'Pad1','trigger_pad1_bar_volume')

    def target() :
        """Target sensitive detector"""
        active_target = TrigScintSD(4,'Target','target')
        active_target.use_birks_law = True

        return active_target

