from LDMX.Framework import ConditionsObjectProvider, conditions_object_provider


@conditions_object_provider(
    "TrackersTrackingGeometry",
    "tracking::geo::TrackersTrackingGeometryProvider",
    "Tracking",
)
class TrackersTrackingGeometryProvider(ConditionsObjectProvider):
    """The provider of the tracking geometry

    This is a singleton-class and is created when this module is imported.
    Users can then modify its configuration by setting attributes and
    calling methods after calling get_instance(). For example, if you
    wanted to change the detector to v12 since that is what you used during
    the simulation, you would

        from LDMX.Tracking.geo import TrackersTrackingGeometryProvider as trackgeo
        trackgeo.get_instance().set_detector('ldmx-det-v12')

    The default detector is 'ldmx-det-v15-8gev'.
    """

    __instance = None

    detector: str = ""
    tracker_y_length: float = 480.0
    tracker_z_length: float = 240.0

    def get_instance():
        if TrackersTrackingGeometryProvider.__instance is None:
            TrackersTrackingGeometryProvider()

        return TrackersTrackingGeometryProvider.__instance

    def set_detector(self, det_name):
        """Set the detector GDML based on the detector name

        Parameters
        ----------
        det_name : str
            name of a detector in the Detectors module

        See Also
        --------
        LDMX.Detectors.makePath for definitions of the path making functions.
        """

        from LDMX.Detectors import make_path as mp

        print("Setting detector for tracking to " + det_name)
        self.detector = mp.make_detector_path(det_name)

    def __post_init__(self):
        if TrackersTrackingGeometryProvider.__instance is not None:
            raise Exception(
                "TrackersTrackingGeometryProvider is a singleton class and "
                "should only be retrieved using get_instance()"
            )
        else:
            self.set_detector("ldmx-det-v15-8gev")
            TrackersTrackingGeometryProvider.__instance = self


TrackersTrackingGeometryProvider.get_instance()


@conditions_object_provider(
    "GeometryContext", "tracking::geo::GeometryContextProvider", "Tracking"
)
class GeometryContextProvider(ConditionsObjectProvider):
    """Provider of the geometry context condition."""


geometry_context = GeometryContextProvider()


@conditions_object_provider(
    "MagneticFieldContext",
    "tracking::geo::MagneticFieldContextProvider",
    "Tracking",
)
class MagneticFieldContextProvider(ConditionsObjectProvider):
    """Provider of the magnetic field context condition."""


magfield_context = MagneticFieldContextProvider()


@conditions_object_provider(
    "CalibrationContext",
    "tracking::geo::CalibrationContextProvider",
    "Tracking",
)
class CalibrationContextProvider(ConditionsObjectProvider):
    """Provider of the calibration context condition."""


calibration_context = CalibrationContextProvider()
