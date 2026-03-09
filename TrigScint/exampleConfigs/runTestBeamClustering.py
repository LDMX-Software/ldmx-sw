from os.path import exists
from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('clusters') #

import sys

input_pass_name="hits"
n_ev=400000
#p.max_events = n_ev

time_sample = int(sys.argv[2]) if len(sys.argv) > 2 else 21

from LDMX.TrigScint.trigScint import TestBeamClusterProducer


tbClustersUp  =TestBeamClusterProducer("tb_clusters")
tbClustersUp.input_pass_name=input_pass_name
#tbClustersUp.input_collection="TestBeamHitsUp"
tbClustersUp.pad_time=0.
tbClustersUp.threshold=15.
tbClustersUp.time_tolerance=50.
tbClustersUp.verbosity=0
p.sequence = [
    tbClustersUp
    ]


#generate on the fly
p.input_files = [sys.argv[1]]
p.output_files = [ sys.argv[1].replace(".root", "_clusters.root") ]

p.term_log_level = 2
p.log_file_name = p.output_files[0].replace(".root", ".log")
p.logFileLevel=0

