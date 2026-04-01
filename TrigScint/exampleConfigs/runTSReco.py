from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('reco') #

import sys

input_pass_name="unpack"
n_ev=60000

time_sample = int(sys.argv[2]) if len(sys.argv) > 2 else 21

from LDMX.TrigScint.trig_scint import TrigScintRecHitProducer


# ------------------- all set; setup in detail, and run with these settings
# ---------------


ts_rec_hits_up  =TrigScintRecHitProducer.up()
ts_rec_hits_up.input_pass_name=input_pass_name
ts_rec_hits_up.input_collection="decodedQIEUp"
ts_rec_hits_up.sipm_gain=4e6
ts_rec_hits_up.pedestal = 4. #12.
ts_rec_hits_up.sample_of_interest=time_sample

p.sequence = [
    ts_rec_hits_up#, ts_digis_tag, ts_digis_down,
    ]


#generate on the fly
p.input_files = [sys.argv[1]]
p.output_files = [ sys.argv[1].replace(".root", "_reco.root") ]
p.max_events = n_ev

p.term_log_level = 2
