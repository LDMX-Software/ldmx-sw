
from LDMX.Framework import ldmxcfg

class TrackerDigiDQM(ldmxcfg.Analyzer):
    """
    """
    def __init__(self, name='TaggerTracker'): 
        super().__init__(name, 'tracking::dqm::TrackerDigiDQM', 'Tracking')

        self.output_measurements_passname = ''
        self.measurements_passname = ''
        
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
        
        self.nbins    = 1000
        self.d0min    = -50#-15.
        self.d0max    = 50# 15.
        self.z0min    = -200#-45.
        self.z0max    = 200# 45.
        self.phimin   = -0.1#-0.2
        self.phimax   = 0.1#0.2
        self.thetamin = 1.4#1.4
        self.thetamax = 1.7#1.7
        self.qopmin   = -22
        self.qopmax   = 22
        self.pmax     =  10.

        self.trackStates = ["ecal","target"]
        self.doTruth= True
      
        self.track_collection = "TruthTracks"
        self.truth_collection = "TaggerTruthTracks"
        self.measurement_collection = "DigiTaggerSimHits"    
        self.measurement_passname = ""
        
        self.ecal_sp_events_passname= ""
        self.ecal_sp_passname= ""
        self.target_sp_events_passname= ""
        self.target_sp_passname= ""
        self.truth_events_passname = ""
        self.truth_passname = ""
        self.track_collection_events_passname = ""
        self.track_passname = ""

    def buildHistograms(self) :
        
        nbins   =self.nbins   
        d0min   =self.d0min    
        d0max   =self.d0max   
        z0min   =self.z0min   
        z0max   =self.z0max   
        phimin  =self.phimin  
        phimax  =self.phimax  
        thetamin=self.thetamin
        thetamax=self.thetamax
        qopmin  =self.qopmin  
        qopmax  =self.qopmax  
        pmax    =self.pmax    
        
        
        self.build1DHistogram("N_tracks",
                              "N tracks",10,0,10)
        
        
        self.build1DHistogram("d0",
                              "d0 [mm]",nbins,d0min,d0max)
                    
        self.build1DHistogram("z0",
                              "z0 [mm]",nbins,z0min,z0max)
        
        self.build1DHistogram("phi",
                              "#phi [rad]",nbins,phimin,phimax)
        
        self.build1DHistogram("theta",
                              "#theta [rad]",nbins,thetamin,thetamax)
        
        self.build1DHistogram("p",
                              "P [GeV]",nbins,0,pmax)
        
        self.build1DHistogram("qop",
                              "qOverP [GeV^{-1}]",nbins,qopmin,qopmax)
        
        self.build1DHistogram("pt_bending",
                              "pT bending plane [GeV]",nbins,0.,pmax)
        self.build1DHistogram("pt_beam",
                              "pT beam axis [GeV]",nbins,0,0.5)
        
        self.build1DHistogram("nHits",
                              "nHits",15,0,15)
        self.build1DHistogram("LayersHit",
                              "LayersHit",15,0,15)
        self.build1DHistogram("Chi2",
                              "Chi2",nbins,0,100)
        self.build1DHistogram("ndf",
                              "ndf",10,0,10)
        self.build1DHistogram("Chi2_per_ndf",
                              "Chi2/ndf",nbins,0,10)
        self.build1DHistogram("nShared",
                              "nShared",5,0,5)
        self.build1DHistogram("nHoles",
                              "nHoles",5,0,5)
        self.build1DHistogram("px",
                              "pX [GeV]",nbins,0.0,pmax)
        self.build1DHistogram("py",
                              "pY [GeV]",nbins,-pmax/20.0,pmax/20.0)
        self.build1DHistogram("pz",
                              "pZ [GeV]",nbins,-pmax/20.0,pmax/20.0)
        self.build1DHistogram("d0_err",
                              "#sigma_{d0} [mm]",nbins,0,0.2)
        self.build1DHistogram("z0_err",
                              "#sigma_{z0} [mm]",nbins,0,0.7)
        self.build1DHistogram("phi_err",
                              "#sigma_{#phi} [rad]",nbins,0,0.04)
        self.build1DHistogram("theta_err",
                              "#sigma_{#theta} [rad]",nbins,0,0.06)
        self.build1DHistogram("qop_err",
                              "#sigma_{qOp} [GeV^{-1}]",nbins,0,1)
        self.build1DHistogram("p_err",
                              "#sigma_{p} [GeV]", nbins, 0, 1)


        self.build2DHistogram("d0_err_vs_p",
                              "p [GeV]", nbins, 0, pmax,
                              "#sigma_{d0} [mm]", nbins, 0,0.2)

        self.build2DHistogram("z0_err_vs_p",
                              "p [GeV]", nbins, 0, pmax,
                              "#sigma_{z0} [mm]", nbins, 0,0.8)

        self.build2DHistogram("p_err_vs_p",
                              "p [GeV]", nbins, 0, pmax,
                              "#sigma_{p} [GeV]", nbins, 0,1)

        self.build2DHistogram("p_err_vs_p_8hits",
                              "p [GeV]", nbins, 0, pmax,
                              "#sigma_{p} [GeV]", nbins, 0,1)

        self.build2DHistogram("p_err_vs_p_9hits",
                              "p [GeV]", nbins, 0, pmax,
                              "#sigma_{p} [GeV]", nbins, 0,1)

        self.build2DHistogram("p_err_vs_p_10hits",
                              "p [GeV]", nbins, 0, pmax,
                              "#sigma_{p} [GeV]", nbins, 0,1)

        self.build2DHistogram("res_p_vs_p",
                              "p [GeV]", nbins, 0, pmax,
                              "res_{p} [GeV]", nbins, -3,3)

        self.build2DHistogram("res_qop_vs_p",
                              "p [GeV]", nbins, 0, pmax,
                              "res_{qop} [GeV^{-1}]", nbins, -0.5,0.5)

        self.build2DHistogram("res_d0_vs_p",
                              "p [GeV]"      , nbins, 0, pmax,
                              "res_{d0} [mm]", nbins, -0.05,0.05)

        self.build2DHistogram("res_z0_vs_p",
                              "p [GeV]"      , nbins, 0, pmax,
                              "res_{z0} [mm]", nbins,-0.5,0.5)

        self.build2DHistogram("res_phi_vs_p",
                              "p [GeV]"   , nbins, 0, pmax,
                              "res_{#phi}", nbins, -0.005,0.005)

        self.build2DHistogram("res_theta_vs_p",
                              "p [GeV]"     , nbins, 0, pmax,
                              "res_{#theta}", nbins, -0.01,0.01)

        self.build2DHistogram("res_p_vs_p_8hits",
                              "p [GeV]",       nbins, 0, pmax,
                              "res_{p} [GeV]", nbins, -3,3)

        self.build2DHistogram("res_p_vs_p_9hits",
                              "p [GeV]", nbins, 0, pmax,
                              "res_{p} [GeV]", nbins, -3,3)

        self.build2DHistogram("res_p_vs_p_10hits",
                              "p [GeV]", nbins, 0, pmax,
                              "res_{p} [GeV]", nbins, -3,3)


        self.build2DHistogram("pull_qop_vs_p",
                              "p [GeV]"   , nbins, 0, pmax,
                              "pull_{qop}", nbins, -5,5)

        self.build2DHistogram("pull_d0_vs_p",
                              "p [GeV]"      , nbins, 0, pmax,
                              "pull_{d0}"    , nbins, -5,5)

        self.build2DHistogram("pull_z0_vs_p",
                              "p [GeV]"      , nbins, 0, pmax,
                              "pull_{z0}"    , nbins,-5,5)

        self.build2DHistogram("pull_phi_vs_p",
                              "p [GeV]"   , nbins, 0, pmax,
                              "pull_{#phi}", nbins, -5,5)

        self.build2DHistogram("pull_theta_vs_p",
                              "p [GeV]"      , nbins, 0, pmax,
                              "pull_{#theta}", nbins, -5,5)
                
        
        self.build2DHistogram("res_pt_beam_vs_p",
                              "truth p [GeV]", nbins, 0, pmax,
                              "res_{pt} beam", nbins, -0.5,0.5)
        
    
        if self.doTruth:
            self.build1DHistogram("truth_N_tracks",
                              "truth_N tracks",10,0,10)
            self.build1DHistogram("truth_nHits",
                                  "truth nHits", 15, 0,15)
            self.build1DHistogram("truth_LayersHit",
                                  "truth_LayersHit",15,0,15)
            self.build1DHistogram("truth_d0",
                                  "truth d0 [mm]", nbins, d0min,d0max)
            self.build1DHistogram("truth_z0",
                                  "truth z0 [mm]", nbins, z0min,z0max)
            self.build1DHistogram("truth_phi",
                                  "truth #phi", nbins, phimin, phimax)
            self.build1DHistogram("truth_theta",
                                  "truth #theta", nbins, thetamin,thetamax)
            self.build1DHistogram("truth_qop",
                                  "truth q/p [GeV^{-1}]",nbins,qopmin,qopmax)
            self.build1DHistogram("truth_p",
                                  "truth p [GeV]",nbins,0,pmax)
            self.build1DHistogram("truth_beam_angle",
                                  "angle wrt beam axis",20,0,2)
            self.build1DHistogram("truth_PID",
                                  "Particles",8,-4,4)
            
            self.build1DHistogram("truth_kminus_p",
                                  "truth p",nbins, 0., pmax)
            self.build1DHistogram("truth_kplus_p",
                                  "truth p",nbins, 0., pmax)
            self.build1DHistogram("truth_piminus_p",
                                  "truth p",nbins, 0., pmax)
            self.build1DHistogram("truth_piplus_p",
                                  "truth p",nbins, 0., pmax)
            self.build1DHistogram("truth_electron_p",
                                  "truth p",nbins, 0., pmax)
            self.build1DHistogram("truth_positron_p",
                                  "truth p",nbins, 0., pmax)
            self.build1DHistogram("truth_proton_p",
                                  "truth p",nbins, 0., pmax)
            
            
            #self.build1DHistogram("truth_pt_bending",
            #                  "pT bending plane [GeV]",100,-pmax,pmax)
            #self.build1DHistogram("truth_pt_beam",
            #                      "pT beam axis [GeV]",100,-pmax,pmax)
            
            #res
            self.build1DHistogram("res_d0",
                                  "res d0 [mm]", nbins, -0.2,0.2)
            self.build1DHistogram("res_z0",
                                  "res z0 [mm]", nbins, -0.6,0.6)
            self.build1DHistogram("res_phi",
                                  "res #phi", nbins, -0.02, 0.02)
            self.build1DHistogram("res_theta",
                                  "res #theta", nbins, -0.04,0.04)
            self.build1DHistogram("res_p",
                                  "res p [GeV]",nbins,-1,1)

            self.build1DHistogram("res_pt_beam",
                                  "res pt beam [GeV]", nbins, -1,1)
            
            self.build1DHistogram("res_qop",
                                  "res q/p [GeV^{-1}]",nbins,-1,1)
            
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
            self.build1DHistogram("match_prob",
                                  "reco truth match probability", nbins, 0.0,1.1)
            self.build1DHistogram("match_d0",
                                  "reco match d0 [mm]", nbins, d0min,d0max)
            self.build1DHistogram("match_z0",
                                  "reco match z0 [mm]", nbins, z0min,z0max)
            self.build1DHistogram("match_phi",
                                  "reco match #phi",    nbins, phimin, phimax)
            self.build1DHistogram("match_theta",
                                  "reco match #theta",  nbins, thetamin,thetamax)
            self.build1DHistogram("match_qop",
                                  "truth q/p [GeV^{-1}]",nbins,qopmin,qopmax)
            self.build1DHistogram("match_p",
                                  "truth p [GeV]", nbins,0,pmax)
            self.build1DHistogram("match_beam_angle",
                                  "angle wrt beam axis", 20, 0, 2)
            self.build1DHistogram("match_PID",
                                  "Particles",8,-4,4)
            self.build1DHistogram("match_nHits",
                                  "match nHits",15,0,15)
            self.build1DHistogram("match_LayersHit",
                                  "match_LayersHit",15,0,15)



            self.build1DHistogram("match_kminus_p",
                                  "truth p",nbins, 0., pmax)
            self.build1DHistogram("match_kplus_p",
                                  "truth p",nbins, 0., pmax)
            self.build1DHistogram("match_piminus_p",
                                  "truth p",nbins, 0., pmax)
            self.build1DHistogram("match_piplus_p",
                                  "truth p",nbins, 0., pmax)
            self.build1DHistogram("match_electron_p",
                                  "truth p",nbins, 0., pmax)
            self.build1DHistogram("match_positron_p",
                                  "truth p",nbins, 0., pmax)
            self.build1DHistogram("match_proton_p",
                                  "truth p",nbins, 0., pmax)
            

            
            chi2Fake_max    = 500
            chi2NdfFake_max = 50
            scaling = 1.
            
            #Fake Plots
            self.build1DHistogram("fake_d0",
                                  "fake d0 [mm]", nbins, scaling*d0min,scaling*d0max)
            self.build1DHistogram("fake_z0",
                                  "fake z0 [mm]", nbins, scaling*z0min,scaling*z0max)
            self.build1DHistogram("fake_phi",
                                  "fake #phi", nbins, scaling*phimin, scaling*phimax)
            self.build1DHistogram("fake_theta",
                                  "fake #theta", nbins, scaling*thetamin,scaling*thetamax)
            self.build1DHistogram("fake_p",
                                  "fake p [GeV]",nbins,0,pmax)
            self.build1DHistogram("fake_qop",
                                  "fake qOverP [GeV^{-1}]",nbins,-40,40)
            self.build1DHistogram("fake_nHits",
                                  "fake nHits",15,0,15)
            self.build1DHistogram("fake_LayersHit",
                                  "fake_LayersHit",15,0,15)
            self.build1DHistogram("fake_Chi2",
                                  "fake Chi2",100,0,chi2Fake_max)
            self.build1DHistogram("fake_Chi2_per_ndf",
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
                                  "dup #phi", 100, phimin, phimax)
            self.build1DHistogram("dup_theta",
                                  "dup #theta", 100, thetamin,thetamax)
            self.build1DHistogram("dup_p",
                                  "dup p [GeV]",100,0,pmax)
            self.build1DHistogram("dup_qop",
                                  "dup qOverP [GeV^{-1}]",nbins,qopmin,qopmax)
            self.build1DHistogram("dup_nHits",
                                  "dup nHits",15,0,15)
            self.build1DHistogram("dup_LayersHit",
                                  "dup_LayersHit",15,0,15)
            self.build1DHistogram("dup_Chi2",
                                  "dup Chi2",100,0,100)
            self.build1DHistogram("dup_Chi2_per_ndf",
                                  "dup Chi2/ndf",100,0,10)
            self.build1DHistogram("dup_nShared",
                                  "dup nShared",5,0,5)
            self.build1DHistogram("dup_nHoles",
                                  "dup nHoles",5,0,5)
                
            



            #Track states extrapolations
            for trackState in self.trackStates:
                
                self.build1DHistogram("trk_"+trackState+"_loc0","trk_"+trackState+"_loc0 [mm]",200,-50,50)
                self.build1DHistogram("trk_"+trackState+"_loc1","trk_"+trackState+"_loc1 [mm]",200,-50,50)
                self.build1DHistogram(trackState+"_sp_hit_X",trackState+"_sp_hit_X [mm]",200,-50,50)
                self.build1DHistogram(trackState+"_sp_hit_Y",trackState+"_sp_hit_Y [mm]",200,-50,50)
                self.build1DHistogram("trk_"+trackState+"_loc0-sp_hit_X",trackState+"_diff loc0 and hit_X [mm]",200,-0.2,0.2)
                self.build1DHistogram("trk_"+trackState+"_loc1-sp_hit_Y",trackState+"_diff loc1 and hit_Y [mm]",200,-5,5)
                self.build1DHistogram(trackState+"_Pulls_of_loc0",trackState+"_pulls_of_loc0 [mm]",200,-5,5)
                self.build1DHistogram(trackState+"_Pulls_of_loc1",trackState+"_pulls_of_loc1 [mm]",200,-5,5)
           
                self.build2DHistogram(trackState+"_res_loc0-vs-N_hits","N_hits",  5,6.5,11.5,trackState+"_res_loc0 [mm]",100,-0.2,0.2)
                self.build2DHistogram(trackState+"_res_loc1-vs-N_hits","N_hits",  5,6.5,11.5,trackState+"_res_loc1 [mm]",100,-5,5)
                self.build2DHistogram(trackState+"_res_loc0-vs-trk_p",  "trk_p",200,0,5,     trackState+"_res_loc0 [mm]",100,-0.2,0.2)
                self.build2DHistogram(trackState+"_res_loc1-vs-trk_p",  "trk_p",200,0,5,     trackState+"_res_loc1 [mm]",100,-5,5)
                self.build2DHistogram(trackState+"_pulls_loc0-vs-N_hits","N_hits",5,6.5,11.5,trackState+"_pulls_loc0 [mm]",100,-3,3)
                self.build2DHistogram(trackState+"_pulls_loc1-vs-N_hits","N_hits",5,6.5,11.5,trackState+"_pulls_loc1 [mm]",100,-3,3)
                self.build2DHistogram(trackState+"_pulls_loc0-vs-trk_p","trk_p",200,0,5,     trackState+"_pulls_loc0 [mm]",100,-3,3)
                self.build2DHistogram(trackState+"_pulls_loc1-vs-trk_p","trk_p",200,0,5,     trackState+"_pulls_loc1 [mm]",100,-3,3)
            
