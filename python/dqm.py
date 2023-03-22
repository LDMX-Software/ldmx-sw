
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



class TrackingRecoDQM(ldmxcfg.Analyzer):
    """
    """
    def __init__(self, name="TrackingRecoDQM"):
        super().__init__(name, 'tracking::dqm::TrackingRecoDQM',
                         'Tracking')

        d0min = -2.
        d0max =  2.
        z0min = -2.
        z0max =  2.
        pmax  =  6.

        titles = ["tagger_trk_","recoil_trk_"]

        for title in titles:

            self.build1DHistogram(title+"d0",
                                  "d0 [mm]",100,d0min,d0max)
        
            self.build1DHistogram(title+"z0",
                                  "z0 [mm]",100,z0min,z0max)

            self.build1DHistogram(title+"phi",
                                  "#phi [rad]",100,-3.14,3.14)

            self.build1DHistogram(title+"theta",
                                  "#theta [rad]",100,0.7,2.1)

            self.build1DHistogram(title+"p",
                                  "P [GeV]",100,0,pmax)

            self.build1DHistogram(title+"qOp",
                                  "qOverP [GeV^{-1}]",100,-1/pmax,1/pmax)
                        
            
            
