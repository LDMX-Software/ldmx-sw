"""ConditionsProvider for TrigScintGeometry"""

from LDMX.DetDescr.trigscint_geometry import TrigScintGeometry
from LDMX.Framework import (
    ConditionsObjectProvider,
    conditions_object_provider,
    field,
)


@conditions_object_provider(
    "TrigScintGeometryProvider", "trigscint::TrigScintGeometryProvider", "TrigScint"
)
class TrigScintGeometryProvider(ConditionsObjectProvider):
    """Provides access to the trigger-scintillator bar geometry.

    The field name (`trig_scint_geometry`) must match the conditions object
    name / param-subtree the C++ provider looks up.
    """

    __instance = None
    trig_scint_geometry: TrigScintGeometry = field(default_factory=TrigScintGeometry)
    # Explicit detector name (real data whose RunHeader carries no detector);
    # empty => fall back to the RunHeader (sim).
    detector: str = ""

    def get_instance():
        if TrigScintGeometryProvider.__instance is None:
            TrigScintGeometryProvider()
        return TrigScintGeometryProvider.__instance

    def set_detector(self, det_name):
        """Set the detector name used to select the geometry parameters."""
        self.detector = det_name

    def __post_init__(self):
        if TrigScintGeometryProvider.__instance is not None:
            raise Exception(
                "TrigScintGeometryProvider is a singleton class and should only "
                "be retrieved using get_instance()"
            )
        else:
            TrigScintGeometryProvider.__instance = self


# create the global instance so the condition is registered with the process
TrigScintGeometryProvider.get_instance()
