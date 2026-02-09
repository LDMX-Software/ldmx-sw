import os
import sys


file_in = sys.argv[1:]
file_name =  " ".join(sys.argv[1:])

from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process('myAna')
dump_run_header = ldmxcfg.RunHeaderAna()

p.max_events = -1
p.run = 2

p.inputFiles  = file_in
print(p.inputFiles)

p.sequence = [dump_run_header]



