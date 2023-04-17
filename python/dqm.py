
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
                                  "qOverP [GeV^{-1}]",100,-20,20)

            self.build1DHistogram(title+"pt_bending",
                                  "pT bending plane [GeV]",100,-pmax,pmax)
            self.build1DHistogram(title+"pt_beam",
                                  "pT beam axis [GeV]",100,-pmax,pmax)
                        
            self.build1DHistogram(title+"nHits",
                                  "nHits",15,0,15)
            self.build1DHistogram(title+"Chi2",
                                  "Chi2",100,0,100)
            self.build1DHistogram(title+"Chi2/ndf",
                                  "Chi2/ndf",100,0,10)
            self.build1DHistogram(title+"nShared",
                                  "nShared",5,0,5)
            self.build1DHistogram(title+"nHoles",
                                  "nHoles",5,0,5)
            self.build1DHistogram(title+"px",
                                  "pX [GeV]",100,-pmax,pmax)
            self.build1DHistogram(title+"py",
                                  "pY [GeV]",100,-pmax,pmax)
            self.build1DHistogram(title+"pz",
                                  "pZ [GeV]",100,-pmax,pmax)
            self.build1DHistogram(title+"d0_err",
                                  "#sigma_{d0} [mm]",100,0,1)
            self.build1DHistogram(title+"z0_err",
                                  "#sigma_{z0} [mm]",100,0,2)
            self.build1DHistogram(title+"phi_err",
                                  "#sigma_{#phi} [rad]",100,0,1)
            self.build1DHistogram(title+"theta_err",
                                  "#sigma_{#theta} [rad]",100,0,1)
            self.build1DHistogram(title+"qop_err",
                                  "#sigma_{qOp} [GeV-1]",100,0,20)
            self.build1DHistogram(title+"p_err",
                                  "#sigma_{p} [GeV]", 100, 0, 1)
            
            
            
