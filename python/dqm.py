
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

        nbins = 200
        d0min = -2.
        d0max =  2.
        z0min = -2.
        z0max =  2.
        pmax  =  6.
        self.doTruth= True

        
        self.build1DHistogram("d0",
                              "d0 [mm]",nbins,d0min,d0max)
                    
        self.build1DHistogram("z0",
                              "z0 [mm]",nbins,z0min,z0max)
        
        self.build1DHistogram("phi",
                              "#phi [rad]",nbins,-3.14,3.14)
        
        self.build1DHistogram("theta",
                              "#theta [rad]",nbins,0.7,2.1)
        
        self.build1DHistogram("p",
                              "P [GeV]",nbins,0,pmax)
        
        self.build1DHistogram("qop",
                              "qOverP [GeV^{-1}]",nbins,-20,20)
        
        self.build1DHistogram("pt_bending",
                              "pT bending plane [GeV]",nbins,-pmax,pmax)
        self.build1DHistogram("pt_beam",
                              "pT beam axis [GeV]",nbins,-pmax,pmax)
        
        self.build1DHistogram("nHits",
                              "nHits",15,0,15)
        self.build1DHistogram("Chi2",
                              "Chi2",nbins,0,100)
        self.build1DHistogram("ndf",
                              "ndf",10,0,10)
        self.build1DHistogram("Chi2/ndf",
                              "Chi2/ndf",nbins,0,10)
        self.build1DHistogram("nShared",
                              "nShared",5,0,5)
        self.build1DHistogram("nHoles",
                              "nHoles",5,0,5)
        self.build1DHistogram("px",
                              "pX [GeV]",nbins,-pmax,pmax)
        self.build1DHistogram("py",
                              "pY [GeV]",nbins,-pmax,pmax)
        self.build1DHistogram("pz",
                              "pZ [GeV]",nbins,-pmax,pmax)
        self.build1DHistogram("d0_err",
                              "#sigma_{d0} [mm]",nbins,0,1)
        self.build1DHistogram("z0_err",
                              "#sigma_{z0} [mm]",nbins,0,2)
        self.build1DHistogram("phi_err",
                              "#sigma_{#phi} [rad]",nbins,0,1)
        self.build1DHistogram("theta_err",
                              "#sigma_{#theta} [rad]",nbins,0,1)
        self.build1DHistogram("qop_err",
                              "#sigma_{qOp} [GeV-1]",nbins,0,20)
        self.build1DHistogram("p_err",
                              "#sigma_{p} [GeV]", nbins, 0, 1)

    
        if self.doTruth:
            self.build1DHistogram("truth_d0",
                                  "truth d0 [mm]", nbins, d0min,d0max)
            self.build1DHistogram("truth_z0",
                                  "truth z0 [mm]", nbins, z0min,z0max)
            self.build1DHistogram("truth_phi",
                                  "truth #phi", nbins, -3.14, 3.14)
            self.build1DHistogram("truth_theta",
                                  "truth #theta", nbins, 0.7,2.1)
            self.build1DHistogram("truth_p",
                                  "truth p [GeV]",nbins,0,pmax)
            self.build1DHistogram("truth_qop",
                                  "truth q/p [GeV^{-1}]",nbins,-20,20)
            
            #self.build1DHistogram("truth_pt_bending",
            #                  "pT bending plane [GeV]",100,-pmax,pmax)
            #self.build1DHistogram("truth_pt_beam",
            #                      "pT beam axis [GeV]",100,-pmax,pmax)
            
            #res
            self.build1DHistogram("res_d0",
                                  "res d0 [mm]", nbins, -0.2,0.2)
            self.build1DHistogram("res_z0",
                                  "res z0 [mm]", nbins, -0.2,0.2)
            self.build1DHistogram("res_phi",
                                  "res #phi", nbins, -0.1, 0.1)
            self.build1DHistogram("res_theta",
                                  "res #theta", nbins, -0.1,0.1)
            self.build1DHistogram("res_p",
                                  "res p [GeV]",nbins,-0.2,0.2)
            self.build1DHistogram("res_qop",
                                  "res q/p [GeV^{-1}]",nbins,-2,2)
            
            #pull
            self.build1DHistogram("pull_d0",
                                  "pull d0",     100, -5,5)
            self.build1DHistogram("pull_z0",
                                  "pull z0",     100, -5,5)
            self.build1DHistogram("pull_phi",
                                  "pull #phi",   100, -5, 5)
            self.build1DHistogram("pull_theta",
                                  "pull #theta", 100, -5,5)
            self.build1DHistogram("pull_p",
                                  "pull p",      100,-5,5)
            self.build1DHistogram("pull_qop",
                                  "pull q/p",    100,-5,5)
            
            
            #Efficiency plots
            self.build1DHistogram("match_d0",
                                  "reco match d0 [mm]", nbins, d0min,d0max)
            self.build1DHistogram("match_z0",
                                  "reco match z0 [mm]", nbins, z0min,z0max)
            self.build1DHistogram("match_phi",
                                  "reco match #phi",    nbins, -3.14, 3.14)
            self.build1DHistogram("match_theta",
                                  "reco match #theta",  nbins, 0.7,2.1)
            self.build1DHistogram("match_p",
                                  "reco match p [GeV]", nbins,0,pmax)
            
            
            chi2Fake_max    = 500
            chi2NdfFake_max = 50
            
            #Fake Plots
            self.build1DHistogram("fake_d0",
                                  "fake d0 [mm]", nbins, d0min,d0max)
            self.build1DHistogram("fake_z0",
                                  "fake z0 [mm]", nbins, z0min,z0max)
            self.build1DHistogram("fake_phi",
                                  "fake #phi", nbins, -3.14, 3.14)
            self.build1DHistogram("fake_theta",
                                  "fake #theta", nbins, 0.7,2.1)
            self.build1DHistogram("fake_p",
                                  "fake p [GeV]",nbins,0,pmax)
            self.build1DHistogram("fake_nHits",
                                  "fake nHits",15,0,15)
            self.build1DHistogram("fake_Chi2",
                                  "fake Chi2",100,0,chi2Fake_max)
            self.build1DHistogram("fake_Chi2/ndf",
                                  "fake Chi2/ndf",100,0,chi2NdfFake_max)
            self.build1DHistogram("fake_nShared",
                                  "fake nShared",5,0,5)
            self.build1DHistogram("fake_nHoles",
                                  "fake nHoles",5,0,5)
            

            #Duplicate plots
                
            self.build1DHistogram("dup_d0",
                                  "dup d0 [mm]", 100, d0min,d0max)
            self.build1DHistogram("dup_z0",
                                  "dup z0 [mm]", 100, z0min,z0max)
            self.build1DHistogram("dup_phi",
                                  "dup #phi", 100, -3.14, 3.14)
            self.build1DHistogram("dup_theta",
                                  "dup #theta", 100, 0.7,2.1)
            self.build1DHistogram("dup_p",
                                  "dup p [GeV]",100,0,pmax)
            self.build1DHistogram("dup_nHits",
                                  "dup nHits",15,0,15)
            self.build1DHistogram("dup_Chi2",
                                  "dup Chi2",100,0,100)
            self.build1DHistogram("dup_Chi2/ndf",
                                  "dup Chi2/ndf",100,0,10)
            self.build1DHistogram("dup_nShared",
                                  "dup nShared",5,0,5)
            self.build1DHistogram("dup_nHoles",
                                  "dup nHoles",5,0,5)
                
