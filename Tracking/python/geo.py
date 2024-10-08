
from LDMX.Framework import ldmxcfg

class TrackersTrackingGeometryProvider(ldmxcfg.ConditionsObjectProvider):
    """The provider of the tracking geometry

    This is a singleton-class and is created when this module is imported.
    Users can then modify its configuration by setting attributes and
    calling methods after calling get_instance(). For example, if you
    wanted to change the detector to v12 since that is what you used during
    the simulation, you would

        from LDMX.Tracking.geo import TrackersTrackingGeometryProvider as trackgeo
        trackgeo.get_instance().setDetector('ldmx-det-v12')

    The default detector is 'ldmx-det-v14'.
    """

    __instance = None

    def get_instance(): 
        if TrackersTrackingGeometryProvider.__instance == None:
            TrackersTrackingGeometryProvider()

        return TrackersTrackingGeometryProvider.__instance

    def setDetector(self, det_name):
        """Set the detector GDML based on the detector name

        Parameters
        ----------
        det_name : str
            name of a detector in the Detectors module

        See Also
        --------
        LDMX.Detectors.makePath for definitions of the path making functions.
        """

        from LDMX.Detectors import makePath as mP
        print("Setting detector for tracking to "+det_name)
        self.detector = mP.makeDetectorPath( det_name )

    def __init__(self):
        if TrackersTrackingGeometryProvider.__instance != None:
            raise Exception('TrackersTrackingGeometryProvider is a singleton class and should only be retrieved using get_instance()')
        else: 
            super().__init__('TrackersTrackingGeometry', 'tracking::geo::TrackersTrackingGeometryProvider', 'Tracking')
            self.debug = False
            self.setDetector('ldmx-det-v14-8gev-no-cals')
            TrackersTrackingGeometryProvider.__instance = self

TrackersTrackingGeometryProvider.get_instance()

class GeometryContextProvider(ldmxcfg.ConditionsObjectProvider):
    """provider of the geometry context condition"""
    def __init__(self):
        super().__init__('GeometryContext', 'tracking::geo::GeometryContextProvider', 'Tracking')

geometry_context = GeometryContextProvider()

class MagneticFieldContextProvider(ldmxcfg.ConditionsObjectProvider):
    """provider of the magnetic field context condition"""
    def __init__(self):
        super().__init__('MagneticFieldContext', 'tracking::geo::MagneticFieldContextProvider', 'Tracking')

magfield_context = MagneticFieldContextProvider()

class CalibrationContextProvider(ldmxcfg.ConditionsObjectProvider):
    """provider of the calibration context condition"""
    def __init__(self):
        super().__init__('CalibrationContext', 'tracking::geo::CalibrationContextProvider', 'Tracking')

calibration_context = CalibrationContextProvider()