class StraightTracksDQM(ldmxcfg.Analyzer):
    def __init__(self, name="StraightTracksDQM"):
        super().__init__(name, 'tracking::dqm::StraightTracksDQM','Tracking')
        
        self.nbins    = 100
        self.phimin   = -0.2
        self.phimax   = 0.2
        self.thetamin = -0.3
        self.thetamax = 0.3
                         
        self.doTruth= True
        
        self.track_collection = "LinearRecoilTracks"
        self.truth_collection = "LinearRecoilTruthTracks"
        self.title="recoil_lin_trk_"
        self.subdetector = "Recoil"
        self.measurement_collection = "DigiRecoilSimHits"
        
        self.track_collection_events_passname = ""
        self.truth_collection_events_passname = ""
        self.input_pass_name = ""
    
    def buildHistograms(self) :
        
        nbins   =self.nbins
        phimin  =self.phimin
        phimax  =self.phimax
        thetamin=self.thetamin
        thetamax=self.thetamax
        
        self.build1DHistogram("N_tracks","N tracks",10,0,10)
        
        self.build1DHistogram("phi", "#phi [rad]",nbins,phimin,phimax)
        
        self.build1DHistogram("theta", "#theta [rad]",nbins,thetamin,thetamax)
        
        self.build1DHistogram("trk_PID","Particles",30,-15,15)
        self.build1DHistogram("nHits", "nHits",5,0,5)
        self.build1DHistogram("Chi2", "Chi2",nbins,0,15)
        self.build1DHistogram("ndf", "ndf",5,0,5)
        self.build1DHistogram("Chi2_per_ndf", "Chi2/ndf",nbins,0,10)
        self.build1DHistogram("dRecHit", "Distance to Nearest EcalRecHit [mm]",nbins, 0.0, 20.0)
        self.build1DHistogram("phi_err","#sigma_{#phi} [rad]",nbins,0,0.001)
        self.build1DHistogram("theta_err","#sigma_{#theta} [rad]",nbins,0,0.03)
    
        if self.doTruth:
