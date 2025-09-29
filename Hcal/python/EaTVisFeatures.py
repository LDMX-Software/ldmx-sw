
"""Configuration for EaTVisFeatures Routine

Examples
--------
   from LDMX.Hcal.EaTVisFeatures import EaTVisFeatures
   p.sequence.append(EaTVisFeatures)
"""

from LDMX.Framework import ldmxcfg
import os, sys

class EaTVisFeatures(ldmxcfg.Analyzer) :
    """Just plot the visibles features"""

    def __init__(self, name='EaTvis') :
        super().__init__(name, 'hcal::EaTVisFeatures', 'Hcal')

        ## Parameters choose whether to save features to .txt file ##
        self.training = False
        self.training_file = ""

        ## A few other useful parameters ##
        self.beam_energy = 8000.0 # in MeV
        self.hcal_rec_coll_name = "HcalRecHits"
        self.ecal_rec_coll_name = "EcalRecHits"
        self.recoil_from_tracking = False
        self.track_collection = 'RecoilTracks'
        self.sp_coll_name = 'TargetScoringPlaneHits'
        self.default_pass_name = "tat_vis" #"eat_vis"
        
        ## Define histograms ##
        self.build1DHistogram("ap_parent_track_id","ap_parent_track_id",105,-5,100)
        self.build1DHistogram("number_aps","number_aps",10,0,10);
        self.build1DHistogram("ap_daughter_pdgids","ap_daughter_pdgids",200,-100,100)
        self.build1DHistogram("ap_energy","ap_energy",100,0,10000)
        self.build1DHistogram("ap_vertex_z","ap_vertex_z",100,-200,800)
        self.build1DHistogram("ap_endpoint_z","ap_endpoint_z",580,200,6000)
        self.build1DHistogram("ecalE_up","ecalE_up", 150,0,9800)
        self.build1DHistogram("ecalE_tot","ecalE_tot",150,0,9800)
        self.build1DHistogram("hcalE","hcalE",150,0,9800)
        self.build1DHistogram("ecalHitz","ecalHitz",100,0,500)
        self.build1DHistogram("ap_flight_dist_all","ap_flight_dist_all",100,0,6000)
        self.build1DHistogram("ap_flight_dist_trigger","ap_flight_dist_trigger",100,0,6000)
        self.build1DHistogram("ap_flight_dist_ecalE","ap_flight_dist_ecalE",100,0,6000)
        self.build1DHistogram("ap_flight_dist_hcalE","ap_flight_dist_hcalE",100,0,6000)
        self.build1DHistogram("ap_parent_pdgids", "ap_parent_pdgids", 200, -100,100)
        self.build1DHistogram("ap_parent_energies", "ap_parent_energies",100, 0, 10000)
        self.build1DHistogram("non_beam_parents", "non_beam_parents", 105, -5, 100)
        self.build1DHistogram("non_beam_parents_ap_vertex", "non_beam_parents_ap_vertex", 100, -200, 800)
        self.build1DHistogram("non_beam_parents_ap_energy", "non_beam_parents_ap_energy", 100, 0, 10000)
        self.build1DHistogram("ap_parent_all_daught_esum", "ap_parent_all_daught_esum", 1000, 7000, 9000)
        self.build1DHistogram("pre_target_energy_ap_parent", "pre_target_energy_ap_parent", 250, 3000, 9000);
        self.build1DHistogram("trigger_ecalE","trigger_ecalE", 100,0,10000)
        self.build1DHistogram("ecal_req_ecalE","ecal_req_ecalE",100,0,10000)
        self.build1DHistogram("hcal_req_hcalE","hcal_req_hcalE",100,0,10000)
        #self.build1DHistogram("ecalE_all","ecalE_all",150,0,9800)
        #self.build1DHistogram("ecalpass","ecalpass",100,0,5000)
        self.build1DHistogram("layershit", "layershit", 100, 0, 100);
        self.build1DHistogram("xStd", "xStd", 80, 0, 800);
        self.build1DHistogram("yStd", "yStd", 80, 0, 800);
        self.build1DHistogram("zStd", "zStd", 100, 0, 1000)
        self.build1DHistogram("xMean", "xMeean", 80, -800, 800)
        self.build1DHistogram("yMean", "yMean", 80, -800, 800)
        self.build1DHistogram("rMean", "rMean", 80, 0, 800)
        self.build1DHistogram("isoHits", "isoHits", 100, 0, 100)
        self.build1DHistogram("isoE", "isoE", 80, 0, 800)
        self.build1DHistogram("nHits", "nHits", 200, 0, 200)
        self.build1DHistogram("Etot", "Etot", 100, 4800, 9800)
        #self.build1DHistogram("pass_cut_ap_parent_track_id","pass_cut_ap_parent_track_id",105,-5,100);




