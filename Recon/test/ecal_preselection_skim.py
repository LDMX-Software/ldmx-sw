import os
import sys


this_pass_name = "presel"
input_name = sys.argv[1]

from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process(this_pass_name)

p.termLogLevel = 0
p.inputFiles =[input_name]
p.outputFiles =["eventsPreskimmed.root"]

from LDMX.Recon.ecalPreselectionSkimmer import EcalPreselectionSkimmer


ecal_pres_skimmer = EcalPreselectionSkimmer()
ecal_pres_skimmer.summed_tight_iso_max = 1100.
ecal_pres_skimmer.n_readout_hits_max = 90
''' 
## Reminder for the possible things to cut on
ecal_pres_skimmer.summed_det_max = 9999. 
ecal_pres_skimmer.summed_tight_iso_max = 9999. 
ecal_pres_skimmer.ecal_back_energy_max = 9999. 
ecal_pres_skimmer.n_readout_hits_max = 9999
ecal_pres_skimmer.shower_rms_max = 9999. 
ecal_pres_skimmer.shower_y_std_max = 9999. 
ecal_pres_skimmer.shower_x_std_max = 9999.
ecal_pres_skimmer.max_cell_dep_max = 9999. 
ecal_pres_skimmer.std_layer_hit_max = 9999 
ecal_pres_skimmer.n_straight_tracks_max = 9999
ecal_pres_skimmer.bdt_disc_min = 0.
ecal_pres_skimmer.fiducial_level = 0
'''

p.sequence =[ecal_pres_skimmer]
p.skimDefaultIsDrop()
p.skimConsider(p.sequence[0].instance_name)
