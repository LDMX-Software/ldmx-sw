from LDMX.Framework.ldmxcfg import Producer

class TrackingGeometryMaker(Producer) : 

  def __init__(self, instance_name = 'TrackingGeometryMaker'): 
    super().__init__(instance_name, 'tracking::sim::TrackingGeometryMaker', 'Tracking')
