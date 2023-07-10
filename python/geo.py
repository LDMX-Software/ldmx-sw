
from LDMX.Framework import ldmxcfg

class TrackingGeometryProvider(ldmxcfg.ConditionsObjectProvider):

    __instance = None

    def get_instance(): 
        if TrackingGeometryInstance.__instance == None:
            TrackingGeometryProvider()

        return TrackingGeometryProvider.__instance

    def __init__(self):
        if TrackingGeometryProvider.__instance != None:
            raise Exception('TrackingGeometryProvider is a singleton class and should only be retrieved using get_instance()')
        else: 
            super().__init__('TrackingGeometryProvider', 'tracking::geo::TrackingGeometryProvider', 'Tracking')
            TrackingGeometryProvider.__instance = self

TrackingGeometryProvider.get_instance()
