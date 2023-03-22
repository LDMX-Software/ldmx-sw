
from LDMX.Framework import ldmxcfg

class TrackerDigiDQM(ldmxcfg.Analyzer):
    """
    """
    def __init__(self, name='TaggerTracker'): 
        super().__init__(name, 'tracking::dqm::TrackerDigiDQM', 'Tracking')

        for i in range(0, 14): 
            self.build2DHistogram('global_yz_l%s' % i, 
                                  'Global y (mm)', 70, -50, 20, 
                                  'Global z (mm)', 100, -50, 50)
            self.build2DHistogram('local_uv_l%s' % i, 
                                  'u (mm)', 60, -30, 30, 
                                  'v (mm)', 100, -50, 50)
            self.build1DHistogram('time_l%s' % i, 
                                  'Time (ns)', 100, 0, 100)

        self.build2DHistogram('global_xy', 
                              'Global x (mm)', 100, -1000, 0, 
                              'Global y (mm)', 100, -50, 50)