#            self.build1DHistogram("truth_N_tracks","truth_N tracks",10,0,10)
#            self.build1DHistogram("truth_nHits","truth nHits", 15, 0,15)
            self.build1DHistogram("truth_phi","truth #phi", nbins, phimin, phimax)
            self.build1DHistogram("truth_theta","truth #theta", nbins, thetamin,thetamax)

            self.build1DHistogram("truth_PID","Particles",30,-15,15)

            self.build1DHistogram("res_phi","res #phi", nbins, -0.02, 0.02)
            self.build1DHistogram("res_theta","res #theta", nbins, -0.04,0.04)

            self.build1DHistogram("pull_phi","pull #phi",   100, -5, 5)
            self.build1DHistogram("pull_theta","pull #theta", 100, -5,5)
        
            chi2Fake_max    = 15
            chi2NdfFake_max = 10
            scaling = 1.
                
            #Fake Plots
            self.build1DHistogram("fake_phi","fake #phi", nbins, scaling*phimin, scaling*phimax)
            self.build1DHistogram("fake_theta", "fake #theta", nbins, scaling*thetamin,scaling*thetamax)
            self.build1DHistogram("fake_nHits","fake nHits",5,0,5)
            self.build1DHistogram("fake_Chi2","fake Chi2",nbins,0,chi2Fake_max)
            self.build1DHistogram("fake_Chi2_per_ndf","fake Chi2/ndf",nbins,0,chi2NdfFake_max)
            self.build1DHistogram("fake_trk_PID","Particles",8,-4,4)
                
            #Duplicate plots
            self.build1DHistogram("dup_phi","dup #phi", 100, phimin, phimax)
            self.build1DHistogram("dup_theta","dup #theta", 100, thetamin,thetamax)
            self.build1DHistogram("dup_nHits","dup nHits",5,0,5)
            self.build1DHistogram("dup_Chi2","dup Chi2",1000,0,100)
            self.build1DHistogram("dup_Chi2_per_ndf","dup Chi2/ndf",1000,0,100)
            self.build1DHistogram("dup_trk_PID","Particles",30,-15,15)
                    
            self.build1DHistogram("trk_target_loc0","trk_target_loc0 [mm]",200,-50,50)
            self.build1DHistogram("trk_target_loc1","trk_target_loc1 [mm]",200,-50,50)
            self.build1DHistogram("trk_target_loc0-truth_target_loc0","target trk_loc0 - truth_loc0 [mm]",100,-0.1,0.1)
            self.build1DHistogram("trk_target_loc1-truth_target_loc1","target trk_loc1 - truth_loc1 [mm]",100,-2,2)
            self.build1DHistogram("trk_ecal_loc0","trk_ecal_loc0 [mm]",200,-50,50)
            self.build1DHistogram("trk_ecal_loc1","trk_ecal_loc1 [mm]",200,-50,50)
            self.build1DHistogram("trk_ecal_loc0-truth_ecal_loc0","ecal trk_loc0 - truth_loc0 [mm]",200,-0.3,0.3)
            self.build1DHistogram("trk_ecal_loc1-truth_ecal_loc1","ecal trk_loc1 - truth_loc1 [mm]",200,-6,6)

            self.build1DHistogram("target_Pulls_of_loc0","target_pulls_of_loc0 [mm]",200,-5,5)
            self.build1DHistogram("target_Pulls_of_loc1","target_pulls_of_loc1 [mm]",200,-5,5)
            self.build1DHistogram("ecal_Pulls_of_loc0","ecal_pulls_of_loc0 [mm]",200,-5,5)
            self.build1DHistogram("ecal_Pulls_of_loc1","ecal_pulls_of_loc1 [mm]",200,-5,5)

            self.build2DHistogram("target_res_loc0-vs-N_hits","N_hits",5,0.0,5.0,"target_res_loc0 [mm]",100,-0.2,0.2)
            self.build2DHistogram("target_res_loc1-vs-N_hits","N_hits",5,0.0, 5.0,"target_res_loc1 [mm]",100,-5,5)
            self.build2DHistogram("ecal_res_loc0-vs-N_hits","N_hits",5,0.0,5.0,"ecal_res_loc0 [mm]",100,-0.2,0.2)
            self.build2DHistogram("ecal_res_loc1-vs-N_hits","N_hits",5,0.0, 5.0,"ecal_res_loc1 [mm]",100,-5,5)
            
            self.build2DHistogram("target_pulls_loc0-vs-N_hits","N_hits",5,0.0,5.0,"target_pulls_loc0 [mm]",100,-3,3)
            self.build2DHistogram("target_pulls_loc1-vs-N_hits","N_hits",5,0.0,5.0,"target_pulls_loc1 [mm]",100,-3,3)
            self.build2DHistogram("ecal_pulls_loc0-vs-N_hits","N_hits",5,0.0,5.0,"ecal_pulls_loc0 [mm]",100,-3,3)
            self.build2DHistogram("ecal_pulls_loc1-vs-N_hits","N_hits",5,0.0,5.0,"ecal_pulls_loc1 [mm]",100,-3,3)
